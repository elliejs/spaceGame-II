#include "object.h"
#include "../render/render.h"

extern inline
SGVec SGVec_smooth_min(SGVec a, SGVec b, float k);

extern inline
SGVec SGVec_smooth_max(SGVec a, SGVec b, float k);

SGVec3D_t surface_normal(object_t * self, SGVec3D_t point, int chunk_idx) {
  SGVec dist_pnn = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(-NORMAL_EPSILON))
  }, chunk_idx);
  SGVec dist_nnp = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(NORMAL_EPSILON))
  }, chunk_idx);
  SGVec dist_npn = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(-NORMAL_EPSILON))
  }, chunk_idx);
  SGVec dist_ppp = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(NORMAL_EPSILON))
  }, chunk_idx);
  return SGVec3D_normalize(
    (SGVec3D_t) {
      SGVec_Add_SGVec(
        SGVec_Sub_SGVec(
          SGVec_Sub_SGVec(
            dist_pnn,
            dist_nnp
          ),
          dist_npn
        ),
        dist_ppp
      ),
      SGVec_Add_SGVec(
        SGVec_Sub_SGVec(
          SGVec_Sub_SGVec(
            dist_npn,
            dist_nnp
          ),
          dist_pnn
        ),
        dist_ppp
      ),
      SGVec_Add_SGVec(
        SGVec_Sub_SGVec(
          SGVec_Sub_SGVec(
            dist_nnp,
            dist_npn
          ),
          dist_pnn
        ),
        dist_ppp
      )
    }
  );
}

/*
vec3 calcNormal( in vec3 & p ) // for function f(p)
{
    const float h = 0.0001; // replace by an appropriate value
    const vec2 k = vec2(1,-1);
    return normalize( k.xyy*f( p + k.xyy*h ) +
                      k.yyx*f( p + k.yyx*h ) +
                      k.yxy*f( p + k.yxy*h ) +
                      k.xxx*f( p + k.xxx*h ) );
}
*/
