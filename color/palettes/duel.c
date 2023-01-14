#include "duel.h"

palette_t duel_init() {
  return (palette_t) {
    .num = 256,
    .colors = {
      (rgb_t) {0x00, 0x00, 0x00},
      (rgb_t) {0x22, 0x23, 0x23},
      (rgb_t) {0x43, 0x45, 0x49},
      (rgb_t) {0x62, 0x68, 0x71},
      (rgb_t) {0x82, 0x8b, 0x98},
      (rgb_t) {0xa6, 0xae, 0xba},
      (rgb_t) {0xcd, 0xd2, 0xda},
      (rgb_t) {0xf5, 0xf7, 0xfa},
      (rgb_t) {0x62, 0x5d, 0x54},
      (rgb_t) {0x85, 0x75, 0x65},
      (rgb_t) {0x9e, 0x8c, 0x79},
      (rgb_t) {0xae, 0xa1, 0x89},
      (rgb_t) {0xbb, 0xaf, 0xa4},
      (rgb_t) {0xcc, 0xc3, 0xb1},
      (rgb_t) {0xea, 0xdb, 0xc9},
      (rgb_t) {0xff, 0xf3, 0xd6},
      (rgb_t) {0x58, 0x31, 0x26},
      (rgb_t) {0x73, 0x3d, 0x3b},
      (rgb_t) {0x88, 0x50, 0x41},
      (rgb_t) {0x9a, 0x62, 0x4c},
      (rgb_t) {0xad, 0x6e, 0x51},
      (rgb_t) {0xd5, 0x8d, 0x6b},
      (rgb_t) {0xfb, 0xaa, 0x84},
      (rgb_t) {0xff, 0xce, 0x7f},
      (rgb_t) {0x00, 0x27, 0x35},
      (rgb_t) {0x00, 0x38, 0x50},
      (rgb_t) {0x00, 0x4d, 0x5e},
      (rgb_t) {0x0b, 0x66, 0x7f},
      (rgb_t) {0x00, 0x6f, 0x89},
      (rgb_t) {0x32, 0x8c, 0xa7},
      (rgb_t) {0x24, 0xae, 0xd6},
      (rgb_t) {0x88, 0xd6, 0xff},
      (rgb_t) {0x66, 0x2b, 0x29},
      (rgb_t) {0x94, 0x36, 0x3a},
      (rgb_t) {0xb6, 0x4d, 0x46},
      (rgb_t) {0xcd, 0x5e, 0x46},
      (rgb_t) {0xe3, 0x78, 0x40},
      (rgb_t) {0xf9, 0x9b, 0x4e},
      (rgb_t) {0xff, 0xbc, 0x4e},
      (rgb_t) {0xff, 0xe9, 0x49},
      (rgb_t) {0x28, 0x2b, 0x4a},
      (rgb_t) {0x3a, 0x45, 0x68},
      (rgb_t) {0x61, 0x5f, 0x84},
      (rgb_t) {0x7a, 0x77, 0x99},
      (rgb_t) {0x86, 0x90, 0xb2},
      (rgb_t) {0x96, 0xb2, 0xd9},
      (rgb_t) {0xc7, 0xd6, 0xff},
      (rgb_t) {0xc6, 0xec, 0xff},
      (rgb_t) {0x00, 0x22, 0x19},
      (rgb_t) {0x00, 0x32, 0x21},
      (rgb_t) {0x17, 0x4a, 0x1b},
      (rgb_t) {0x22, 0x59, 0x18},
      (rgb_t) {0x2f, 0x69, 0x0c},
      (rgb_t) {0x51, 0x88, 0x22},
      (rgb_t) {0x7d, 0xa4, 0x2d},
      (rgb_t) {0xa6, 0xcc, 0x34},
      (rgb_t) {0x18, 0x1f, 0x2f},
      (rgb_t) {0x23, 0x32, 0x4d},
      (rgb_t) {0x25, 0x46, 0x6b},
      (rgb_t) {0x36, 0x6b, 0x8a},
      (rgb_t) {0x31, 0x8e, 0xb8},
      (rgb_t) {0x41, 0xb2, 0xe3},
      (rgb_t) {0x52, 0xd2, 0xff},
      (rgb_t) {0x74, 0xf5, 0xfd},
      (rgb_t) {0x1a, 0x33, 0x2c},
      (rgb_t) {0x2f, 0x3f, 0x38},
      (rgb_t) {0x38, 0x51, 0x40},
      (rgb_t) {0x32, 0x5c, 0x40},
      (rgb_t) {0x41, 0x74, 0x55},
      (rgb_t) {0x49, 0x89, 0x60},
      (rgb_t) {0x55, 0xb6, 0x7d},
      (rgb_t) {0x91, 0xda, 0xa1},
      (rgb_t) {0x5e, 0x07, 0x11},
      (rgb_t) {0x82, 0x21, 0x1d},
      (rgb_t) {0xb6, 0x3c, 0x35},
      (rgb_t) {0xe4, 0x5c, 0x5f},
      (rgb_t) {0xff, 0x76, 0x76},
      (rgb_t) {0xff, 0x9b, 0xa8},
      (rgb_t) {0xff, 0xbb, 0xc7},
      (rgb_t) {0xff, 0xdb, 0xff},
      (rgb_t) {0x2d, 0x31, 0x36},
      (rgb_t) {0x48, 0x47, 0x4d},
      (rgb_t) {0x5b, 0x5c, 0x69},
      (rgb_t) {0x73, 0x73, 0x7f},
      (rgb_t) {0x84, 0x87, 0x95},
      (rgb_t) {0xab, 0xae, 0xbe},
      (rgb_t) {0xba, 0xc7, 0xdb},
      (rgb_t) {0xeb, 0xf0, 0xf6},
      (rgb_t) {0x3b, 0x30, 0x3c},
      (rgb_t) {0x5a, 0x3c, 0x45},
      (rgb_t) {0x8a, 0x52, 0x58},
      (rgb_t) {0xae, 0x6b, 0x60},
      (rgb_t) {0xc7, 0x82, 0x6c},
      (rgb_t) {0xd8, 0x9f, 0x75},
      (rgb_t) {0xec, 0xc5, 0x81},
      (rgb_t) {0xff, 0xfa, 0xab},
      (rgb_t) {0x31, 0x22, 0x2a},
      (rgb_t) {0x4a, 0x35, 0x3c},
      (rgb_t) {0x5e, 0x46, 0x46},
      (rgb_t) {0x72, 0x5a, 0x51},
      (rgb_t) {0x7e, 0x6c, 0x54},
      (rgb_t) {0x9e, 0x8a, 0x6e},
      (rgb_t) {0xc0, 0xa5, 0x88},
      (rgb_t) {0xdd, 0xbf, 0x9a},
      (rgb_t) {0x2e, 0x10, 0x26},
      (rgb_t) {0x49, 0x28, 0x3d},
      (rgb_t) {0x66, 0x36, 0x59},
      (rgb_t) {0x97, 0x54, 0x75},
      (rgb_t) {0xb9, 0x6d, 0x91},
      (rgb_t) {0xc1, 0x78, 0xaa},
      (rgb_t) {0xdb, 0x99, 0xbf},
      (rgb_t) {0xf8, 0xc6, 0xda},
      (rgb_t) {0x00, 0x2e, 0x49},
      (rgb_t) {0x00, 0x40, 0x51},
      (rgb_t) {0x00, 0x51, 0x62},
      (rgb_t) {0x00, 0x6b, 0x6d},
      (rgb_t) {0x00, 0x82, 0x79},
      (rgb_t) {0x00, 0xa0, 0x87},
      (rgb_t) {0x00, 0xbf, 0xa3},
      (rgb_t) {0x00, 0xde, 0xda},
      (rgb_t) {0x45, 0x31, 0x25},
      (rgb_t) {0x61, 0x4a, 0x3c},
      (rgb_t) {0x7e, 0x61, 0x44},
      (rgb_t) {0x99, 0x79, 0x51},
      (rgb_t) {0xb2, 0x90, 0x62},
      (rgb_t) {0xcc, 0xa9, 0x6e},
      (rgb_t) {0xe8, 0xcb, 0x82},
      (rgb_t) {0xfb, 0xea, 0xa3},
      (rgb_t) {0x5f, 0x09, 0x26},
      (rgb_t) {0x6e, 0x24, 0x34},
      (rgb_t) {0x90, 0x46, 0x47},
      (rgb_t) {0xa7, 0x60, 0x57},
      (rgb_t) {0xbd, 0x7d, 0x64},
      (rgb_t) {0xce, 0x97, 0x70},
      (rgb_t) {0xed, 0xb6, 0x7c},
      (rgb_t) {0xed, 0xd4, 0x93},
      (rgb_t) {0x32, 0x35, 0x58},
      (rgb_t) {0x4a, 0x52, 0x80},
      (rgb_t) {0x64, 0x65, 0x9d},
      (rgb_t) {0x78, 0x77, 0xc1},
      (rgb_t) {0x8e, 0x8c, 0xe2},
      (rgb_t) {0x9c, 0x9b, 0xef},
      (rgb_t) {0xb8, 0xae, 0xff},
      (rgb_t) {0xdc, 0xd4, 0xff},
      (rgb_t) {0x43, 0x17, 0x29},
      (rgb_t) {0x71, 0x2b, 0x3b},
      (rgb_t) {0x9f, 0x3b, 0x52},
      (rgb_t) {0xd9, 0x4a, 0x69},
      (rgb_t) {0xf8, 0x5d, 0x80},
      (rgb_t) {0xff, 0x7d, 0xaf},
      (rgb_t) {0xff, 0xa6, 0xc5},
      (rgb_t) {0xff, 0xcd, 0xff},
      (rgb_t) {0x49, 0x25, 0x1c},
      (rgb_t) {0x63, 0x34, 0x32},
      (rgb_t) {0x7c, 0x4b, 0x47},
      (rgb_t) {0x98, 0x59, 0x5a},
      (rgb_t) {0xac, 0x6f, 0x6e},
      (rgb_t) {0xc1, 0x7e, 0x7a},
      (rgb_t) {0xd2, 0x8d, 0x7a},
      (rgb_t) {0xe5, 0x9a, 0x7c},
      (rgb_t) {0x20, 0x29, 0x00},
      (rgb_t) {0x2f, 0x4f, 0x08},
      (rgb_t) {0x49, 0x5d, 0x00},
      (rgb_t) {0x61, 0x73, 0x08},
      (rgb_t) {0x7c, 0x83, 0x1e},
      (rgb_t) {0x96, 0x9a, 0x26},
      (rgb_t) {0xb4, 0xaa, 0x33},
      (rgb_t) {0xd0, 0xcc, 0x32},
      (rgb_t) {0x62, 0x2a, 0x00},
      (rgb_t) {0x75, 0x3b, 0x09},
      (rgb_t) {0x85, 0x4f, 0x12},
      (rgb_t) {0x9e, 0x65, 0x20},
      (rgb_t) {0xba, 0x88, 0x2e},
      (rgb_t) {0xd1, 0xaa, 0x39},
      (rgb_t) {0xe8, 0xd2, 0x4b},
      (rgb_t) {0xff, 0xf6, 0x4f},
      (rgb_t) {0x26, 0x23, 0x3d},
      (rgb_t) {0x3b, 0x38, 0x55},
      (rgb_t) {0x56, 0x50, 0x6f},
      (rgb_t) {0x75, 0x68, 0x6e},
      (rgb_t) {0x91, 0x7a, 0x7b},
      (rgb_t) {0xb3, 0x97, 0x83},
      (rgb_t) {0xcf, 0xaf, 0x8e},
      (rgb_t) {0xfe, 0xdf, 0xb1},
      (rgb_t) {0x1d, 0x2c, 0x43},
      (rgb_t) {0x2e, 0x3d, 0x47},
      (rgb_t) {0x39, 0x4d, 0x3c},
      (rgb_t) {0x4c, 0x5f, 0x33},
      (rgb_t) {0x58, 0x71, 0x2c},
      (rgb_t) {0x6b, 0x84, 0x2d},
      (rgb_t) {0x78, 0x9e, 0x24},
      (rgb_t) {0x7f, 0xbd, 0x39},
      (rgb_t) {0x37, 0x24, 0x23},
      (rgb_t) {0x53, 0x39, 0x3a},
      (rgb_t) {0x78, 0x4c, 0x49},
      (rgb_t) {0x94, 0x5d, 0x4f},
      (rgb_t) {0xa9, 0x6d, 0x58},
      (rgb_t) {0xbf, 0x7e, 0x63},
      (rgb_t) {0xd7, 0x93, 0x74},
      (rgb_t) {0xf4, 0xa3, 0x80},
      (rgb_t) {0x2d, 0x4b, 0x47},
      (rgb_t) {0x47, 0x65, 0x5a},
      (rgb_t) {0x5b, 0x7b, 0x69},
      (rgb_t) {0x71, 0x95, 0x7d},
      (rgb_t) {0x87, 0xae, 0x8e},
      (rgb_t) {0x8a, 0xc1, 0x96},
      (rgb_t) {0xa9, 0xd1, 0xc1},
      (rgb_t) {0xe0, 0xfa, 0xeb},
      (rgb_t) {0x00, 0x1b, 0x40},
      (rgb_t) {0x03, 0x31, 0x5f},
      (rgb_t) {0x07, 0x48, 0x7c},
      (rgb_t) {0x10, 0x5d, 0xa2},
      (rgb_t) {0x14, 0x76, 0xc0},
      (rgb_t) {0x40, 0x97, 0xea},
      (rgb_t) {0x55, 0xb1, 0xf1},
      (rgb_t) {0x6d, 0xcc, 0xff},
      (rgb_t) {0x55, 0x47, 0x69},
      (rgb_t) {0x76, 0x5d, 0x73},
      (rgb_t) {0x97, 0x74, 0x88},
      (rgb_t) {0xb9, 0x8c, 0x93},
      (rgb_t) {0xd5, 0xa3, 0x9a},
      (rgb_t) {0xeb, 0xbd, 0x9d},
      (rgb_t) {0xff, 0xd5, 0x9b},
      (rgb_t) {0xfd, 0xf7, 0x86},
      (rgb_t) {0x1d, 0x1d, 0x21},
      (rgb_t) {0x3c, 0x31, 0x51},
      (rgb_t) {0x58, 0x4a, 0x7f},
      (rgb_t) {0x79, 0x64, 0xba},
      (rgb_t) {0x95, 0x85, 0xf1},
      (rgb_t) {0xa9, 0x96, 0xec},
      (rgb_t) {0xba, 0xab, 0xf7},
      (rgb_t) {0xd1, 0xbd, 0xfe},
      (rgb_t) {0x26, 0x24, 0x50},
      (rgb_t) {0x28, 0x33, 0x5d},
      (rgb_t) {0x2d, 0x3d, 0x72},
      (rgb_t) {0x3d, 0x50, 0x83},
      (rgb_t) {0x51, 0x65, 0xae},
      (rgb_t) {0x52, 0x74, 0xc5},
      (rgb_t) {0x6c, 0x82, 0xc4},
      (rgb_t) {0x83, 0x93, 0xc3},
      (rgb_t) {0x49, 0x21, 0x29},
      (rgb_t) {0x5e, 0x41, 0x4a},
      (rgb_t) {0x77, 0x53, 0x5b},
      (rgb_t) {0x91, 0x60, 0x6a},
      (rgb_t) {0xad, 0x79, 0x84},
      (rgb_t) {0xb5, 0x8b, 0x94},
      (rgb_t) {0xd4, 0xae, 0xaa},
      (rgb_t) {0xff, 0xe2, 0xcf},
      (rgb_t) {0x72, 0x1c, 0x03},
      (rgb_t) {0x9c, 0x33, 0x27},
      (rgb_t) {0xbf, 0x5a, 0x3e},
      (rgb_t) {0xe9, 0x86, 0x27},
      (rgb_t) {0xff, 0xb1, 0x08},
      (rgb_t) {0xff, 0xcf, 0x05},
      (rgb_t) {0xff, 0xf0, 0x2b},
      (rgb_t) {0xf7, 0xf4, 0xbf}
    }
  };
}
