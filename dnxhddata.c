/*
 * VC3/DNxHD data.
 * Copyright (c) 2007 SmartJog S.A., Baptiste Coudurier <baptiste dot coudurier at smartjog dot com>.
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

#include "dnxhddata.h"

static const uint8_t dnxhd_1237_luma_weigth[] = {
     0,  32,  33,  34, 34, 36, 37, 36,
    36,  37,  38,  38, 38, 39, 41, 44,
    43,  41,  40,  41, 46, 49, 47, 46,
    47,  49,  51,  54, 60, 62, 59, 55,
    54,  56,  58,  61, 65, 66, 64, 63,
    66,  73,  78,  79, 80, 79, 78, 78,
    82,  87,  89,  90, 93, 95, 96, 97,
    97, 100, 104, 102, 98, 98, 99, 99,
};

static const uint8_t dnxhd_1237_chroma_weigth[] = {
     0,  32,  36,  39, 39, 38, 39,  41,
    45,  51,  57,  58, 53, 48, 47,  51,
    55,  58,  66,  75, 81, 83, 82,  78,
    73,  72,  74,  77, 83, 85, 83,  82,
    89,  99,  96,  90, 94, 97, 99, 105,
   109, 105,  95,  89, 92, 95, 94,  93,
    92,  88,  89,  90, 93, 95, 96,  97,
    97, 100, 104, 102, 98, 98, 99,  99,
};

static const uint8_t dnxhd_1238_luma_weigth[] = {
     0, 32, 32, 33, 34, 33, 33, 33,
    33, 33, 33, 33, 33, 35, 37, 37,
    36, 36, 35, 36, 38, 38, 36, 35,
    36, 37, 38, 41, 42, 41, 39, 38,
    38, 38, 39, 41, 42, 41, 39, 39,
    40, 41, 43, 44, 44, 44, 44, 44,
    45, 47, 47, 47, 49, 50, 51, 51,
    51, 53, 55, 57, 58, 59, 57, 57,
};

static const uint8_t dnxhd_1238_chroma_weigth[] = {
     0, 32, 35, 35, 35, 34, 34, 35,
    39, 43, 45, 45, 41, 39, 40, 41,
    42, 44, 48, 55, 59, 63, 65, 59,
    53, 52, 52, 55, 61, 62, 58, 58,
    63, 66, 66, 65, 70, 74, 70, 66,
    65, 68, 75, 77, 74, 74, 77, 76,
    73, 73, 73, 73, 76, 80, 89, 90,
    82, 77, 80, 86, 84, 82, 82, 82,
};

static const uint8_t dnxhd_1243_luma_weigth[] = {
     0, 32, 32, 33, 33, 35, 35, 35,
    35, 35, 35, 35, 34, 35, 38, 40,
    39, 37, 37, 37, 36, 35, 36, 38,
    40, 41, 42, 44, 45, 44, 42, 41,
    40, 38, 36, 36, 37, 38, 40, 43,
    44, 45, 45, 45, 45, 45, 45, 41,
    39, 41, 45, 47, 47, 48, 48, 48,
    46, 44, 45, 47, 47, 48, 47, 47,
};

static const uint8_t dnxhd_1243_chroma_weigth[] = {
     0, 32, 36, 37, 36, 37, 39, 39,
    41, 43, 43, 42, 41, 41, 41, 42,
    43, 43, 43, 44, 44, 44, 46, 47,
    46, 45, 45, 45, 45, 46, 44, 44,
    45, 44, 42, 41, 43, 46, 45, 44,
    45, 45, 45, 46, 46, 46, 45, 44,
    45, 44, 45, 47, 47, 48, 49, 48,
    46, 45, 46, 47, 47, 48, 47, 47,
};

static const uint8_t dnxhd_1237_dc_codes[12] = {
    0, 12, 13, 1, 2, 3, 4, 5, 14, 30, 62, 63,
};

static const uint8_t dnxhd_1237_dc_bits[12] = {
    3, 4, 4, 3, 3, 3, 3, 3, 4, 5, 6, 6,
};

static const uint16_t dnxhd_1237_ac_codes[257] = {
    0, 1, 4, 5, 12, 26, 27, 56, 57, 58, 59, 120, 121, 244, 245, 246, 247, 248, 498, 499, 500, 501, 502, 1006, 1007, 1008, 1009, 1010, 1011, 2024, 2025, 2026, 2027, 2028, 2029, 2030, 2031, 4064, 4065, 4066, 4067, 4068, 4069, 4070, 4071, 4072, 4073, 8148, 8149, 8150, 8151, 8152, 8153, 8154, 8155, 8156, 8157, 8158, 16318, 16319, 16320, 16321, 16322, 16323, 16324, 16325, 16326, 16327, 16328, 16329, 16330, 16331, 16332, 16333, 32668, 32669, 32670, 32671, 32672, 32673, 32674, 32675, 32676, 32677, 32678, 32679, 32680, 32681, 32682, 32683, 32684, 65370, 65371, 65372, 65373, 65374, 65375, 65376, 65377, 65378, 65379, 65380, 65381, 65382, 65383, 65384, 65385, 65386, 65387, 65388, 65389, 65390, 65391, 65392, 65393, 65394, 65395, 65396, 65397, 65398, 65399, 65400, 65401, 65402, 65403, 65404, 65405, 65406, 65407, 65408, 65409, 65410, 65411, 65412, 65413, 65414, 65415, 65416, 65417, 65418, 65419, 65420, 65421, 65422, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 65470, 65471, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 65479, 65480, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 65488, 65489, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 65515, 65516, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 65525, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534, 65535,
};

static const uint8_t dnxhd_1237_ac_bits[257] = {
    2, 2, 3, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

static const uint8_t dnxhd_1237_ac_level[257] = {
    1, 1, 2, 0, 3, 4, 2, 5, 6, 7, 3, 8, 9, 10, 11, 12, 4, 5, 13, 14, 15, 16, 6, 17, 18, 19, 20, 21, 7, 22, 23, 24, 25, 26, 27, 8, 9, 28, 29, 30, 31, 32, 33, 34, 10, 11, 12, 35, 36, 37, 38, 39, 40, 41, 13, 14, 15, 16, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 17, 18, 19, 20, 21, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 1, 22, 23, 24, 25, 26, 27, 62, 63, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
};

static const uint8_t dnxhd_1237_ac_run_flag[257] = {
    0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const uint8_t dnxhd_1237_ac_index_flag[257] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const uint16_t dnxhd_1237_run_codes[62] = {
    0, 4, 10, 11, 24, 25, 26, 54, 55, 56, 57, 58, 118, 119, 240, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023,
};

static const uint8_t dnxhd_1237_run_bits[62] = {
    1, 3, 4, 4, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};

static const uint8_t dnxhd_1237_run[62] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 53, 57, 58, 59, 60, 61, 62, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55, 56,
};

static const uint8_t dnxhd_1238_dc_codes[12] = {
    0, 12, 13, 1, 2, 3, 4, 5, 14, 30, 62, 63,
};

static const uint8_t dnxhd_1238_dc_bits[12] = {
    3, 4, 4, 3, 3, 3, 3, 3, 4, 5, 6, 6,
};

static const uint16_t dnxhd_1238_ac_codes[257] = {
    0, 1, 4, 10, 11, 24, 25, 26, 54, 55, 56, 57, 116, 117, 118, 119, 240, 241, 242, 243, 244, 245, 492, 493, 494, 495, 496, 497, 498, 499, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026, 2027, 4056, 4057, 4058, 4059, 4060, 4061, 4062, 4063, 4064, 4065, 4066, 4067, 4068, 4069, 8140, 8141, 8142, 8143, 8144, 8145, 8146, 8147, 8148, 8149, 8150, 8151, 8152, 8153, 8154, 8155, 8156, 16314, 16315, 16316, 16317, 16318, 16319, 16320, 16321, 16322, 16323, 16324, 16325, 16326, 16327, 16328, 16329, 16330, 16331, 16332, 16333, 16334, 16335, 16336, 16337, 16338, 32678, 32679, 32680, 32681, 32682, 32683, 32684, 32685, 32686, 32687, 32688, 32689, 32690, 32691, 32692, 32693, 32694, 32695, 32696, 32697, 32698, 32699, 32700, 32701, 32702, 32703, 32704, 32705, 65412, 65413, 65414, 65415, 65416, 65417, 65418, 65419, 65420, 65421, 65422, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 65463, 65464, 65465, 65466, 65467, 65468, 65469, 65470, 65471, 65472, 65473, 65474, 65475, 65476, 65477, 65478, 65479, 65480, 65481, 65482, 65483, 65484, 65485, 65486, 65487, 65488, 65489, 65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 65499, 65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 65508, 65509, 65510, 65511, 65512, 65513, 65514, 65515, 65516, 65517, 65518, 65519, 65520, 65521, 65522, 65523, 65524, 65525, 65526, 65527, 65528, 65529, 65530, 65531, 65532, 65533, 65534, 65535,
};

static const uint8_t dnxhd_1238_ac_bits[257] = {
    2, 2, 3, 4, 4, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

static const uint8_t dnxhd_1238_ac_level[257] = {
    1, 1, 2, 3, 0, 4, 5, 2, 6, 7, 8, 3, 9, 10, 11, 4, 12, 13, 14, 15, 16, 5, 17, 18, 19, 20, 21, 22, 6, 7, 23, 24, 25, 26, 27, 28, 29, 8, 9, 30, 31, 32, 33, 34, 35, 36, 37, 10, 11, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 12, 13, 14, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 15, 16, 17, 18, 62, 63, 64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 19, 20, 21, 22, 23, 24, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 40, 25, 26, 27, 28, 29, 30, 38, 39, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
}; /* 0 is EOB */

