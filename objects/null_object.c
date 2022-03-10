#include <float.h>

#include "object.h"

SGVec distance(object_t * self, SGVec3D_t point) {
  return SGVec_Load_Const(FLT_MAX);
}

object_t create_null_object() {
  return (object_t) {
    .distance = distance,
    .origin = (SGVec3D_t) {
      .x = SGVec_Load_Const(FLT_MAX),
      .y = SGVec_Load_Const(FLT_MAX),
      .z = SGVec_Load_Const(FLT_MAX)
    }
  };
}
