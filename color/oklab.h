#ifndef OKLAB_H
#define OKLAB_H

#include <math.h>
#include "../utils/types/simd.h"

// OKLAB formulas opensource under MIT license
// Thanks to Björn Ottosson
// https://bottosson.github.io/posts/oklab/

typedef
struct oklab_s {
    float l;
    float a;
    float b;
}
oklab_t;

typedef
struct rgb_s {
    float r;
    float g;
    float b;
}
rgb_t;

typedef
struct SGVecOKLAB_s {
  SGVec l;
  SGVec a;
  SGVec b;
}
SGVecOKLAB_t;

#define OKLAB_BLACK (oklab_t) {.l = 0., .a = 0., .b = 0.}

inline
float delta_E(oklab_t x, oklab_t y) {
  float delta_l = x.l - y.l;
  float c1 = sqrtf(x.a * x.a + x.b * x.b);
  float c2 = sqrtf(y.a * y.a + y.b * y.b);
  float delta_c = c1 - c2;
  float delta_a = x.a - y.a;
  float delta_b = x.b - y.b;
  float delta_h = sqrtf(delta_a * delta_a + delta_b * delta_b - delta_c * delta_c);
  float delta_E_OK = sqrtf(delta_l * delta_l + delta_c * delta_c + delta_h * delta_h);
  return delta_E_OK;
}

inline
rgb_t normalize_rgb(unsigned int r, unsigned int g, unsigned int b) {
  return (rgb_t) {
    .r = r / (float) 255.,
    .g = g / (float) 255.,
    .b = b / (float) 255.
  };
}

inline
rgb_t denormalize_rgb(rgb_t rgb) {
  return (rgb_t) {
    .r = rgb.r * (float) 255.,
    .g = rgb.g * (float) 255.,
    .b = rgb.b * (float) 255.
  };
}

inline
oklab_t linear_srgb_to_oklab(rgb_t c) {
  float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
	float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
	float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

  float l_ = cbrtf(l);
  float m_ = cbrtf(m);
  float s_ = cbrtf(s);

  return (oklab_t) {
    .l = 0.2104542553f*l_ + 0.7936177850f*m_ - 0.0040720468f*s_,
    .a = 1.9779984951f*l_ - 2.4285922050f*m_ + 0.4505937099f*s_,
    .b = 0.0259040371f*l_ + 0.7827717662f*m_ - 0.8086757660f*s_
  };
}

inline
rgb_t oklab_to_linear_srgb(oklab_t c) {
  float l_ = c.l + 0.3963377774f * c.a + 0.2158037573f * c.b;
  float m_ = c.l - 0.1055613458f * c.a - 0.0638541728f * c.b;
  float s_ = c.l - 0.0894841775f * c.a - 1.2914855480f * c.b;

  float l = l_*l_*l_;
  float m = m_*m_*m_;
  float s = s_*s_*s_;

  return (rgb_t) {
  	.r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
  	.g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
  	.b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s
  };
}

#endif /* end of include guard: OKLAB_H */
