#include <stdlib.h>

#include <string.h>
#include <math.h>

#include "nblessings.h"

// {" ", "▖", "▗", "▄", "▘", "▌", "▚", "▙", "▝", "▞", "▐", "▟", "▀", "▛", "▜", "█"};

#define BLOCK_CHAR_PREFIX (unsigned char) 0xe2, (unsigned char) 0x96

const unsigned char pixel_selector_byte[16] = {
  (unsigned char) 0x00, (unsigned char) 0x96, (unsigned char) 0x97, (unsigned char) 0x84,
  (unsigned char) 0x98, (unsigned char) 0x8c, (unsigned char) 0x9a, (unsigned char) 0x99,
  (unsigned char) 0x9d, (unsigned char) 0x9e, (unsigned char) 0x90, (unsigned char) 0x9f,
  (unsigned char) 0x80, (unsigned char) 0x9b, (unsigned char) 0x9c, (unsigned char) 0x88
};


#define ZERO_CHAR '0'
#define A_CHAR 'A'

#define ESC (unsigned char) 0x1B
#define CSI ESC, '['
#define OSC ESC, ']'
#define DCS ESC, 'P'
#define ST ESC, '\\'

#define START_ALTERNATE_BUFFER CSI, '?', '1', '0', '4', '9', 'h'
#define CLEAR_SCREEN CSI, '2', 'J'
#define RESET_HOME ESC, 'Y', '0', '0'
// #define SET_SCROLLING_REGION(TOP, BOTTOM) CSI, TOP, ';', BOTTOM, 'r'
// #define RESET_CHARACTER_ATTRIBUTES CSI, '0', 'm'

#define START_ALTERNATE_BUFFER_SIZE 8
#define LOAD_COLOR_SIZE 22

#define NBLESSINGS_HEADER_SIZE (START_ALTERNATE_BUFFER_SIZE + CLEAR_SCREEN_SIZE + LOAD_COLOR_SIZE * 256 + 1)

#define INT2CHARS(I) ZERO_CHAR + (unsigned char) (I / 100), ZERO_CHAR + (unsigned char) ((I % 100) / 10), ZERO_CHAR + (unsigned char) (I % 10)
#define HEX2CHAR(HEX) (HEX) > 9 ? (unsigned char) A_CHAR + (HEX) - 10 : (unsigned char) ZERO_CHAR + (HEX)
#define HEX2CHARS(HEX) HEX2CHAR((HEX >> 4) & 0x0F), HEX2CHAR(HEX & 0x0F)

#define LOAD_COLOR(I, R, G, B) OSC, '4', ';', INT2CHARS(I), ';', 'r', 'g', 'b', ':', HEX2CHARS((int) roundf(R)), '/', HEX2CHARS((int) roundf(G)), '/', HEX2CHARS((int) roundf(B)), ST
#define APPLY_FORE(I) '3', '8', ';', '5', ';', INT2CHARS(I)
#define APPLY_BACK(I) '4', '8', ';', '5', ';', INT2CHARS(I)

static
unsigned char nblessings_header[NBLESSINGS_HEADER_SIZE] = {
  START_ALTERNATE_BUFFER,
  CLEAR_SCREEN
};

unsigned char * nblessings_header_data(oklab_t * colors, unsigned int num_colors, unsigned int * header_len) {
  for(size_t i = 0; i < num_colors; i++) {
    rgb_t rgb = scale_denormalize_rgb(oklab_to_linear_srgb(colors[i]));
    memcpy(nblessings_header + START_ALTERNATE_BUFFER_SIZE + CLEAR_SCREEN_SIZE + i * LOAD_COLOR_SIZE, (unsigned char[]){LOAD_COLOR(i, rgb.r, rgb.g, rgb.b)}, LOAD_COLOR_SIZE * sizeof(unsigned char));
  }
  nblessings_header[START_ALTERNATE_BUFFER_SIZE + CLEAR_SCREEN_SIZE + num_colors * LOAD_COLOR_SIZE] = '\0';

  *header_len = START_ALTERNATE_BUFFER_SIZE + CLEAR_SCREEN_SIZE + num_colors * LOAD_COLOR_SIZE + 1;

  return nblessings_header;
}

unsigned char * nblessings_footer_data(unsigned int * footer_len) {
 return NULL;
}

