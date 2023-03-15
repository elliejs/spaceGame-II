#include "vector_3d.h"

extern inline
SGVec3D_t SGVec3D_Add_SGVec3D(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t SGVec3D_Sub_SGVec3D(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t SGVec3D_Mult_SGVec(SGVec3D_t a, SGVec b);

extern inline
SGVec SGVec3D_dot(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t SGVec3D_cross(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t rot_vec3d(SGVec4D_t rot_quat, SGVec3D_t p);

extern inline
SGVec SGVec3D_distance(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t SGVec3D_normalize(SGVec3D_t vec);

extern inline
SGVec4D_t SGVec4D_normalize(SGVec4D_t vec);

extern inline
SGVec4D_t SGVec4D_Mult_SGVec4D(SGVec4D_t a, SGVec4D_t b);

extern inline
SGVec4D_t prepare_rot_quat(SGVec rots_sin, SGVec rots_cos, SGVec3D_t axis);

extern inline
SGVec4D_t SGVec4D_Invert(SGVec4D_t a);
