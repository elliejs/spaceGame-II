#include "palette.h"
#include "../palette_helper.h"

void install_palette(unsigned int id, palette_t palette, unsigned int max_colors, oklab_t * color_storage, unsigned int * num_colors) {
  if (palette.num > max_colors - *num_colors) {
    printf("[palette_loader %u]: Can't load this palette, it has too many colors. (%d %d %d)\n", id, palette.num, max_colors, *num_colors);
    return;
  }
  for (int i = 0; i < palette.num; i++) {
    oklab_t color_as_oklab = linear_srgb_to_oklab(normalize_rgb(palette.colors[i]));
    install_color(color_as_oklab, max_colors, color_storage, num_colors);
  }
}