unsigned int rasterize_frame(pixel_t * pixels, unsigned int num_pixels, unsigned int width, unsigned char * buffer) {
  unsigned int i = 0;
  memcpy(buffer + i, (unsigned char[]) {CLEAR_SCREEN}, CLEAR_SCREEN_SIZE);
  i += CLEAR_SCREEN_SIZE;
  unsigned int fore_color = pixels[0].fore;
  unsigned int back_color = pixels[0].back;
  memcpy(buffer + i, (unsigned char[]) {CSI, APPLY_FORE(fore_color), ';', APPLY_BACK(back_color), 'm'}, 20);
  i += 20;

  for(size_t p_i = 0; p_i < num_pixels; p_i++) {
    unsigned char p_v = pixels[p_i].shape;
    // if (!p_v) {
    //   memcpy(buffer + i, (unsigned char[]) {CSI, APPLY_FORE(0), 'm'}, APPLY_COLOR_SIZE);
    //   i += APPLY_COLOR_SIZE;
    //   buffer[i++] = ' ';
    // } else {
    //   memcpy(buffer + i, (unsigned char[]) {CSI, APPLY_FORE(15), 'm'}, APPLY_COLOR_SIZE);
    //   i += APPLY_COLOR_SIZE;


    if(pixels[p_i].fore == pixels[p_i].back) {
      if (pixels[p_i].back != back_color) {
        back_color = pixels[p_i].back;
        memcpy(buffer + i, (unsigned char[]) {CSI, APPLY_BACK(back_color), 'm'}, APPLY_COLOR_SIZE);
        i += APPLY_COLOR_SIZE;
      }
      buffer[i++] = ' ';
    } else {

      if(pixels[p_i].fore != fore_color || pixels[p_i].back != back_color) {
        memcpy(buffer + i, (unsigned char[]) {CSI}, 2);
        i += 2;
        bool adsfg = 0;
        if(pixels[p_i].fore != fore_color) {
          fore_color = pixels[p_i].fore;
          memcpy(buffer + i, (unsigned char[]) {APPLY_FORE(fore_color)}, 8);
          i += 8;
          adsfg = 1;
        }

        if(pixels[p_i].back != back_color) {
          if (adsfg) {
            memcpy(buffer + i, (unsigned char[]) {';'}, 1);
            i += 1;
          }
          back_color = pixels[p_i].back;
          memcpy(buffer + i, (unsigned char[]) {APPLY_BACK(back_color)}, 8);
          i += 8;
        }
        memcpy(buffer + i, (unsigned char[]) {'m'}, 1);
        i += 1;

      }

      //
      // if (pixels[p_i].fore == fore_color) {
      //   if(pixels[p_i].back == back_color) {
      //
      //   } else {
      //     memcpy(buffer + i, (char[]) {APPLY_BACK(pixels[p_i].back)}, APPLY_COLOR_SIZE);
      //     i += APPLY_COLOR_SIZE;
      //     back_color = pixels[p_i].back;
      //   }
      // } else if (pixels[p_i].fore == back_color) {
      //   p_v ^= 0b1111;
      //   if(pixels[p_i].back == fore_color) {
      //
      //   } else {
      //     memcpy(buffer + i, (char[]) {APPLY_FORE(pixels[p_i].back)}, APPLY_COLOR_SIZE);
      //     i += APPLY_COLOR_SIZE;
      //     fore_color = pixels[p_i].back;
      //   }
      // }
      // else {
        // memcpy(buffer + i, (char[]) {APPLY_FORE(pixels[p_i].fore), APPLY_BACK(pixels[p_i].back)}, 2 * APPLY_COLOR_SIZE);
        // i += 2 * APPLY_COLOR_SIZE;
        // back_color = pixels[p_i].back;
        // fore_color = pixels[p_i].fore;
      // }


      // if(p_v == 0) {
      //   // buffer[i++] = ' ';
      //   memcpy(buffer + i, (char[]) {0x20, 0x00, 0x00, 0x00}, PIXEL_CHAR_SIZE);
      //   i += PIXEL_CHAR_SIZE;
      // } else {
      memcpy(buffer + i, (unsigned char[]) {BLOCK_CHAR_PREFIX, pixel_selector_byte[p_v]}, PIXEL_CHAR_SIZE);
      i += PIXEL_CHAR_SIZE;
    }
    // if(!((p_i + 1) % width)) {
    //   buffer[i++] = '\n';
    // }
  }
  buffer[i++] = '\0';

  return i;
}
