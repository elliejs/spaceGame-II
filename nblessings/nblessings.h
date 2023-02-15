#ifndef NBLESSINGS_H
#define NBLESSINGS_H

#include "../color/oklab.h"

typedef
struct pixel_s {
  unsigned int fore;
  unsigned int back;
  unsigned char shape;
}
pixel_t;

#define CLEAR_SCREEN_SIZE 4
#define RESET_HOME_SIZE 6
#define APPLY_COLOR_SIZE 11
#define PIXEL_CHAR_SIZE 3

#define MAX_FRAMEBUFFER_SIZE(W, H) (CLEAR_SCREEN_SIZE + RESET_HOME_SIZE + (unsigned int) (W) * (unsigned int) (H) * (2 * APPLY_COLOR_SIZE + PIXEL_CHAR_SIZE) + (H) + 1)

#define ZERO_CHAR '0'
#define A_CHAR 'A'

#define ESC (unsigned char) 0x1B
#define CSI ESC, '['
#define OSC ESC, ']'
#define DCS ESC, 'P'
#define ST ESC, '\\'

#define START_ALTERNATE_BUFFER CSI, '?', '1', '0', '4', '9', 'h'
#define CLEAR_SCREEN CSI, '2', 'J'
#define RESET_HOME CSI, '1', ';', '1', 'H'
// #define SET_SCROLLING_REGION(TOP, BOTTOM) CSI, TOP, ';', BOTTOM, 'r'
// #define RESET_CHARACTER_ATTRIBUTES CSI, '0', 'm'

#define START_ALTERNATE_BUFFER_SIZE 8
#define LOAD_COLOR_SIZE 22

#define NBLESSINGS_HEADER_SIZE (START_ALTERNATE_BUFFER_SIZE + CLEAR_SCREEN_SIZE + LOAD_COLOR_SIZE * 256 + 1)

#define INT2CHARS(I) ZERO_CHAR + (unsigned char) ((I) / 100), ZERO_CHAR + (unsigned char) (((I) % 100) / 10), ZERO_CHAR + (unsigned char) ((I) % 10)
#define HEX2CHAR(HEX) (HEX) > 9 ? (unsigned char) A_CHAR + (HEX) - 10 : (unsigned char) ZERO_CHAR + (HEX)
#define HEX2CHARS(HEX) HEX2CHAR((HEX >> 4) & 0x0F), HEX2CHAR(HEX & 0x0F)

#define LOAD_COLOR(I, R, G, B) OSC, '4', ';', INT2CHARS(I), ';', 'r', 'g', 'b', ':', HEX2CHARS((int) roundf(R)), '/', HEX2CHARS((int) roundf(G)), '/', HEX2CHARS((int) roundf(B)), ST
#define APPLY_FORE(I) '3', '8', ';', '5', ';', INT2CHARS(I)
#define APPLY_BACK(I) '4', '8', ';', '5', ';', INT2CHARS(I)

#define MOVE_TO_SIZE 10
#define MOVE_DIR_SIZE 6
#define MOVE_TO(X,Y) CSI, INT2CHARS(Y), ';', INT2CHARS(X), 'H'
#define MOVE_Y(Y) CSI, INT2CHARS(Y), (Y) < 0 ? 'A' : 'B'
#define MOVE_X(X) CSI, INT2CHARS(X), (X) > 0 ? 'C' : 'D'

unsigned char * nblessings_header_data(oklab_t * colors, unsigned int num_colors, unsigned int * header_len);
unsigned char * nblessings_footer_data(unsigned int * footer_len);
unsigned int rasterize_frame(pixel_t * pixels, unsigned int num_pixels, unsigned int width, unsigned char * buffer);

#endif /* end of include guard: NBLESSINGS_H */
