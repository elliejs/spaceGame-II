#include <stdio.h>

#include "planet.h"

static
SGVec distance(object_t * self, SGVec3D_t point, int chunk_idx) {
  SGVec3D_t chunk_offset = get_chunk_offset(chunk_idx);
  SGVec dist = SGVec_Sub_SGVec(
    SGVec3D_distance(
      (SGVec3D_t) {
        SGVec_Add_SGVec(self->origin.x, chunk_offset.x),
        SGVec_Add_SGVec(self->origin.y, chunk_offset.y),
        SGVec_Add_SGVec(self->origin.z, chunk_offset.z)
      },
      point
    ),
    self->radius);

  return dist;
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point, int chunk_idx) {
  float z = 0, y = 0, x = 0;
  while (chunk_idx >= 9) {
    chunk_idx -= 9;
    z += 0.5;
  }
  while (chunk_idx >= 3) {
    chunk_idx -= 3;
    y += 0.5;
  }
  while (chunk_idx >= 1) {
    chunk_idx -= 1;
    x += 0.5;
  }

  const oklab_t basis = linear_srgb_to_oklab((rgb_t) {x, y, z});
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
    .color = color,
    .planet = (planet_t) {
    }
  };
}
