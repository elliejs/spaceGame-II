#include "star.h"

static
float3D_t float_normal(object_t * self, float3D_t point) {
  return float_normalize((float3D_t) {
    .x = point.x - self->float_origin.x,
    .y = point.y - self->float_origin.y,
    .z = point.z - self->float_origin.z
  });
}

static
SGVec SGVec_distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(SGVec3D_distance(self->SGVec_origin, point), self->star.SGVec_radius);
}

static
oklab_t get_color(object_t * self, float3D_t point) {
  return (oklab_t) {
    .l = 2.0,
    .a = 0.,
    .b = 0.
  };
}

SGVecOKLAB_t get_lighting(star_t * self, SGVec dists) {
  return (SGVecOKLAB_t) {
    .l = SGVec_Mult_SGVec(SGVec_Load_Const(10000.), SGVec_Reciprocal(SGVec_Mult_SGVec(dists, dists))),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

object_t create_star(float3D_t origin, float radius) {
  return (object_t) {
    .float_origin = origin,
    .SGVec_origin = (SGVec3D_t) {
      .x = SGVec_Load_Const(origin.x),
      .y = SGVec_Load_Const(origin.y),
      .z = SGVec_Load_Const(origin.z)
    },
    .SGVec_distance = SGVec_distance,
    .float_normal = float_normal,
    .get_color = get_color,
    .star = (star_t) {
      .SGVec_radius = SGVec_Load_Const(radius),
      .float_radius = radius,
      .get_lighting = get_lighting
    }
  };
}
