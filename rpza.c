/*
 * Quicktime Video (RPZA) Video Decoder
 * Copyright (C) 2003 the ffmpeg project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * @file rpza.c
 * QT RPZA Video Decoder by Roberto Togni <rtogni@bresciaonline.it>
 * For more information about the RPZA format, visit:
 *   http://www.pcisys.net/~melanson/codecs/
 *
 * The RPZA decoder outputs RGB555 colorspace data.
 *
 * Note that this decoder reads big endian RGB555 pixel values from the
 * bytestream, arranges them in the host's endian order, and outputs
 * them to the final rendered map in the same host endian order. This is
 * intended behavior as the ffmpeg documentation states that RGB555 pixels
 * shall be stored in native CPU endianness.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "avcodec.h"
#include "dsputil.h"

typedef struct RpzaContext {

    AVCodecContext *avctx;
    DSPContext dsp;
    AVFrame frame;
    AVFrame prev_frame;

    unsigned char *buf;
    int size;

} RpzaContext;

#define BE_16(x)  ((((uint8_t*)(x))[0] << 8) | ((uint8_t*)(x))[1])
#define BE_32(x)  ((((uint8_t*)(x))[0] << 24) | \
                   (((uint8_t*)(x))[1] << 16) | \
                   (((uint8_t*)(x))[2] << 8) | \
                    ((uint8_t*)(x))[3])

#define ADVANCE_BLOCK() \
{ \
    pixel_ptr += 4; \
    if (pixel_ptr >= width) \
    { \
        pixel_ptr = 0; \
        row_ptr += stride * 4; \
    } \
    total_blocks--; \
    if (total_blocks < 0) \
    { \
        av_log(s->avctx, AV_LOG_ERROR, "warning: block counter just went negative (this should not happen)\n"); \
        return; \
    } \
}

static void rpza_decode_stream(RpzaContext *s)
{
    int width = s->avctx->width;
    int stride = s->frame.linesize[0] / 2;
    int row_inc = stride - 4;
    int stream_ptr = 0;
    int chunk_size;
    unsigned char opcode;
    int n_blocks;
    unsigned short colorA = 0, colorB;
    unsigned short color4[4];
    unsigned char index, idx;
    unsigned short ta, tb;
    unsigned short *pixels = (unsigned short *)s->frame.data[0];
    unsigned short *prev_pixels = (unsigned short *)s->prev_frame.data[0];

    int row_ptr = 0;
    int pixel_ptr = 0;
    int block_ptr;
    int pixel_x, pixel_y;
    int total_blocks;

    /* First byte is always 0xe1. Warn if it's different */
    if (s->buf[stream_ptr] != 0xe1)
        av_log(s->avctx, AV_LOG_ERROR, "First chunk byte is 0x%02x instead of 0x1e\n",
            s->buf[stream_ptr]);

    /* Get chunk size, ingnoring first byte */
    chunk_size = BE_32(&s->buf[stream_ptr]) & 0x00FFFFFF;
    stream_ptr += 4;

    /* If length mismatch use size from MOV file and try to decode anyway */
    if (chunk_size != s->size)
        av_log(s->avctx, AV_LOG_ERROR, "MOV chunk size != encoded chunk size; using MOV chunk size\n");

    chunk_size = s->size;

    /* Number of 4x4 blocks in frame. */
    total_blocks = (s->avctx->width * s->avctx->height) / (4 * 4);

    /* Process chunk data */
    while (stream_ptr < chunk_size) {
        opcode = s->buf[stream_ptr++]; /* Get opcode */

        n_blocks = (opcode & 0x1f) + 1; /* Extract block counter from opcode */

        /* If opcode MSbit is 0, we need more data to decide what to do */
        if ((opcode & 0x80) == 0) {
            colorA = (opcode << 8) | (s->buf[stream_ptr++]);
            opcode = 0;
            if ((s->buf[stream_ptr] & 0x80) != 0) {
                /* Must behave as opcode 110xxxxx, using colorA computed 
                 * above. Use fake opcode 0x20 to enter switch block at 
                 * the right place */
                opcode = 0x20;
                n_blocks = 1;
            }
        }

        switch (opcode & 0xe0) {

        /* Skip blocks */
        case 0x80:
            while (n_blocks--) {
                block_ptr = row_ptr + pixel_ptr;
                for (pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (pixel_x = 0; pixel_x < 4; pixel_x++){
                        pixels[block_ptr] = prev_pixels[block_ptr];
                        block_ptr++;
                    }
                    block_ptr += row_inc;
                }
                ADVANCE_BLOCK();
            }
            break;

        /* Fill blocks with one color */
        case 0xa0:
            colorA = BE_16 (&s->buf[stream_ptr]);
            stream_ptr += 2;
            while (n_blocks--) {
                block_ptr = row_ptr + pixel_ptr;
                for (pixel_y = 0; pixel_y < 4; pixel_y++) {
                    for (pixel_x = 0; pixel_x < 4; pixel_x++){
                        pixels[block_ptr] = colorA;
                        block_ptr++;
                    }
                    block_ptr += row_inc;
                }
                ADVANCE_BLOCK();
            }
            break;

        /* Fill blocks with 4 colors */
        case 0xc0:
            colorA = BE_16 (&s->buf[stream_ptr]);
            stream_ptr += 2;
        case 0x20:
            colorB = BE_16 (&s->buf[stream_ptr]);
            stream_ptr += 2;

            /* sort out the colors */
            color4[0] = colorB;
            color4[1] = 0;
            color4[2] = 0;
            color4[3] = colorA;

            /* red components */
            ta = (colorA >> 10) & 0x1F;
            tb = (colorB >> 10) & 0x1F;
            color4[1] |= ((11 * ta + 21 * tb) >> 5) << 10;
            color4[2] |= ((21 * ta + 11 * tb) >> 5) << 10;

            /* green components */
            ta = (colorA >> 5) & 0x1F;
            tb = (colorB >> 5) & 0x1F;
            color4[1] |= ((11 * ta + 21 * tb) >> 5) << 5;
            color4[2] |= ((21 * ta + 11 * tb) >> 5) << 5;

            /* blue components */
            ta = colorA & 0x1F;
            tb = colorB & 0x1F;
            color4[1] |= ((11 * ta + 21 * tb) >> 5);
            color4[2] |= ((21 * ta + 11 * tb) >> 5);

            while (n_blocks--) {
                block_ptr = row_ptr + pixel_ptr;
                for (pixel_y = 0; pixel_y < 4; pixel_y++) {
                    index = s->buf[stream_ptr++];
                    for (pixel_x = 0; pixel_x < 4; pixel_x++){
                        idx = (index >> (2 * (3 - pixel_x))) & 0x03;
                        pixels[block_ptr] = color4[idx];
                        block_ptr++;
                    }
                    block_ptr += row_inc;
                }
                ADVANCE_BLOCK();
            }
            break;

        /* Fill block with 16 colors */
        case 0x00:
            block_ptr = row_ptr + pixel_ptr;
            for (pixel_y = 0; pixel_y < 4; pixel_y++) {
                for (pixel_x = 0; pixel_x < 4; pixel_x++){
                    /* We already have color of upper left pixel */
                    if ((pixel_y != 0) || (pixel_x !=0)) {
                        colorA = BE_16 (&s->buf[stream_ptr]);
                        stream_ptr += 2;
                    }
                    pixels[block_ptr] = colorA;
                    block_ptr++;
                }
                block_ptr += row_inc;
            }
            ADVANCE_BLOCK();
            break;

        /* Unknown opcode */
        default:
            av_log(s->avctx, AV_LOG_ERROR, "Unknown opcode %d in rpza chunk."
                 " Skip remaining %d bytes of chunk data.\n", opcode,
                 chunk_size - stream_ptr);
            return;
        } /* Opcode switch */
    }
}

