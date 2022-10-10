#ifndef VECTOR_3D_H
#define VECTOR_3D_H

#include <math.h>
#include <stdio.h>

#include "../utils/types/simd.h"

#define DEG2RAD(X) (((float) M_PI / (float) 180.) * (float) (X))

typedef
struct SGVec3D_s {
  SGVec x;
  SGVec y;
  SGVec z;
}
SGVec3D_t;


typedef
struct float3D_s {
  float x;
  float y;
  float z;
}
float3D_t;

#define SGVec3D_ZERO (SGVec3D_t) {SGVec_ZERO, SGVec_ZERO, SGVec_ZERO}

inline
SGVec3D_t SGVec3D_Sub_SGVec3D(SGVec3D_t a, SGVec3D_t b) {
  return (SGVec3D_t) {
    .x = a.x - b.x,
    .y = a.y - b.y,
    .z = a.z - b.z
  };
}

inline
SGVec SGVec3D_dot(SGVec3D_t a, SGVec3D_t b) {
  return
  SGVec_Add_Mult_SGVec(
    SGVec_Add_Mult_SGVec(
      SGVec_Mult_SGVec(a.x, b.x),
      a.y,
      b.y
    ),
    a.z,
    b.z
  );
}

inline
SGVec3D_t SGVec3D_cross(SGVec3D_t a, SGVec3D_t b) {
  return (SGVec3D_t) {
    .x = SGVec_Sub_SGVec(
      SGVec_Mult_SGVec(a.y, b.z),
      SGVec_Mult_SGVec(a.z, b.y)
    ),
    .y = SGVec_Sub_SGVec(
      SGVec_Mult_SGVec(a.z, b.x),
      SGVec_Mult_SGVec(a.x, b.z)
    ),
    .z = SGVec_Sub_SGVec(
      SGVec_Mult_SGVec(a.x, b.y),
      SGVec_Mult_SGVec(a.y, b.x)
    )
  };
}

inline
SGVec3D_t SGVec3D_normalize(SGVec3D_t vec) {
  SGVec recip_magnitude =
    SGVec_Recip_Sqrt(
      SGVec3D_dot(vec, vec)
    );
    // printf("recip mag: %f %f %f %f\n", SGVec_Get_Lane(recip_magnitude, 0), SGVec_Get_Lane(recip_magnitude, 1), SGVec_Get_Lane(recip_magnitude, 2), SGVec_Get_Lane(recip_magnitude, 3));
  return (SGVec3D_t) {
    .x = SGVec_Mult_SGVec(vec.x, recip_magnitude),
    .y = SGVec_Mult_SGVec(vec.y, recip_magnitude),
    .z = SGVec_Mult_SGVec(vec.z, recip_magnitude)
  };
}

inline
float3D_t float_normalize(float3D_t vec) {
  float magnitude = sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  // printf("mag: %f\n", magnitude);

  return (float3D_t) {
    .x = vec.x / (float) magnitude,
    .y = vec.y / (float) magnitude,
    .z = vec.z / (float) magnitude
  };
}

