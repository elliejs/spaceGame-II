#ifndef PLANET_H
#define PLANET_H

#include "../../math/vector_3d.h"

typedef
struct planet_s {
  SGVec radius;
}
planet_t;

#include "../object.h"

object_t create_planet(SGVec3D_t origin);

#endif /* end of include guard: PLANET_H */
