#ifndef PLANET_H
#define PLANET_H

#include "../../math/vector_3d.h"

#include "../../math/FastNoiseLite.h"

#include "../../color/oklab.h"

typedef
struct planet_s {
  fnl_state state;
  SGVec noise_amplitude;
  SGVecOKLAB_t basis;
  SGVec3D_t pos_hash;
  SGVec3D_t spin_axis;
  unsigned int revolve_period;
}
planet_t;

#include "../object.h"

object_t create_planet(SGVec3D_t origin, SGVec radius, SGVec noise_amplitude, SGVecOKLAB_t basis, SGVec3D_t pos_hash);

#endif /* end of include guard: PLANET_H */
