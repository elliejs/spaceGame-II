#include <stdio.h>

#include "planet.h"

#include "../../world_db/world_db.h"

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
  SGVec sph = SGVec_Sub_SGVec(SGVec3D_distance(self->SGVec_origin, point), self->planet.SGVec_radius);
  return sph;
}

static
oklab_t get_color(object_t * self, float3D_t point) {
  return linear_srgb_to_oklab((rgb_t) {0., 1., 0.});
}

object_t create_planet(float3D_t origin, float radius) {
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
    .planet = (planet_t) {
      .SGVec_radius = SGVec_Load_Const(radius),
      .float_radius = radius
    }
  };
}
