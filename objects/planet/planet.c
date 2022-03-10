#include "planet.h"

static
SGVec distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(vector_distance(self->origin, point), self->planet.radius);
}

object_t create_planet(SGVec3D_t origin) {
  return (object_t) {
    .origin = origin,
    .distance = distance,
    .planet = (planet_t) {
      .radius = SGVec_Load_Const(5.)
    }
  };
}
