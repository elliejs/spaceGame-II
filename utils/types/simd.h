#ifndef SIMD_H
#define SIMD_H

#include <stdbool.h>

#define NEON

#if defined(NEON)
#   include <arm_neon.h>
#   define SGVec                                float32x4_t
#   define SGVecUInt                            uint32x4_t
#   define SGVecShortUInt                       uint32x2_t
#   define SGVecShort                           float32x2_t
#   define SGVec_Load_Array(X)                  vld1q_f32((X))
#   define SGVecUInt_Store_Array(X, Y)          vst1q_u32((X), (Y))
#   define SGVec_Store_Array(X, Y)              vst1q_f32((X), (Y))
#   define SGVec_Load_Const(X)                  vdupq_n_f32((X))
#   define SGVecUInt_Load_Const(X)              vdupq_n_u32((X))
#   define SGVecUInt_Load_Array(X)              vld1q_u32((X))
#   define SGVec_Negate(X)                      vnegq_f32((X))
#   define SGVec_Mult_SGVec(X, Y)               vmulq_f32((X), (Y))
#   define SGVec_Add_Mult_SGVec(X, Y, Z)        vmlaq_f32((X), (Y), (Z))
#   define SGVec_Mult_Float(X, Y)               vmulq_n_f32((X), (Y))
#   define SGVecUInt_Mult_UInt(X, Y)            vmulq_n_u32((X), (Y))
#   define SGVec_Add_SGVec(X, Y)                vaddq_f32((X), (Y))
#   define SGVec_Sub_SGVec(X, Y)                vsubq_f32((X), (Y))
#   define SGVec_Minimum(X, Y)                  vminq_f32((X), (Y))
#   define SGVec_Maximum(X, Y)                  vmaxq_f32((X), (Y))
#   define SGVec_Absolute(X)                    vabsq_f32((X))
#   define SGVec_Less_Than(X, Y)                vcltq_f32((X), (Y))
#   define SGVec_Less_Or_Eq_Than(X, Y)          vcleq_f32((X), (Y))
#   define SGVec_Gtr_Or_Eq_Than(X, Y)           vcgeq_f32((X), (Y))
#   define SGVec_Gtr_Than(X, Y)                 vcgtq_f32((X), (Y))
#   define SGVecUInt_Or(X, Y)                   vorrq_u32((X), (Y))
#   define SGVecUInt_And(X, Y)                  vandq_u32((X), (Y))
#   define SGVecUInt_Xor(X, Y)                  veorq_u32((X), (Y))
#   define SGVecUInt_Not(X)                     vmvnq_u32((X))
#   define SGVecUInt_Ternary(X, Y, Z)           vbslq_u32((X), (Y), (Z))
#   define SGVec_Ternary(X, Y, Z)               vbslq_f32((X), (Y), (Z))

#   define SGVecShortUInt_Fold_Min(X, Y)        vpmin_u32((X), (Y))
#   define SGVecShortUInt_Fold_Max(X, Y)        vpmax_u32((X), (Y))
#   define SGVecUInt_Top_Short(X)               vget_high_u32((X))
#   define SGVecUInt_Bottom_Short(X)            vget_low_u32((X))
#   define SGVecShortUInt_Get_Lane(X, Y)        vget_lane_u32((X), (Y))
#   define SGVecUInt_Get_Lane(X, Y)             vgetq_lane_u32((X), (Y))
#   define SGVec_Get_Lane(X, Y)                 vgetq_lane_f32((X), (Y))

#   define SGVecShort_Fold_Min(X, Y)            vpmin_f32((X), (Y))
#   define SGVecShort_Fold_Max(X, Y)            vpmax_f32((X), (Y))
#   define SGVec_Top_Short(X)                   vget_high_f32((X))
#   define SGVec_Bottom_Short(X)                vget_low_f32((X))
#   define SGVecShort_Get_Lane(X, Y)            vget_lane_f32((X), (Y))

#   define SGVec_Recip_Sqrt(X)                  vrsqrteq_f32((X))
#   define SGVec_Reciprocal(X)                  vrecpeq_f32((X))

#   define SGVecUInt_Cast_SGVec(X)              vcvtq_u32_f32((X))
#   define SGVec_Cast_SGVecUInt(X)              vcvtq_f32_u32((X))
#   define SGVecInt_Cast_SGVec(X)               vcvtq_s32_f32((X))
#   define SGVec_Cast_SGVecInt(X)               vcvtq_f32_s32((X))
#   define SGVecUInt_Shift_Right(X, Y)          vshrq_n_u32((X), (Y))
#   define SGVecUInt_Shift_Left(X, Y)           vshlq_n_u32((X), (Y))
#   define SGVecUInt_Shift_Left_Insert(X, Y, Z) vsliq_n_u32((X), (Y), (Z))
#endif


#define SGVecUInt_ZERO                          SGVecUInt_Load_Const(0)
#define SGVec_ZERO                              SGVec_Load_Const(0)
#define SGVec_ONE                               SGVec_Load_Const(1)

inline
bool lanes_true(SGVecUInt x) {
  SGVecShortUInt x_2 = SGVecShortUInt_Fold_Min(SGVecUInt_Bottom_Short(x), SGVecUInt_Top_Short(x));
  return ~(SGVecShortUInt_Get_Lane(SGVecShortUInt_Fold_Min(x_2, x_2), 0)) == 0;
}

inline
bool lanes_false(SGVecUInt x) {
  SGVecShortUInt x_2 = SGVecShortUInt_Fold_Max(SGVecUInt_Bottom_Short(x), SGVecUInt_Top_Short(x));
  return (SGVecShortUInt_Get_Lane(SGVecShortUInt_Fold_Max(x_2, x_2), 0)) == 0;
}

inline
SGVec SGVec_Clamp(SGVec x, float min, float max) {
  return SGVec_Minimum(SGVec_Maximum(x, SGVec_Load_Const(min)), SGVec_Load_Const(max));
}

#endif /* end of include guard: SIMD_H */
