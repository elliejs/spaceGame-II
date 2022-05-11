#ifndef PLANET_H
#define PLANET_H

#include "../../math/vector_3d.h"

typedef
struct planet_s {
}
planet_t;

#include "../object.h"

object_t create_planet(SGVec3D_t origin, SGVec radius);

#endif /* end of include guard: PLANET_H */
