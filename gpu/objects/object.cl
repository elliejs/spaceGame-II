#include "object.h"
#include "../render/render.h"

// extern inline
// SGVec SGVec_smooth_min(SGVec a, SGVec b, float k);

// extern inline
// SGVec SGVec_smooth_max(SGVec a, SGVec b, float k);

SGVec3D_t surface_normal(object_t * self, SGVec3D_t point, unsigned int cube_idx, long long time) {
  SGVec dist_pnn = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(-NORMAL_EPSILON))
  }, cube_idx, time);
  SGVec dist_nnp = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(NORMAL_EPSILON))
  }, cube_idx, time);
  SGVec dist_npn = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(-NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(-NORMAL_EPSILON))
  }, cube_idx, time);
  SGVec dist_ppp = self->distance(self, (SGVec3D_t) {
    SGVec_Add_SGVec(point.x, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.y, SGVec_Load_Const(NORMAL_EPSILON)),
    SGVec_Add_SGVec(point.z, SGVec_Load_Const(NORMAL_EPSILON))
  }, cube_idx, time);
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

SGVec3D_t localize_point(SGVec3D_t point, SGVec3D_t origin, SGVec3D_t cube_offset) {
  return SGVec3D_Sub_SGVec3D(point, SGVec3D_Add_SGVec3D(origin, cube_offset));
}