inline
SGVec3D_t rot_vec3d(SGVec rots_sin, SGVec rots_cos, SGVec3D_t axis, SGVec3D_t p) {
  //Q
  SGVec q_x = SGVec_Mult_SGVec(axis.x, rots_sin);
  SGVec q_y = SGVec_Mult_SGVec(axis.y, rots_sin);
  SGVec q_z = SGVec_Mult_SGVec(axis.z, rots_sin);
  SGVec q_s = rots_cos;

  //Q x P
  SGVec qXp_x_1 = SGVec_Mult_SGVec(p.x, q_s);
  SGVec qXp_y_1 = SGVec_Mult_SGVec(p.y, q_s);
  SGVec qXp_z_1 = SGVec_Mult_SGVec(p.z, q_s);

  //qXp_[xyz]_2 is 0 since p_s = 0

  SGVec qXp_x_3 = SGVec_Sub_SGVec(
                    SGVec_Mult_SGVec(q_y, p.z),
                    SGVec_Mult_SGVec(q_z, p.y)
                  );
  SGVec qXp_y_3 = SGVec_Sub_SGVec(
                    SGVec_Mult_SGVec(q_z, p.x),
                    SGVec_Mult_SGVec(q_x, p.z)
                  );
  SGVec qXp_z_3 = SGVec_Sub_SGVec(
                    SGVec_Mult_SGVec(q_x, p.y),
                    SGVec_Mult_SGVec(q_y, p.x)
                  );

  SGVec qXp_x = SGVec_Add_SGVec(qXp_x_1, qXp_x_3);
  SGVec qXp_y = SGVec_Add_SGVec(qXp_y_1, qXp_y_3);
  SGVec qXp_z = SGVec_Add_SGVec(qXp_z_1, qXp_z_3);
  SGVec qXp_s = SGVec_Negate(
                  SGVec_Add_SGVec(
                    SGVec_Add_SGVec(
                      SGVec_Mult_SGVec(q_x, p.x),
                      SGVec_Mult_SGVec(q_y, p.y)
                    ),
                    SGVec_Mult_SGVec(q_z, p.z)
                  )
                );

  SGVec nq_x = SGVec_Negate(q_x);
  SGVec nq_y = SGVec_Negate(q_y);
  SGVec nq_z = SGVec_Negate(q_z);
  //nq_s = q_s

  SGVec qXpXnq_x_1 = SGVec_Mult_SGVec(nq_x, qXp_s);
  SGVec qXpXnq_y_1 = SGVec_Mult_SGVec(nq_y, qXp_s);
  SGVec qXpXnq_z_1 = SGVec_Mult_SGVec(nq_z, qXp_s);

  SGVec qXpXnq_x_2 = SGVec_Mult_SGVec(qXp_x, q_s);
  SGVec qXpXnq_y_2 = SGVec_Mult_SGVec(qXp_y, q_s);
  SGVec qXpXnq_z_2 = SGVec_Mult_SGVec(qXp_z, q_s);

  SGVec qXpXnq_x_3 = SGVec_Sub_SGVec(
                       SGVec_Mult_SGVec(qXp_y, nq_z),
                       SGVec_Mult_SGVec(qXp_z, nq_y)
                     );
  SGVec qXpXnq_y_3 = SGVec_Sub_SGVec(
                       SGVec_Mult_SGVec(qXp_z, nq_x),
                       SGVec_Mult_SGVec(qXp_x, nq_z)
                     );
  SGVec qXpXnq_z_3 = SGVec_Sub_SGVec(
                       SGVec_Mult_SGVec(qXp_x, nq_y),
                       SGVec_Mult_SGVec(qXp_y, nq_x)
                     );

  SGVec r_x = SGVec_Add_SGVec(
                SGVec_Add_SGVec(
                  qXpXnq_x_1,
                  qXpXnq_x_2
                ),
                qXpXnq_x_3
              );
  SGVec r_y = SGVec_Add_SGVec(
                SGVec_Add_SGVec(
                  qXpXnq_y_1,
                  qXpXnq_y_2
                ),
                qXpXnq_y_3
              );
  SGVec r_z = SGVec_Add_SGVec(
                SGVec_Add_SGVec(
                  qXpXnq_z_1,
                  qXpXnq_z_2
                ),
                qXpXnq_z_3
              );

  return (SGVec3D_t) {
    .x = r_x,
    .y = r_y,
    .z = r_z
  };
}

inline
SGVec SGVec3D_distance(SGVec3D_t a, SGVec3D_t b) {
  SGVec3D_t a2b = (SGVec3D_t) {
    .x = SGVec_Sub_SGVec(b.x, a.x),
    .y = SGVec_Sub_SGVec(b.y, a.y),
    .z = SGVec_Sub_SGVec(b.z, a.z)
  };
  SGVec dist2 = SGVec3D_dot(a2b, a2b);

  return SGVec_Reciprocal(SGVec_Recip_Sqrt(dist2));
}

inline
float float3D_distance(float3D_t a, float3D_t b) {
  float bSUBa_x = b.x - a.x;
  float bSUBa_y = b.y - a.y;
  float bSUBa_z = b.z - a.z;

  return sqrtf(bSUBa_x * bSUBa_x + bSUBa_y * bSUBa_y + bSUBa_z * bSUBa_z);
}

#endif /* end of include guard: VECTOR_3D_H */
