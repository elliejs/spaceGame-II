#ifndef OBJECT_H
#define OBJECT_H

typedef struct object_s object_t;

#include "planet/planet.h"
#include "ship/ship.h"
#include "star/star.h"

#include "../math/vector_3d.h"
#include "../color/oklab.h"

struct object_s {
  SGVec (*SGVec_distance)(struct object_s *, SGVec3D_t);
  float (*float_distance)(struct object_s *, float3D_t);
  oklab_t (*get_color)(struct object_s *, float3D_t);
  SGVec3D_t SGVec_origin;
  float3D_t float_origin;

  union {
    planet_t planet;
    ship_t ship;
    star_t star;
  };
};

#endif /* end of include guard: OBJECT_H */
