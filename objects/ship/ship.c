#include "ship.h"

static
SGVec distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(vector_distance(self->origin, point), self->ship.radius);
}

object_t create_ship(SGVec3D_t origin) {
  const SGVec vec_one = SGVec_Load_Const(1.);
  const SGVec vec_zero = SGVec_Load_Const(0.);

  return (object_t) {
    .origin = origin,
    .distance = distance,

    .ship = (ship_t) {
      .orientation = (orientation_t) {
        .up = (SGVec3D_t) {
          .x = vec_zero,
          .y = vec_one,
          .z = vec_zero
        },
        .forward = (SGVec3D_t) {
          .x = vec_zero,
          .y = vec_zero,
          .z = vec_one,
        },
        .right = (SGVec3D_t) {
          .x = vec_one,
          .y = vec_zero,
          .z = vec_zero
        }
      },
      .radius = SGVec_Load_Const(2.)
    }
  };
}
