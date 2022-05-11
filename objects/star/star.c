#include "star.h"

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
    .l = SGVec_Load_Const(1.),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

SGVecOKLAB_t radiance(star_t * self, SGVec dists) {
  return (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(1.), //SGVec_Mult_SGVec(SGVec_Load_Const(1000.), SGVec_Reciprocal(SGVec_Mult_SGVec(dists, dists))),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

object_t create_star(SGVec3D_t origin, SGVec radius) {
  return (object_t) {
    .origin = origin,
    .radius = radius,
    .distance = distance,
    .normal = normal,
    .color = color,
    .star = (star_t) {
      .radiance = radiance
    }
  };
}
