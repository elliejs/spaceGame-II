#include "ship.h"

static
SGVec SGVec_distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(SGVec3D_distance(self->SGVec_origin, point), self->ship.SGVec_radius);
}

static
float float_distance(object_t * self, float3D_t point) {
  return float3D_distance(self->float_origin, point) - self->ship.float_radius;
}

static
oklab_t get_color(object_t * self, float3D_t point) {
  return linear_srgb_to_oklab((rgb_t) {0., 0., 0.});
}

object_t create_ship(float3D_t origin) {
  const SGVec vec_one = SGVec_Load_Const(1.);
  const SGVec vec_zero = SGVec_Load_Const(0.);

  return (object_t) {
    .float_origin = origin,
    .SGVec_origin = (SGVec3D_t) {
      .x = SGVec_Load_Const(origin.x),
      .y = SGVec_Load_Const(origin.y),
      .z = SGVec_Load_Const(origin.z)
    },
    .SGVec_distance = SGVec_distance,
    .float_distance = float_distance,
    .get_color = get_color,
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
      .SGVec_radius = SGVec_Load_Const(2.),
      .float_radius = 2.
    }
  };
}
