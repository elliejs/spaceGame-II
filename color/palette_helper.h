#ifndef PALETTE_HELPER_H
#define PALETTE_HELPER_H

#include <float.h>

#include "oklab.h"

inline
unsigned int closest_color_index(oklab_t color, oklab_t * colors, unsigned int num_colors) {
  float delta = FLT_MAX;
  unsigned int color_index = 0;
  for (int i = 0; i < num_colors; i++) {
    float color_delta = delta_E(colors[i], color);
    if (color_delta < delta) {
      delta = color_delta;
      color_index = i;
    }
  }
  return color_index;
}

inline
SGVecOKLAB_t normalize_SGVecOKLAB(SGVecOKLAB_t color) {
  SGVec ab_max = SGVec_Maximum(SGVec_Absolute(color.a), SGVec_Absolute(color.b));
  SGVecUInt out_of_range = SGVec_Gtr_Than(ab_max, SGVec_Load_Const(100.));
  SGVec ab_multiplier = SGVec_Ternary(out_of_range, SGVec_Reciprocal(SGVec_Mult_Float(ab_max, 0.01)), SGVec_Load_Const(1.));

  return (SGVecOKLAB_t) {
    .l = SGVec_Clamp(color.l, 0., 1.),
    .a = SGVec_Mult_SGVec(color.a, ab_multiplier),
    .b = SGVec_Mult_SGVec(color.b, ab_multiplier)
  };
}

inline
bool install_color(oklab_t color, unsigned int max_colors, oklab_t * color_storage, unsigned int * num_colors) {
  if(*num_colors >= max_colors) return false;
  color_storage[(*num_colors)++] = color;
  return true;
}

inline
void create_gradient(oklab_t start, oklab_t end, unsigned int steps, unsigned int max_colors, oklab_t * color_storage, unsigned int * num_colors) {
  steps++;
  oklab_t delta = (oklab_t) {
    .l = (end.l - start.l) / (float) steps,
    .a = (end.a - start.a) / (float) steps,
    .b = (end.b - start.b) / (float) steps
  };
  for (int i = 1; i < steps; i++) {
    if (!install_color((oklab_t) {
      .l = start.l + delta.l * i,
      .a = start.a + delta.a * i,
      .b = start.b + delta.b * i,
    }, max_colors, color_storage, num_colors)) break;
  }
}

#endif /* end of include guard: PALETTE_HELPER_H */
