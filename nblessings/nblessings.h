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
#define RESET_HOME_SIZE 4
#define APPLY_COLOR_SIZE 11
#define PIXEL_CHAR_SIZE 3

#define MAX_FRAMEBUFFER_SIZE(W, H) (CLEAR_SCREEN_SIZE + RESET_HOME_SIZE + (unsigned int) (W) * (unsigned int) (H) * (2 * APPLY_COLOR_SIZE + PIXEL_CHAR_SIZE) + 1)

unsigned char * nblessings_header_data(oklab_t * colors, unsigned int num_colors, unsigned int * header_len);
unsigned char * nblessings_footer_data(unsigned int * footer_len);
unsigned int rasterize_frame(pixel_t * pixels, unsigned int num_pixels, unsigned char * buffer);

#endif /* end of include guard: NBLESSINGS_H */
