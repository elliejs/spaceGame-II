#ifndef SIMD_H
#define SIMD_H

#define NEON

#if defined(NEON)
#   include <arm_neon.h>
#   define SGVec float32x4_t
#   define SGVec_Load_Array(X)           vld1q_f32((X))
#   define SGVec_Load_Const(X)           vdupq_n_f32((X))
#   define SGVec_Negate(X)               vnegq_f32((X))
#   define SGVec_Mult_SGVec(X, Y)        vmulq_f32((X), (Y))
#   define SGVec_Add_Mult_SGVec(X, Y, Z) vmlaq_f32((X), (Y), (Z))
// #   define SGVec_Mult_Float(X, Y) vmulq_n_f32((X), (Y))
#   define SGVec_Add_SGVec(X, Y)         vaddq_f32((X), (Y))
#   define SGVec_Sub_SGVec(X, Y)         vsubq_f32((X), (Y))
#endif

#endif
