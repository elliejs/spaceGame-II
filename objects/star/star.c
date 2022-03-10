#include "star.h"

static
SGVec distance(object_t * self, SGVec3D_t point) {
  return SGVec_Sub_SGVec(vector_distance(self->origin, point), self->star.radius);
}

SGVecOKLAB_t get_lighting(star_t * self, SGVec dists) {
  return (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(0.8),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

object_t create_star(SGVec3D_t origin) {
  return (object_t) {
    .origin = origin,
    .distance = distance,
    .star = (star_t) {
      .radius = SGVec_Load_Const(5.),
      .get_lighting = get_lighting
    }
  };
}
