#ifndef PALETTE_H
#define PALETTE_H

#include "../oklab.h"

#define MAX_COLORS 256

typedef
struct palette_s {
  unsigned int num;
  rgb_t colors[MAX_COLORS];
}
palette_t;

void install_palette(unsigned int id, palette_t palette, unsigned int max_colors, oklab_t * color_storage, unsigned int * num_colors);

#include "duel.h"

#endif /* end of include guard: PALETTE_H */
