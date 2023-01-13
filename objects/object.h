#include "../world/world_server.h"

#ifndef OBJECT_H
#define OBJECT_H

//OBJECTS MAY NOT USE *ALLOC. THEY ARE STORED IN A SHMEM BLOCK.
// USING *ALLOC WILL CAUSE ALL OTHER PROCESSES THAN THE ALLOCATOR
// TO JUMP TO INVALID MEMORY.

typedef struct object_s object_t;

#include "planet/planet.h"
#include "ship/ship.h"
#include "star/star.h"

#include "../math/vector_3d.h"
#include "../color/oklab.h"

struct object_s {
  SGVec (*distance)(struct object_s *, SGVec3D_t, unsigned int cube_idx);
  SGVecOKLAB_t (*color)(struct object_s *, SGVec3D_t);
  SGVec3D_t origin;
  SGVec radius;

  union {
    planet_t planet;
    ship_t ship;
    star_t star;
  };
};

// inline
// SGVec SGVec_smooth_min(SGVec a, SGVec b, float k) {
//   SGVec h = SGVec_Mult_Float(SGVec_Maximum(SGVec_Sub_SGVec(SGVec_Load_Const(k), SGVec_Absolute(SGVec_Sub_SGVec(a,b))), SGVec_ZERO), 1./k);
//   return SGVec_Sub_SGVec(SGVec_Minimum(a, b), SGVec_Mult_Float(SGVec_Mult_Float(SGVec_Mult_SGVec(SGVec_Mult_SGVec(h,h),h),k),1.0/6.0));
// }
//
// inline
// SGVec SGVec_smooth_max(SGVec a, SGVec b, float k) {
//   SGVec h = SGVec_Mult_Float(SGVec_Maximum(SGVec_Sub_SGVec(SGVec_Load_Const(k), SGVec_Absolute(SGVec_Sub_SGVec(a,b))), SGVec_ZERO), 1./k);
//   return SGVec_Add_SGVec(SGVec_Maximum(a, b), SGVec_Mult_Float(SGVec_Mult_Float(SGVec_Mult_SGVec(SGVec_Mult_SGVec(h,h),h),k),1.0/6.0));
// }

SGVec3D_t surface_normal(object_t * self, SGVec3D_t point, unsigned int cube_idx);

#endif /* end of include guard: OBJECT_H */
