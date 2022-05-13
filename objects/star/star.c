#include "star.h"

static
SGVec3D_t normal(object_t * self, SGVec3D_t point, int chunk_idx) {
  return SGVec3D_normalize((SGVec3D_t) {
    .x = SGVec_Sub_SGVec(point.x, SGVec_Add_SGVec(self->origin.x, chunk_offsets[chunk_idx].x)),
    .y = SGVec_Sub_SGVec(point.y, SGVec_Add_SGVec(self->origin.y, chunk_offsets[chunk_idx].y)),
    .z = SGVec_Sub_SGVec(point.z, SGVec_Add_SGVec(self->origin.z, chunk_offsets[chunk_idx].z))
  });
}

static
SGVec distance(object_t * self, SGVec3D_t point, int chunk_idx) {
  return SGVec_Sub_SGVec(
    SGVec3D_distance(
      (SGVec3D_t) {
        SGVec_Add_SGVec(self->origin.x, chunk_offsets[chunk_idx].x),
        SGVec_Add_SGVec(self->origin.y, chunk_offsets[chunk_idx].y),
        SGVec_Add_SGVec(self->origin.z, chunk_offsets[chunk_idx].z)
      },
      point
    ),
    self->radius);
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point, int chunk_idx) {
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
