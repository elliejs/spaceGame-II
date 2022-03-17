#ifndef SHIP_H
#define SHIP_H

#include "../../math/vector_3d.h"

typedef
struct orientation_s {
  SGVec3D_t forward;
  SGVec3D_t right;
  SGVec3D_t up;
}
orientation_t;

typedef
struct ship_s {
  orientation_t orientation;
  SGVec SGVec_radius;
  float float_radius;
}
ship_t;

#include "../object.h"

object_t create_ship(float3D_t origin);

#endif /* end of include guard: SHIP_H */
