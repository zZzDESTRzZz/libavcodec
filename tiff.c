/*
 * TIFF image decoder
 * Copyright (c) 2006 Konstantin Shishkov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */
#include "avcodec.h"
#ifdef CONFIG_ZLIB
#include <zlib.h>
#endif

/* abridged list of TIFF tags */
enum TiffTags{
    TIFF_WIDTH = 0x100,
    TIFF_HEIGHT,
    TIFF_BPP,
    TIFF_COMPR,
    TIFF_INVERT = 0x106,
    TIFF_STRIP_OFFS = 0x111,
    TIFF_ROWSPERSTRIP = 0x116,
    TIFF_STRIP_SIZE,
    TIFF_XPOS = 0x11E,
    TIFF_YPOS = 0x11F,
    TIFF_PREDICTOR = 0x13D
};

enum TiffCompr{
    TIFF_RAW = 1,
    TIFF_CCITT_RLE,
    TIFF_G3,
    TIFF_G4,
    TIFF_LZW,
    TIFF_JPEG,
    TIFF_NEWJPEG,
    TIFF_ADOBE_DEFLATE,
    TIFF_PACKBITS = 0x8005,
    TIFF_DEFLATE = 0x80B2
};

enum TiffTypes{
    TIFF_BYTE = 1,
    TIFF_STRING,
    TIFF_SHORT,
    TIFF_LONG,
    TIFF_LONGLONG
};

typedef struct TiffContext {
    AVCodecContext *avctx;
    AVFrame picture;

    int width, height;
    unsigned int bpp;
    int le;
    int compr;

    int strips, rps;
    int sot;
    uint8_t* stripdata;
    uint8_t* stripsizes;
    int stripsize, stripoff;
} TiffContext;

static int tget_short(uint8_t **p, int le){
    int v = le ? LE_16(*p) : BE_16(*p);
    *p += 2;
    return v;
}

static int tget_long(uint8_t **p, int le){
    int v = le ? LE_32(*p) : BE_32(*p);
    *p += 4;
    return v;
}

static int tget(uint8_t **p, int type, int le){
    switch(type){
    case TIFF_BYTE : return *(*p)++;
    case TIFF_SHORT: return tget_short(p, le);
    case TIFF_LONG : return tget_long (p, le);
    default        : return -1;
    }
}

static int tiff_unpack_strip(TiffContext *s, uint8_t* dst, int stride, uint8_t *src, int size, int lines){
    int c, line, pixels, code;
    uint8_t *ssrc = src;
    int width = s->width * (s->bpp / 8);
#ifdef CONFIG_ZLIB
    uint8_t *zbuf; unsigned long outlen;

    if(s->compr == TIFF_DEFLATE || s->compr == TIFF_ADOBE_DEFLATE){
        outlen = width * lines;
        if(lines != s->height){
            av_log(s->avctx, AV_LOG_ERROR, "This decoder won't decode ZLib-packed TIFF with %i lines per strip\n", lines);
            return -1;
        }
        zbuf = av_malloc(outlen);
        if(uncompress(zbuf, &outlen, src, size) != Z_OK){
            av_log(s->avctx, AV_LOG_ERROR, "Uncompressing failed (%lu of %lu)\n", outlen, (unsigned long)width * lines);
            av_free(zbuf);
            return -1;
        }
        src = zbuf;
        for(line = 0; line < lines; line++){
            memcpy(dst, src, width);
            dst += stride;
            src += width;
        }
        av_free(zbuf);
        return 0;
    }
#endif
    for(line = 0; line < lines; line++){
        if(src - ssrc > size){
            av_log(s->avctx, AV_LOG_ERROR, "Source data overread\n");
            return -1;
        }
        switch(s->compr){
        case TIFF_RAW:
            memcpy(dst, src, s->width * (s->bpp / 8));
            src += s->width * (s->bpp / 8);
            break;
        case TIFF_PACKBITS:
            for(pixels = 0; pixels < width;){
                code = (int8_t)*src++;
                if(code >= 0){
                    code++;
                    if(pixels + code > width){
                        av_log(s->avctx, AV_LOG_ERROR, "Copy went out of bounds\n");
                        return -1;
                    }
                    memcpy(dst + pixels, src, code);
                    src += code;
                    pixels += code;
                }else if(code != -128){ // -127..-1
                    code = (-code) + 1;
                    if(pixels + code > width){
                        av_log(s->avctx, AV_LOG_ERROR, "Run went out of bounds\n");
                        return -1;
                    }
                    c = *src++;
                    memset(dst + pixels, c, code);
                    pixels += code;
                }
            }
            break;
        }
        dst += stride;
    }
    return 0;
}


