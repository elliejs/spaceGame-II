#ifndef PLANET_H
#define PLANET_H

#include "../../math/vector_3d.h"

typedef
struct planet_s {
  SGVec SGVec_radius;
  float float_radius;
}
planet_t;

#include "../object.h"

object_t create_planet(float3D_t origin, float radius);

#endif /* end of include guard: PLANET_H */
