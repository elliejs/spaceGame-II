#include <stdio.h>

#include "planet.h"

#include "../../world_db/world_db.h"

// static
// float3D_t float_normal(object_t * self, float3D_t point) {
//   return float_normalize((float3D_t) {
//     .x = point.x - self->float_origin.x,
//     .y = point.y - self->float_origin.y,
//     .z = point.z - self->float_origin.z
//   });
// }

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
    // .float_origin = origin,
    .origin = origin,
    .radius = radius,
    .distance = distance,
    .normal = normal,
    // .float_normal = float_normal,
    .color = color,
    .planet = (planet_t) {
      // .float_radius = radius
    }
  };
}