static int tiff_decode_tag(TiffContext *s, uint8_t *start, uint8_t *buf, uint8_t *end_buf, AVFrame *pic)
{
    int tag, type, count, off, value = 0;
    uint8_t *src, *dst;
    int i, j, ssize, soff, stride;

    tag = tget_short(&buf, s->le);
    type = tget_short(&buf, s->le);
    count = tget_long(&buf, s->le);
    off = tget_long(&buf, s->le);

    if(count == 1){
        switch(type){
        case TIFF_BYTE:
        case TIFF_SHORT:
            buf -= 4;
            value = tget(&buf, type, s->le);
            buf = NULL;
            break;
        case TIFF_LONG:
            value = off;
            buf = NULL;
            break;
        default:
            value = -1;
            buf = start + off;
        }
    }else{
        buf = start + off;
    }

    if(buf && (buf < start || buf > end_buf)){
        av_log(s->avctx, AV_LOG_ERROR, "Tag referencing position outside the image\n");
        return -1;
    }

    switch(tag){
    case TIFF_WIDTH:
        s->width = value;
        break;
    case TIFF_HEIGHT:
        s->height = value;
        s->avctx->pix_fmt = PIX_FMT_RGB24;
        if(s->width != s->avctx->width || s->height != s->avctx->height){
            if(avcodec_check_dimensions(s->avctx, s->width, s->height))
                return -1;
            avcodec_set_dimensions(s->avctx, s->width, s->height);
        }
        if(s->picture.data[0])
            s->avctx->release_buffer(s->avctx, &s->picture);
        if(s->avctx->get_buffer(s->avctx, &s->picture) < 0){
            av_log(s->avctx, AV_LOG_ERROR, "get_buffer() failed\n");
            return -1;
        }
        break;
    case TIFF_BPP:
        if(count == 1) s->bpp = value;
        else{
            switch(type){
            case TIFF_BYTE:
                s->bpp = (off & 0xFF) + ((off >> 8) & 0xFF) + ((off >> 16) & 0xFF);
                break;
            case TIFF_SHORT:
            case TIFF_LONG:
                s->bpp = tget(&buf, type, s->le) + tget(&buf, type, s->le) + tget(&buf, type, s->le);
                break;
            default:
                s->bpp = -1;
            }
        }
        if(s->bpp != 24){
            av_log(s->avctx, AV_LOG_ERROR, "Only RGB24 is supported\n");
            return -1;
        }
        break;
    case TIFF_COMPR:
        s->compr = value;
        switch(s->compr){
        case TIFF_RAW:
        case TIFF_PACKBITS:
            break;
        case TIFF_DEFLATE:
        case TIFF_ADOBE_DEFLATE:
#ifdef CONFIG_ZLIB
            break;
#else
            av_log(s->avctx, AV_LOG_ERROR, "Deflate: ZLib not compiled in\n");
            return -1;
#endif
        case TIFF_LZW:
            av_log(s->avctx, AV_LOG_ERROR, "LZW: not implemented yet\n");
            return -1;
        case TIFF_G3:
            av_log(s->avctx, AV_LOG_ERROR, "CCITT G3 compression is not supported\n");
            return -1;
        case TIFF_G4:
            av_log(s->avctx, AV_LOG_ERROR, "CCITT G4 compression is not supported\n");
            return -1;
        case TIFF_CCITT_RLE:
            av_log(s->avctx, AV_LOG_ERROR, "CCITT RLE compression is not supported\n");
            return -1;
        default:
            av_log(s->avctx, AV_LOG_ERROR, "Unknown compression method %i\n", s->compr);
            return -1;
        }
        break;
    case TIFF_ROWSPERSTRIP:
        if(value < 1 || value > s->height){
            av_log(s->avctx, AV_LOG_ERROR, "Incorrect value of rows per strip\n");
            return -1;
        }
        s->rps = value;
        break;
    case TIFF_STRIP_OFFS:
        if(count == 1){
            s->stripdata = NULL;
            s->stripoff = value;
        }else
            s->stripdata = start + off;
        s->strips = count;
        s->sot = type;
        if(s->stripdata > end_buf){
            av_log(s->avctx, AV_LOG_ERROR, "Tag referencing position outside the image\n");
            return -1;
        }
        break;
    case TIFF_STRIP_SIZE:
        if(count == 1){
            s->stripsizes = NULL;
            s->stripsize = value;
            s->strips = 1;
        }else{
            s->stripsizes = start + off;
        }
        s->strips = count;
        if(s->stripsizes > end_buf){
            av_log(s->avctx, AV_LOG_ERROR, "Tag referencing position outside the image\n");
            return -1;
        }
        if(!pic->data[0]){
            av_log(s->avctx, AV_LOG_ERROR, "Picture initialization missing\n");
            return -1;
        }
        /* now we have the data and may start decoding */
        stride = pic->linesize[0];
        dst = pic->data[0];
        for(i = 0; i < s->height; i += s->rps){
            if(s->stripsizes)
                ssize = tget(&s->stripsizes, type, s->le);
            else
                ssize = s->stripsize;

            if(s->stripdata){
                soff = tget(&s->stripdata, s->sot, s->le);
            }else
                soff = s->stripoff;
            src = start + soff;
            if(tiff_unpack_strip(s, dst, stride, src, ssize, FFMIN(s->rps, s->height - i)) < 0)
                break;
            dst += s->rps * stride;
        }
        break;
    case TIFF_PREDICTOR:
        if(!pic->data[0]){
            av_log(s->avctx, AV_LOG_ERROR, "Picture initialization missing\n");
            return -1;
        }
        if(value == 2){
            src = pic->data[0];
            stride = pic->linesize[0];
            soff = s->bpp >> 3;
            ssize = s->width * soff;
            for(i = 0; i < s->height; i++) {
                for(j = soff; j < ssize; j++)
                    src[j] += src[j - soff];
                src += stride;
            }
        }
        break;
    }
    return 0;
}

