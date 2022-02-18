#ifndef SIMD_H
#define SIMD_H

#define NEON

// #include <immintrin.h>

#if defined(NEON)
#   include <arm_neon.h>
#   define SGVec float32x4_t
#   define SGVec_Load_Array(X) vld1q_f32((X))
#   define SGVec_Load_Const(X) vdupq_n_f32((X))
#endif

#endif
