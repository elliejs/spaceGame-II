#ifndef SHIP_H
#define SHIP_H

#include "../../math/vector_3d.h"
#include "../../world/chunk.h"


typedef
struct orientation_s {
  SGVec3D_t forward;
  SGVec3D_t right;
  SGVec3D_t up;
}
orientation_t;

typedef
struct orient_rot_quat_s {
  SGVec4D_t forward;
  SGVec4D_t right;
  SGVec4D_t up;
}
orient_rot_quat_t;

typedef
struct ship_s {
  orientation_t orientation;
  orient_rot_quat_t rot_quats;
  SGVec height;
  SGVec la;
  SGVec lb;
  unsigned int vision;
  chunk_coord_t abs_coord;
}
ship_t;

#include "../object.h"

object_t create_ship(SGVec3D_t origin, chunk_coord_t abs_coord);

#define DEFAULT_ORIENTATION \
(orientation_t) { \
  .up = (SGVec3D_t) { \
    .x = SGVec_ZERO, \
    .y = SGVec_ONE, \
    .z = SGVec_ZERO \
  }, \
  .forward = (SGVec3D_t) { \
    .x = SGVec_ZERO, \
    .y = SGVec_ZERO, \
    .z = SGVec_ONE, \
  }, \
  .right = (SGVec3D_t) { \
    .x = SGVec_ONE, \
    .y = SGVec_ZERO, \
    .z = SGVec_ZERO \
  } \
}

#define DEFAULT_ROT_QUAT \
  (orient_rot_quat_t) { \
    .forward = SGVec4D_IDENTITY, \
    .right = SGVec4D_IDENTITY, \
    .up = SGVec4D_IDENTITY \
  }
#endif /* end of include guard: SHIP_H */
