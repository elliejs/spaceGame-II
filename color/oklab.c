#include "oklab.h"

extern inline
rgb_t normalize_rgb(rgb_t rgb);

extern inline
rgb_t clamp_denormalize_rgb(rgb_t rgb);

extern inline
rgb_t scale_denormalize_rgb(rgb_t rgb);

extern inline
float delta_E(oklab_t x, oklab_t y);

extern inline
oklab_t linear_srgb_to_oklab(rgb_t c);

extern inline
rgb_t oklab_to_linear_srgb(oklab_t c);

extern inline
SGVec SGVecOKLAB_dot(SGVecOKLAB_t a, SGVecOKLAB_t b);
