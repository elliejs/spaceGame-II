#ifndef STAR_H
#define STAR_H

#include "../../utils/types/simd.h"

#include "../../color/oklab.h"

typedef
struct star_s {
  // float float_radius;
  SGVecOKLAB_t (*radiance)(struct star_s *, SGVec distance);
}
star_t;

#include "../object.h"

object_t create_star(SGVec3D_t origin, SGVec radius);

#endif /* end of include guard: STAR_H */
