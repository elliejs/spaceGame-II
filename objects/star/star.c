#include "star.h"

static
SGVec SGVec_distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(SGVec3D_distance(self->SGVec_origin, point), self->star.SGVec_radius);
}

static
float float_distance(object_t * self, float3D_t point) {
  return float3D_distance(self->float_origin, point) - self->star.float_radius;
}

static
oklab_t get_color(object_t * self, float3D_t point) {
  return (oklab_t) {
    .l = 0.5,
    .a = 0.,
    .b = 0.
  };
}

SGVecOKLAB_t get_lighting(star_t * self, SGVec dists) {
  // return (SGVecOKLAB_t) {
  //   .l = SGVec_Load_Const(0.8),
  //   .a = SGVec_Load_Const(0.),
  //   .b = SGVec_Load_Const(0.)
  // };
  return (SGVecOKLAB_t) {
    .l = SGVec_Mult_SGVec(SGVec_Load_Const(10000.), SGVec_Reciprocal(SGVec_Mult_SGVec(dists, dists))),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

object_t create_star(float3D_t origin) {
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
    .star = (star_t) {
      .SGVec_radius = SGVec_Load_Const(50.),
      .float_radius = 50.,
      .get_lighting = get_lighting
    }
  };
}
