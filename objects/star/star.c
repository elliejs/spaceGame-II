#include "star.h"

static
SGVec distance(object_t * self, SGVec3D_t point, int chunk_idx) {
  SGVec3D_t chunk_offset = get_chunk_offset(chunk_idx);
  return SGVec_Sub_SGVec(
    SGVec3D_distance(
      (SGVec3D_t) {
        SGVec_Add_SGVec(self->origin.x, chunk_offset.x),
        SGVec_Add_SGVec(self->origin.y, chunk_offset.y),
        SGVec_Add_SGVec(self->origin.z, chunk_offset.z)
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
    // .l = SGVec_Load_Const(0.4),
    .l = SGVec_Mult_SGVec(SGVec_Load_Const(5000.), SGVec_Reciprocal(SGVec_Mult_SGVec(dists, dists))),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

object_t create_star(SGVec3D_t origin, SGVec radius) {
  return (object_t) {
    .origin = origin,
    .radius = radius,
    .distance = distance,
    .color = color,
    .star = (star_t) {
      .radiance = radiance
    }
  };
}
