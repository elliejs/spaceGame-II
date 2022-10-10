#include "vector_3d.h"

extern inline
SGVec3D_t SGVec3D_Sub_SGVec3D(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec SGVec3D_dot(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t SGVec3D_cross(SGVec3D_t a, SGVec3D_t b);

extern inline
SGVec3D_t rot_vec3d(SGVec rots_sin, SGVec rots_cos, SGVec3D_t axis, SGVec3D_t p);

extern inline
SGVec SGVec3D_distance(SGVec3D_t a, SGVec3D_t b);

extern inline
float float3D_distance(float3D_t a, float3D_t b);

extern inline
SGVec3D_t SGVec3D_normalize(SGVec3D_t vec);

extern inline
float3D_t float_normalize(float3D_t vec);
