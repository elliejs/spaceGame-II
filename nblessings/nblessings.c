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
  memcpy(buffer + i, (unsigned char[]) {CLEAR_SCREEN, RESET_HOME}, CLEAR_SCREEN_SIZE + RESET_HOME_SIZE);
  i += CLEAR_SCREEN_SIZE + RESET_HOME_SIZE;
  unsigned int fore_color = pixels[0].fore;
  unsigned int back_color = pixels[0].back;
  memcpy(buffer + i, (unsigned char[]) {CSI, APPLY_FORE(fore_color), ';', APPLY_BACK(back_color), 'm'}, 20);
  i += 20;

  for(size_t p_i = 0; p_i < num_pixels; p_i++) {
    unsigned char p_v = pixels[p_i].shape;
    if(pixels[p_i].fore == pixels[p_i].back || p_v == 0) {
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
        bool compound_switch = 0;
        if(pixels[p_i].fore != fore_color) {
          fore_color = pixels[p_i].fore;
          memcpy(buffer + i, (unsigned char[]) {APPLY_FORE(fore_color)}, 8);
          i += 8;
          compound_switch = 1;
        }

        if(pixels[p_i].back != back_color) {
          if (compound_switch) {
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
      memcpy(buffer + i, (unsigned char[]) {BLOCK_CHAR_PREFIX, pixel_selector_byte[p_v]}, PIXEL_CHAR_SIZE);
      i += PIXEL_CHAR_SIZE;
    }
  }
  // memcpy(buffer + i, (unsigned char[]) {CSI}, 2);
  // i += 2;
  // memcpy(buffer + i, (unsigned char[]) {APPLY_FORE(30)}, 8);
  // i += 8;
  // memcpy(buffer + i, (unsigned char[]) {';'}, 1);
  // i += 1;
  // memcpy(buffer + i, (unsigned char[]) {APPLY_BACK(0)}, 8);
  // i += 8;
  // memcpy(buffer + i, (unsigned char[]) {'m'}, 1);
  // i += 1;
  // for (int c_i = 0; c_i < sizeof(unsigned int) * 2; c_i++) {
  //   unsigned int hex = (encoded_chunk_id & 0b1111);
  //   buffer[i++] = hex > 9 ? 'A' + hex - 10 : '0' + hex;
  //   encoded_chunk_id >>= 4;
  // }
  buffer[i++] = '\0';

  return i;
}