static const uint8_t dnxhd_1238_ac_run_flag[257] = {
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const uint8_t dnxhd_1238_ac_index_flag[257] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

static const uint16_t dnxhd_1238_run_codes[62] = {
    0, 4, 10, 11, 24, 25, 26, 27, 56, 57, 58, 59, 120, 242, 486, 487, 488, 489, 980, 981, 982, 983, 984, 985, 986, 987, 988, 989, 990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023,
};

static const uint8_t dnxhd_1238_run_bits[62] = {
    1, 3, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
};

static const uint8_t dnxhd_1238_run[62] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 20, 21, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
};

const CIDEntry ff_dnxhd_cid_table[] = {
    { 1237, 1920, 1080, 0, 606208, 606208, 4, 8,
      dnxhd_1237_luma_weigth, dnxhd_1237_chroma_weigth,
      dnxhd_1237_dc_codes, dnxhd_1237_dc_bits,
      dnxhd_1237_ac_codes, dnxhd_1237_ac_bits, dnxhd_1237_ac_level,
      dnxhd_1237_ac_run_flag, dnxhd_1237_ac_index_flag,
      dnxhd_1237_run_codes, dnxhd_1237_run_bits, dnxhd_1237_run },
    { 1238, 1920, 1080, 0, 917504, 917504, 4, 8,
      dnxhd_1238_luma_weigth, dnxhd_1238_chroma_weigth,
      dnxhd_1238_dc_codes, dnxhd_1238_dc_bits,
      dnxhd_1238_ac_codes, dnxhd_1238_ac_bits, dnxhd_1238_ac_level,
      dnxhd_1238_ac_run_flag, dnxhd_1238_ac_index_flag,
      dnxhd_1238_run_codes, dnxhd_1238_run_bits, dnxhd_1238_run },
    { 1243, 1920, 1080, 1, 917504, 458752, 4, 8,
      dnxhd_1243_luma_weigth, dnxhd_1243_chroma_weigth,
      dnxhd_1238_dc_codes, dnxhd_1238_dc_bits,
      dnxhd_1238_ac_codes, dnxhd_1238_ac_bits, dnxhd_1238_ac_level,
      dnxhd_1238_ac_run_flag, dnxhd_1238_ac_index_flag,
      dnxhd_1238_run_codes, dnxhd_1238_run_bits, dnxhd_1238_run },
};

int ff_dnxhd_get_cid_table(int cid)
{
    int i;
    for (i = 0; i < sizeof(ff_dnxhd_cid_table)/sizeof(CIDEntry); i++)
        if (ff_dnxhd_cid_table[i].cid == cid)
            return i;
    return -1;
}
