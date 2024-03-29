#include "star.h"
#include "../../world/world_server.h"

static
SGVec distance(object_t * self, SGVec3D_t point, unsigned int cube_idx, long long time) {
  // return SGVec_Load_Const(1000000000000.);
  SGVec3D_t cube_offset = get_cube_offset(cube_idx);
  return SGVec_Sub_SGVec(
    SGVec3D_distance(
      (SGVec3D_t) {
        SGVec_Add_SGVec(self->origin.x, cube_offset.x),
        SGVec_Add_SGVec(self->origin.y, cube_offset.y),
        SGVec_Add_SGVec(self->origin.z, cube_offset.z)
      },
      point
    ),
    self->radius);
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point, long long time) {
  float radiance_percent = (time % self->star.radiance_period) / (float) self->star.radiance_period;
  if (radiance_percent > 0.5) radiance_percent = 1.0 - radiance_percent;
  return (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(radiance_percent + 0.5),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };
}

SGVecOKLAB_t radiance(star_t * self, SGVec dists, long long time) {
  float radiance_percent = (time % self->radiance_period) / (float) self->radiance_period;
  if (radiance_percent > 0.5) radiance_percent = 1.0 - radiance_percent;
  return (SGVecOKLAB_t) {
    .l = SGVec_Mult_SGVec(SGVec_Load_Const(10. * radiance_percent + 50.), SGVec_Reciprocal(dists)),
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
      .radiance = radiance,
      .radiance_period = 1000,
    }
  };
}
