#ifndef SIMD_H
#define SIMD_H

#include <stdbool.h>

#define ARM
// #define INTEL
// #define AMD


#if defined(ARM)
#   include <arm_neon.h>

//TYPES
  //float
#   define SGVec                                float32x4_t
#   define SGVecShort                           float32x2_t
  //int
#   define SGVecUInt                            uint32x4_t
#   define SGVecShortUInt                       uint32x2_t

//LOAD
  //float
#   define SGVec_Load_Const(X)                  vdupq_n_f32((X))
#   define SGVec_Load_Array(X)                  vld1q_f32((X))
  //int
#   define SGVecUInt_Load_Const(X)              vdupq_n_u32((X))
#   define SGVecUInt_Load_Array(X)              vld1q_u32((X))
#   define SGVecUInt_Load_Lane(X, Y, Z)         vld1q_lane_u32((X), (Y), (Z))

//STORE
  //float
#   define SGVec_Store_Array(X, Y)              vst1q_f32((X), (Y))
#   define SGVec_First_Lane(X)                  vgetq_lane_f32((X), 0)

  //int
#   define SGVecUInt_Store_Array(X, Y)          vst1q_u32((X), (Y))
#endif

#if defined(INTEL) || defined(AMD)
#   include <immintrin.h>
//TYPES
  //float
#   define SGVec                                __m128
  //int
#   define SGVecUInt                            __m128i

//LOAD
  //float
#   define SGVec_Load_Const(X)                  _mm_set1_ps((X))
#   define SGVec_Load_Array(X)                  _mm_loadu_ps((X))
  //int
#   define SGVecUInt_Load_Const(X)              _mm_set1_epi32((X))
#   define SGVecUInt_Load_Array(X)              _mm_loadu_si32((X))

//STORE
  //float
#   define SGVec_Store_Array(X, Y)              _mm_store_ps((X), (Y))
#   define SGVec_First_Lane(X)                  _mm_cvtss_f32((X))

  //int
#   define SGVecUInt_Store_Array(X, Y)          _mm_storeu_si128((X), (Y))
#endif

//float
#if defined(ARM)
#   define SGVec_ZERO                           SGVec_Load_Const(0.0)
#endif
#if defined(INTEL) || defined(AMD)
#   define SGVec_ZERO                           _mm_setzero_ps()
#endif
#define SGVec_ONE                               SGVec_Load_Const(1.0)
//int
#define SGVecUInt_ZERO                          SGVecUInt_Load_Const(0)
#define SGVecUInt_ONE                           SGVecUInt_Load_Const(1)

#if defined(ARM)
//MATH
  //float
#   define SGVec_Add_SGVec(X, Y)                vaddq_f32((X), (Y))
#   define SGVec_Sub_SGVec(X, Y)                vsubq_f32((X), (Y))
#   define SGVec_Mult_SGVec(X, Y)               vmulq_f32((X), (Y))

#   define SGVec_Add_Mult_SGVec(X, Y, Z)        vmlaq_f32((X), (Y), (Z))
#   define SGVec_Mult_Float(X, Y)               vmulq_n_f32((X), (Y))

#   define SGVec_Negate(X)                      vnegq_f32((X))
#   define SGVec_Absolute(X)                    vabsq_f32((X))

#   define SGVec_Minimum(X, Y)                  vminq_f32((X), (Y))
#   define SGVec_Maximum(X, Y)                  vmaxq_f32((X), (Y))

#   define SGVec_Recip_Sqrt(X)                  vrsqrteq_f32((X))
#   define SGVec_Reciprocal(X)                  vrecpeq_f32((X))

#   define SGVec_Floor(X)                       vrndq_f32((X))
  //int
// #   define SGVecUInt_Mult_UInt(X, Y)            vmulq_n_u32((X), (Y))
#   define SGVecUInt_Sub_SGVecUInt(X, Y)        vsubq_u32((X), (Y))

//LOGIC
  //float
  //int
#   define SGVecUInt_Or(X, Y)                   vorrq_u32((X), (Y))
#   define SGVecUInt_And(X, Y)                  vandq_u32((X), (Y))
#   define SGVecUInt_Xor(X, Y)                  veorq_u32((X), (Y))

//CONDITIONAL
  //float
#   define SGVec_Less_Than(X, Y)                vcltq_f32((X), (Y))
#   define SGVec_Less_Or_Eq_Than(X, Y)          vcleq_f32((X), (Y))
#   define SGVec_Gtr_Than(X, Y)                 vcgtq_f32((X), (Y))
#   define SGVec_Gtr_Or_Eq_Than(X, Y)           vcgeq_f32((X), (Y))

#   define SGVec_Ternary(X, Y, Z)               vbslq_f32((X), (Y), (Z))
  //int
#   define SGVecUInt_Ternary(X, Y, Z)           vbslq_u32((X), (Y), (Z))


//CAST
  //float
// #   define SGVecUInt_Cast_SGVec(X)              vcvtq_u32_f32((X))
// #   define SGVecInt_Cast_SGVec(X)               vcvtq_s32_f32((X))

  //int
// #   define SGVec_Cast_SGVecUInt(X)              vcvtq_f32_u32((X))
// #   define SGVec_Cast_SGVecInt(X)               vcvtq_f32_s32((X))
// #   define SGVecInt_Cast_SGVecUInt(X)           vreinterpretq_u32_s32((X))
#endif

#if defined(INTEL) || defined(AMD)
//MATH
  //float
