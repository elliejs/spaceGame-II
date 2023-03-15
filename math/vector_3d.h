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
struct SGVec4D_s {
  SGVec x;
  SGVec y;
  SGVec z;
  SGVec w;
}
SGVec4D_t;

#define SGVec3D_ZERO (SGVec3D_t)     {SGVec_ZERO, SGVec_ZERO, SGVec_ZERO}

#define SGVec4D_IDENTITY (SGVec4D_t) {SGVec_ZERO, SGVec_ZERO, SGVec_ZERO, SGVec_ONE}

#define SGFrame_RIGHT (SGVec3D_t)    {SGVec_ONE, SGVec_ZERO, SGVec_ZERO}  // 1, 0, 0
#define SGFrame_UP (SGVec3D_t)       {SGVec_ZERO, SGVec_ONE, SGVec_ZERO}  // 0, 1, 0
#define SGFrame_FORWARD (SGVec3D_t)  {SGVec_ZERO, SGVec_ZERO, SGVec_ONE}  // 0, 0, 1


inline
SGVec3D_t SGVec3D_Add_SGVec3D(SGVec3D_t a, SGVec3D_t b) {
  return (SGVec3D_t) {
    .x = SGVec_Add_SGVec(a.x, b.x),
    .y = SGVec_Add_SGVec(a.y, b.y),
    .z = SGVec_Add_SGVec(a.z, b.z)
  };
}

inline
SGVec3D_t SGVec3D_Sub_SGVec3D(SGVec3D_t a, SGVec3D_t b) {
  return (SGVec3D_t) {
    .x = SGVec_Sub_SGVec(a.x, b.x),
    .y = SGVec_Sub_SGVec(a.y, b.y),
    .z = SGVec_Sub_SGVec(a.z, b.z)
  };
}

inline
SGVec3D_t SGVec3D_Mult_SGVec(SGVec3D_t a, SGVec b) {
  return (SGVec3D_t) {
    .x = SGVec_Mult_SGVec(a.x, b),
    .y = SGVec_Mult_SGVec(a.y, b),
    .z = SGVec_Mult_SGVec(a.z, b)
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
SGVec SGVec4D_dot(SGVec4D_t a, SGVec4D_t b) {
  return
  SGVec_Add_Mult_SGVec(
    SGVec_Add_Mult_SGVec(
      SGVec_Add_Mult_SGVec(
        SGVec_Mult_SGVec(a.x, b.x),
        a.y,
        b.y
      ),
      a.z,
      b.z
    ),
    a.w,
    b.w
  );
}

inline
SGVec4D_t SGVec4D_normalize(SGVec4D_t vec) {
  SGVec recip_magnitude =
    SGVec_Recip_Sqrt(
      SGVec4D_dot(vec, vec)
    );
    // printf("recip mag: %f %f %f %f\n", SGVec_Get_Lane(recip_magnitude, 0), SGVec_Get_Lane(recip_magnitude, 1), SGVec_Get_Lane(recip_magnitude, 2), SGVec_Get_Lane(recip_magnitude, 3));
  return (SGVec4D_t) {
    .x = SGVec_Mult_SGVec(vec.x, recip_magnitude),
    .y = SGVec_Mult_SGVec(vec.y, recip_magnitude),
    .z = SGVec_Mult_SGVec(vec.z, recip_magnitude),
    .w = SGVec_Mult_SGVec(vec.w, recip_magnitude)
  };
}


inline
SGVec4D_t prepare_rot_quat(SGVec rots_sin, SGVec rots_cos, SGVec3D_t axis) {
  return (SGVec4D_t) {
    .x = SGVec_Mult_SGVec(axis.x, rots_sin),
    .y = SGVec_Mult_SGVec(axis.y, rots_sin),
    .z = SGVec_Mult_SGVec(axis.z, rots_sin),
    .w = rots_cos
  };
}

inline
SGVec4D_t SGVec4D_Invert(SGVec4D_t a) {
 return (SGVec4D_t) {
   .x = SGVec_Negate(a.x),
   .y = SGVec_Negate(a.y),
   .z = SGVec_Negate(a.z),
   .w = a.w
 };
}

inline
SGVec4D_t SGVec4D_Mult_SGVec4D(SGVec4D_t a, SGVec4D_t b) {
  return (SGVec4D_t) {
    .x =
      SGVec_Sub_SGVec(
        SGVec_Add_SGVec(
          SGVec_Add_SGVec(
            SGVec_Mult_SGVec(a.w, b.x),
            SGVec_Mult_SGVec(a.x, b.w)
          ),
          SGVec_Mult_SGVec(a.y, b.z)
        ),
        SGVec_Mult_SGVec(a.z, b.y)
      ),
    .y =
      SGVec_Add_SGVec(
        SGVec_Add_SGVec(
          SGVec_Sub_SGVec(
            SGVec_Mult_SGVec(a.w, b.y),
            SGVec_Mult_SGVec(a.x, b.z)
          ),
          SGVec_Mult_SGVec(a.y, b.w)
        ),
        SGVec_Mult_SGVec(a.z, b.x)
      ),
    .z =
      SGVec_Add_SGVec(
        SGVec_Sub_SGVec(
          SGVec_Add_SGVec(
            SGVec_Mult_SGVec(a.w, b.z),
            SGVec_Mult_SGVec(a.x, b.y)
          ),
          SGVec_Mult_SGVec(a.y, b.x)
        ),
        SGVec_Mult_SGVec(a.z, b.w)
      ),
    .w =
      SGVec_Sub_SGVec(
        SGVec_Sub_SGVec(
          SGVec_Sub_SGVec(
            SGVec_Mult_SGVec(a.w, b.w),
            SGVec_Mult_SGVec(a.x, b.x)
          ),
          SGVec_Mult_SGVec(a.y, b.y)
        ),
        SGVec_Mult_SGVec(a.z, b.z)
      )
  };
}

inline
SGVec3D_t rot_vec3d(SGVec4D_t rot_quat, SGVec3D_t p) {
  //Q
  SGVec q_x = rot_quat.x;
  SGVec q_y = rot_quat.y;
  SGVec q_z = rot_quat.z;
  SGVec q_s = rot_quat.w;

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

#endif /* end of include guard: VECTOR_3D_H */
