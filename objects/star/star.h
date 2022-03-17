#ifndef STAR_H
#define STAR_H

#include "../../utils/types/simd.h"

#include "../../color/oklab.h"

typedef
struct star_s {
  SGVec SGVec_radius;
  float float_radius;
  SGVecOKLAB_t (*get_lighting)(struct star_s *, SGVec distance);
}
star_t;

#include "../object.h"

object_t create_star(float3D_t origin);

#endif /* end of include guard: STAR_H */
