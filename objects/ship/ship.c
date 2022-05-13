#include "ship.h"

static
SGVec3D_t normal(object_t * self, SGVec3D_t point) {
  return SGVec3D_normalize((SGVec3D_t) {
    .x = SGVec_Sub_SGVec(point.x, self->origin.x),
    .y = SGVec_Sub_SGVec(point.y, self->origin.y),
    .z = SGVec_Sub_SGVec(point.z, self->origin.z)
  });
}

static
SGVec distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(SGVec3D_distance(self->origin, point), self->radius);
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point) {
  return (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(0.),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

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

object_t create_ship(SGVec3D_t origin) {
  return (object_t) {
    .origin = origin,
    .radius = SGVec_Load_Const(2.),
    .distance = distance,
    .normal = normal,
    .color = color,
    .ship = (ship_t) {
      .orientation = DEFAULT_ORIENTATION
    }
  };
}