static int rpza_decode_init(AVCodecContext *avctx)
{
    RpzaContext *s = (RpzaContext *)avctx->priv_data;

    s->avctx = avctx;
    avctx->pix_fmt = PIX_FMT_RGB555;
    avctx->has_b_frames = 0;
    dsputil_init(&s->dsp, avctx);

    s->frame.data[0] = s->prev_frame.data[0] = NULL;

    return 0;
}

static int rpza_decode_frame(AVCodecContext *avctx,
                             void *data, int *data_size,
                             uint8_t *buf, int buf_size)
{
    RpzaContext *s = (RpzaContext *)avctx->priv_data;

    s->buf = buf;
    s->size = buf_size;

    s->frame.reference = 1;
    if (avctx->get_buffer(avctx, &s->frame)) {
        av_log(avctx, AV_LOG_ERROR, "  RPZA Video: get_buffer() failed\n");
        return -1;
    }

    rpza_decode_stream(s);

    if (s->prev_frame.data[0])
        avctx->release_buffer(avctx, &s->prev_frame);

    /* shuffle frames */
    s->prev_frame = s->frame;

    *data_size = sizeof(AVFrame);
    *(AVFrame*)data = s->frame;

    /* always report that the buffer was completely consumed */
    return buf_size;
}

static int rpza_decode_end(AVCodecContext *avctx)
{
    RpzaContext *s = (RpzaContext *)avctx->priv_data;

    if (s->prev_frame.data[0])
        avctx->release_buffer(avctx, &s->prev_frame);

    return 0;
}

AVCodec rpza_decoder = {
    "rpza",
    CODEC_TYPE_VIDEO,
    CODEC_ID_RPZA,
    sizeof(RpzaContext),
    rpza_decode_init,
    NULL,
    rpza_decode_end,
    rpza_decode_frame,
    CODEC_CAP_DR1,
};
