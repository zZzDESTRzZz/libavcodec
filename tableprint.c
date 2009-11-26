/*
 * Generate a file for hardcoded tables
 *
 * Copyright (c) 2009 Reimar Döffinger <Reimar.Doeffinger@gmx.de>
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
 */

#include <stdio.h>
#include <inttypes.h>
#include "tableprint.h"

#define WRITE_1D_FUNC(name, type, fmtstr, linebrk)\
void write_##name##_array(const void *arg, int len, int dummy)\
{\
    const type *data = arg;\
    int i;\
    printf("   ");\
    for (i = 0; i < len - 1; i++) {\
       printf(" "fmtstr",", data[i]);\
       if ((i & linebrk) == linebrk) printf("\n   ");\
    }\
    printf(" "fmtstr"\n", data[i]);\
}

WRITE_1D_FUNC(int8,   int8_t,   "%3"PRIi8, 15)
WRITE_1D_FUNC(uint32, uint32_t, "0x%08"PRIx32, 7)

#define WRITE_2D_FUNC(name, type)\
void write_##name##_2d_array(const void *arg, int len, int len2)\
{\
    const type *data = arg;\
    int i;\
    printf("    {\n");\
    for (i = 0; i < len; i++) {\
        write_##name##_array(data + i * len2, len2, 0);\
        printf(i == len - 1 ? "    }\n" : "    }, {\n");\
    }\
}

WRITE_2D_FUNC(int8,   int8_t)
WRITE_2D_FUNC(uint32, uint32_t)

int main(int argc, char *argv[])
{
    int i;

    printf("/* This file was generated by libavcodec/tableprint */\n");
    printf("#include <stdint.h>\n");
    tableinit();

    for (i = 0; tables[i].declaration; i++) {
        printf(tables[i].declaration);
        printf(" = {\n");
        tables[i].printfunc(tables[i].data, tables[i].size, tables[i].size2);
        printf("};\n");
    }
    return 0;
}
