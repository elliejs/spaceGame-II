#include <stdio.h>

#include "planet.h"

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
  SGVec dist = SGVec_Sub_SGVec(
    SGVec3D_distance(
      (SGVec3D_t) {
        SGVec_Add_SGVec(self->origin.x, chunk_offsets[chunk_idx].x),
        SGVec_Add_SGVec(self->origin.y, chunk_offsets[chunk_idx].y),
        SGVec_Add_SGVec(self->origin.z, chunk_offsets[chunk_idx].z)
      },
      point
    ),
    self->radius);

  return dist;
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point, int chunk_idx) {
  const oklab_t basis = linear_srgb_to_oklab((rgb_t) {0., 0.2, 0.});
  return (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(basis.l),
    .a = SGVec_Load_Const(basis.a),
    .b = SGVec_Load_Const(basis.b)
  };
}

object_t create_planet(SGVec3D_t origin, SGVec radius) {
  return (object_t) {
    .origin = origin,
    .radius = radius,
    .distance = distance,
    .normal = normal,
    .color = color,
    .planet = (planet_t) {
    }
  };
}