#   define SGVec_Add_SGVec(X, Y)                _mm_add_ps((X), (Y))
#   define SGVec_Sub_SGVec(X, Y)                _mm_sub_ps((X), (Y))
#   define SGVec_Mult_SGVec(X, Y)               _mm_mul_ps((X), (Y))

#   define SGVec_Add_Mult_SGVec(X, Y, Z)        SGVec_Add_SGVec((X), SGVec_Mult_SGVec((Y), (Z)))
  //_mm_fmadd_ps((Y), (Z), (X)) //FMA extension required
#   define SGVec_Mult_Float(X, Y)               SGVec_Mult_SGVec((X), SGVec_Load_Const((Y))) //DOESNT EXIST

#   define SGVec_Negate(X)                      _mm_xor_ps(SGVec_Load_Const(-0.f), (X)) //SGVec_Sub_SGVec(SGVec_ZERO, (X)) //DOESNT EXIST
#   define SGVec_Absolute(X)                    _mm_andnot_ps(SGVec_Load_Const(-0.f), (X))

#   define SGVec_Minimum(X, Y)                  _mm_min_ps((X), (Y))
#   define SGVec_Maximum(X, Y)                  _mm_max_ps((X), (Y))

#   define SGVec_Recip_Sqrt(X)                  _mm_rsqrt_ps((X))
#   define SGVec_Reciprocal(X)                  _mm_rcp_ps((X))
#   define SGVec_Sqrt(X)                        _mm_sqrt_ps((X))
  //int
// #   define SGVecUInt_Mult_UInt(X, Y)            vmulq_n_u32((X), (Y))

//LOGIC
  //float
  //int
#   define SGVecUInt_Or(X, Y)                   _mm_or_si128((X), (Y))
#   define SGVecUInt_And(X, Y)                  _mm_and_si128((X), (Y))
#   define SGVecUInt_Xor(X, Y)                  _mm_xor_si128((X), (Y))

//CONDITIONAL
  //float
#   define SGVec_Less_Than(X, Y)                _mm_cmplt_ps((X), (Y))
#   define SGVec_Less_Or_Eq_Than(X, Y)          _mm_cmple_ps((X), (Y))
#   define SGVec_Gtr_Than(X, Y)                 _mm_cmpgt_ps((X), (Y))
#   define SGVec_Gtr_Or_Eq_Than(X, Y)           _mm_cmpge_ps((X), (Y))

#   define SGVec_Ternary(X, Y, Z)               _mm_blendv_ps((Y), (Z), (X))
  //int
#   define SGVecUInt_Ternary(X, Y, Z)           _mm_blendv_epi8((Y), (Z), (X))
#endif



#if defined(ARM)
#   define SGVecShortUInt_Fold_Min(X, Y)        vpmin_u32((X), (Y))
#   define SGVecShortUInt_Fold_Max(X, Y)        vpmax_u32((X), (Y))
#   define SGVecUInt_Top_Short(X)               vget_high_u32((X))
#   define SGVecUInt_Bottom_Short(X)            vget_low_u32((X))
#   define SGVecShortUInt_Get_Lane(X, Y)        vget_lane_u32((X), (Y))

#   define SGVecShort_Fold_Min(X, Y)            vpmin_f32((X), (Y))
#   define SGVecShort_Fold_Max(X, Y)            vpmax_f32((X), (Y))
#   define SGVec_Top_Short(X)                   vget_high_f32((X))
#   define SGVec_Bottom_Short(X)                vget_low_f32((X))
#   define SGVecShort_Get_Lane(X, Y)            vget_lane_f32((X), (Y))
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

  #define steal_lane(source, mask, lane) { \
    static const int all_one = ~0; \
    source = SGVec_Ternary(SGVecUInt_Load_Lane(&all_one, SGVecUInt_ZERO, lane), mask, source); \
  }

  // inline
  // float max_lane(SGVec a) {
  //   SGVecShort max_temp = SGVecShort_Fold_Max(SGVec_Bottom_Short(a), SGVec_Top_Short(a));
  //   return SGVecShort_Get_Lane(SGVecShort_Fold_Max(max_temp, max_temp), 0);
  // }
  //
  // inline
  // float min_lane(SGVec a) {
  //   SGVecShort min_temp = SGVecShort_Fold_Min(SGVec_Bottom_Short(a), SGVec_Top_Short(a));
  //   return SGVecShort_Get_Lane(SGVecShort_Fold_Min(min_temp, min_temp), 0);
  // }
#endif

#if defined(INTEL) || defined(AMD)
  inline
  bool lanes_true(SGVecUInt x) {
    return _mm_test_all_zeros(SGVecUInt_Xor(x, SGVecUInt_Load_Const(~0)), SGVecUInt_Load_Const(~0));
  }

  inline
  bool lanes_false(SGVecUInt x) {
    return _mm_test_all_zeros(x, SGVecUInt_Load_Const(~0));
  }

  #define steal_lane(source, mask, lane) { \
     source = _mm_blend_ps(source, mask, 1 << lane); \
  }
  //
  // inline
  // float max_lane(SGVec a) {
  //   SGVecShort max_temp = SGVecShort_Fold_Max(SGVec_Bottom_Short(a), SGVec_Top_Short(a));
  //   return SGVecShort_Get_Lane(SGVecShort_Fold_Max(max_temp, max_temp), 0);
  // }
  //
  // inline
  // float min_lane(SGVec a) {
  //   SGVecShort min_temp = SGVecShort_Fold_Min(SGVec_Bottom_Short(a), SGVec_Top_Short(a));
  //   return SGVecShort_Get_Lane(SGVecShort_Fold_Min(min_temp, min_temp), 0);
  // }
#endif

#endif /* end of include guard: SIMD_H */