static int decode_frame(AVCodecContext *avctx,
                        void *data, int *data_size,
                        uint8_t *buf, int buf_size)
{
    TiffContext * const s = avctx->priv_data;
    AVFrame *picture = data;
    AVFrame * const p= (AVFrame*)&s->picture;
    uint8_t *orig_buf = buf, *end_buf = buf + buf_size;
    int id, le, off;
    int i, entries;

    //parse image header
    id = LE_16(buf); buf += 2;
    if(id == 0x4949) le = 1;
    else if(id == 0x4D4D) le = 0;
    else{
        av_log(avctx, AV_LOG_ERROR, "TIFF header not found\n");
        return -1;
    }
    s->le = le;
    // As TIFF 6.0 specification puts it "An arbitrary but carefully chosen number
    // that further identifies the file as a TIFF file"
    if(tget_short(&buf, le) != 42){
        av_log(avctx, AV_LOG_ERROR, "The answer to life, universe and everything is not correct!\n");
        return -1;
    }
    /* parse image file directory */
    off = tget_long(&buf, le);
    if(orig_buf + off + 14 >= end_buf){
        av_log(avctx, AV_LOG_ERROR, "IFD offset is greater than image size\n");
        return -1;
    }
    buf = orig_buf + off;
    entries = tget_short(&buf, le);
    for(i = 0; i < entries; i++){
        if(tiff_decode_tag(s, orig_buf, buf, end_buf, p) < 0)
            return -1;
        buf += 12;
    }

    *picture= *(AVFrame*)&s->picture;
    *data_size = sizeof(AVPicture);

    return buf_size;
}

static int tiff_init(AVCodecContext *avctx){
    TiffContext *s = avctx->priv_data;

    s->width = 0;
    s->height = 0;
    s->avctx = avctx;
    avcodec_get_frame_defaults((AVFrame*)&s->picture);
    avctx->coded_frame= (AVFrame*)&s->picture;
    s->picture.data[0] = NULL;

    return 0;
}

static int tiff_end(AVCodecContext *avctx)
{
    TiffContext * const s = avctx->priv_data;

    if(s->picture.data[0])
        avctx->release_buffer(avctx, &s->picture);
    return 0;
}

AVCodec tiff_decoder = {
    "tiff",
    CODEC_TYPE_VIDEO,
    CODEC_ID_TIFF,
    sizeof(TiffContext),
    tiff_init,
    NULL,
    tiff_end,
    decode_frame,
    0,
    NULL
};
