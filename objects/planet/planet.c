#include <stdio.h>

#include "planet.h"

#include "../../world_db/world_db.h"

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
  SGVec sph = SGVec_Sub_SGVec(SGVec3D_distance(self->origin, point), self->radius);
  return sph;
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point) {
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
