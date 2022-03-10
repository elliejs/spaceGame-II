#ifndef OBJECT_H
#define OBJECT_H

typedef struct object_s object_t;

#include "planet/planet.h"
#include "ship/ship.h"
#include "star/star.h"

#include "../math/vector_3d.h"

struct object_s {
  SGVec (*distance)(struct object_s *, SGVec3D_t);
  SGVec3D_t origin;

  union {
    planet_t planet;
    ship_t ship;
    star_t star;
  };
};

object_t create_null_object();

#endif /* end of include guard: OBJECT_H */
