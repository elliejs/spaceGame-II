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
  float3D_t (*float_normal)(struct object_s *, float3D_t);
  oklab_t (*get_color)(struct object_s *, float3D_t);
  SGVec3D_t SGVec_origin;
  float3D_t float_origin;

  union {
    planet_t planet;
    ship_t ship;
    star_t star;
  };
};

inline
SGVec SGVec_smooth_min(SGVec a, SGVec b, float k) {
  SGVec h = SGVec_Mult_Float(SGVec_Maximum(SGVec_Sub_SGVec(SGVec_Load_Const(k), SGVec_Absolute(SGVec_Sub_SGVec(a,b))), SGVec_ZERO), 1./k);
  return SGVec_Sub_SGVec(SGVec_Minimum(a, b), SGVec_Mult_Float(SGVec_Mult_Float(SGVec_Mult_SGVec(SGVec_Mult_SGVec(h,h),h),k),1.0/6.0));
}

inline
SGVec SGVec_smooth_max(SGVec a, SGVec b, float k) {
  SGVec h = SGVec_Mult_Float(SGVec_Maximum(SGVec_Sub_SGVec(SGVec_Load_Const(k), SGVec_Absolute(SGVec_Sub_SGVec(a,b))), SGVec_ZERO), 1./k);
  return SGVec_Add_SGVec(SGVec_Maximum(a, b), SGVec_Mult_Float(SGVec_Mult_Float(SGVec_Mult_SGVec(SGVec_Mult_SGVec(h,h),h),k),1.0/6.0));
}

inline
float float_smooth_min(float a, float b, float k) {
  float h = fmax(k - fabs(a - b), 0.) / k;
  return fmin(a, b) - h * h * h * k * (1.0/6.0);
}

inline
float float_smooth_max(float a, float b, float k) {
  float h = fmax(k - fabs(a - b), 0.) / k;
  return fmax(a, b) + h * h * h * k * (1.0/6.0);
}

#endif /* end of include guard: OBJECT_H */
