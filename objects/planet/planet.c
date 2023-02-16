#include <stdio.h>

#include "planet.h"
#include "../../math/simplex_noise.h"
#include "../../world/world_server.h"

static
SGVec distance(object_t * self, SGVec3D_t point, unsigned int cube_idx) {
  SGVec3D_t cube_offset = get_cube_offset(cube_idx);
  SGVec3D_t rel_offset = SGVec3D_Add_SGVec3D(self->origin, cube_offset);

  SGVec dist_to_center = SGVec3D_distance(point, rel_offset);
  SGVec3D_t planetary_offset = SGVec3D_Add_SGVec3D(SGVec3D_Sub_SGVec3D(point, rel_offset), self->planet.pos_hash);

  SGVecUInt lod = SGVec_Gtr_Than(dist_to_center, SGVec_Add_SGVec(self->radius, self->planet.noise_amplitude));
  uint32_t lod_array[4]; SGVecUInt_Store_Array(lod_array, lod);

  float absolute_x[4]; SGVec_Store_Array(absolute_x, planetary_offset.x);
  float absolute_y[4]; SGVec_Store_Array(absolute_y, planetary_offset.y);
  float absolute_z[4]; SGVec_Store_Array(absolute_z, planetary_offset.z);

  float noises[4] = {0.};
  for (int i = 0; i < 4; i++) {
    if (lod_array[i]) continue;
    noises[i] = fnlGetNoise3D(&(self->planet.state), absolute_x[i], absolute_y[i], absolute_z[i]);
  }

  SGVec noise = SGVec_Load_Array(noises);
  // return SGVec_Mult_SGVec(noise, self->planet.noise_amplitude);

  return SGVec_Sub_SGVec(dist_to_center, SGVec_Add_SGVec(self->radius, SGVec_Mult_SGVec(noise, self->planet.noise_amplitude)));
  // return SGVec_Sub_SGVec(dist_to_center, self->radius);
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point) {
  return self->planet.basis;
}

object_t create_planet(SGVec3D_t origin, SGVec radius, SGVec noise_amplitude, SGVecOKLAB_t basis, SGVec3D_t pos_hash) {
  fnl_state state = fnlCreateState();
  state.fractal_type = FNL_FRACTAL_FBM;
  // state.gain = 1.;
  return (object_t) {
    .origin = origin,
    .radius = radius,
    .distance = distance,
    .color = color,
    .planet = (planet_t) {
      .state = state,
      .noise_amplitude = noise_amplitude,
      .basis = basis,
      .pos_hash = pos_hash
    }
  };
}
