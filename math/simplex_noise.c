// #include "simplex_noise.h"
//
// SGVec simplex_noise(SGVec3D_t point) {
//   const float F = 1 / 3.;
//   const SGVec s = SGVec_Mult_Float(SGVec_Add_SGVec(SGVec_Add_SGVec(point.x, point.y), point.z), F);
//
//   const SGVec i = SGVec_Floor(SGVec_Add_SGVec(point.x, s));
//   const SGVec j = SGVec_Floor(SGVec_Add_SGVec(point.y, s));
//   const SGVec k = SGVec_Floor(SGVec_Add_SGVec(point.z, s));
//
//   const float G = 1 / 6.;
//   const SGVec t = SGVec_Mult_Float(SGVec_Add_SGVec(SGVec_Add_SGVec(i, j), k), G);
//
//   const SGVec X_0 = SGVec_Sub_SGVec(i, t);
//   const SGVec Y_0 = SGVec_Sub_SGVec(j, t);
//   const SGVec Z_0 = SGVec_Sub_SGVec(k, t);
//
//   const SGVec x_0 = SGVec_Sub_SGVec(point.x, X_0);
//   const SGVec y_0 = SGVec_Sub_SGVec(point.y, Y_0);
//   const SGVec z_0 = SGVec_Sub_SGVec(point.z, Z_0);
//
//   const SGVec i_1 = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, y_0),
//     SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(y_0, z_0),
//       SGVec_ONE,
//       SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, z_0),
//         SGVec_ONE,
//         SGVec_ZERO
//       )
//     ),
//     SGVec_ZERO
//   );
//
//   const SGVec j_1 = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, y_0),
//     SGVec_ZERO,
//     SGVec_Ternary(SGVec_Less_Than(y_0, z_0),
//       SGVec_ZERO,
//       SGVec_ONE
//     )
//   );
//
//   const SGVec k_1 = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, y_0),
//     SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(y_0, z_0),
//       SGVec_ZERO,
//       SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, z_0),
//         SGVec_ZERO,
//         SGVec_ONE
//       )
//     ),
//     SGVec_Ternary(SGVec_Less_Than(y_0, z_0),
//       SGVec_ONE,
//       SGVec_ZERO
//     )
//   );
//
//   const SGVec i_2 = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, y_0),
//     SGVec_ONE,
//     SGVec_Ternary(SGVec_Less_Than(y_0, z_0),
//       SGVec_ZERO,
//       SGVec_Ternary(SGVec_Less_Than(x_0, z_0),
//         SGVec_ZERO,
//         SGVec_ONE
//       )
//     )
//   );
//
//   const SGVec j_2 = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, y_0),
//     SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(y_0, z_0),
//       SGVec_ONE,
//       SGVec_ZERO
//     ),
//     SGVec_ONE
//   );
//
//   const SGVec k_2 = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(x_0, y_0),
//     SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(y_0, z_0),
//       SGVec_ZERO,
//       SGVec_ONE
//     ),
//     SGVec_Ternary(SGVec_Less_Than(y_0, z_0),
//       SGVec_ONE,
//       SGVec_Ternary(SGVec_Less_Than(x_0, z_0),
//         SGVec_ONE,
//         SGVec_ZERO
//       )
//     )
//   );
//
//   const SGVec x_1 = SGVec_Add_SGVec(SGVec_Sub_SGVec(x_0, i_1), SGVec_Load_Const(G));
//   const SGVec y_1 = SGVec_Add_SGVec(SGVec_Sub_SGVec(y_0, j_1), SGVec_Load_Const(G));
//   const SGVec z_1 = SGVec_Add_SGVec(SGVec_Sub_SGVec(z_0, k_1), SGVec_Load_Const(G));
//
//   const SGVec x_2 = SGVec_Add_SGVec(SGVec_Sub_SGVec(x_0, i_2), SGVec_Load_Const(2. * G));
//   const SGVec y_2 = SGVec_Add_SGVec(SGVec_Sub_SGVec(y_0, j_2), SGVec_Load_Const(2. * G));
//   const SGVec z_2 = SGVec_Add_SGVec(SGVec_Sub_SGVec(z_0, k_2), SGVec_Load_Const(2. * G));
//
//   const SGVec x_3 = SGVec_Add_SGVec(SGVec_Sub_SGVec(x_0, SGVec_ONE), SGVec_Load_Const(3. * G));
//   const SGVec y_3 = SGVec_Add_SGVec(SGVec_Sub_SGVec(y_0, SGVec_ONE), SGVec_Load_Const(3. * G));
//   const SGVec z_3 = SGVec_Add_SGVec(SGVec_Sub_SGVec(z_0, SGVec_ONE), SGVec_Load_Const(3. * G));
//
//
//   const SGVecUInt i_uint = SGVecUInt_Cast_SGVec(i);
//   const SGVecUInt j_uint = SGVecUInt_Cast_SGVec(j);
//   const SGVecUInt k_uint = SGVecUInt_Cast_SGVec(k);
//
//
//   SGVec3D_t gradient_0;
//   SGVec3D_t gradient_1;
//   SGVec3D_t gradient_2;
//   SGVec3D_t gradient_3;
//
//   SGVecUInt gi_0;
//   SGVecUInt gi_1;
//   SGVecUInt gi_2;
//   SGVecUInt gi_3;
//
//   SGVecUInt xor_magic;
//
//   xor_magic = SGVec_Load_Const(0b11101000111011100000001011110010);
//
//   gi_0 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(i_uint, j_uint)), SGVecUInt_Load_Const(0b11));
//   gi_1 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(j_uint, k_uint)), SGVecUInt_Load_Const(0b11));
//   gi_2 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(k_uint, i_uint)), SGVecUInt_Load_Const(0b11));
//   gi_3 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(SGVecUInt_Xor(i_uint, j_uint), k_uint)), SGVecUInt_Load_Const(0b11));
//
//   gradient_0.x = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_0, SGVecUInt_ONE)));
//   gradient_1.x = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_1, SGVecUInt_ONE)));
//   gradient_2.x = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_2, SGVecUInt_ONE)));
//   gradient_3.x = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_3, SGVecUInt_ONE)));
//
//   xor_magic = SGVec_Load_Const(0b11110001101010110100001010011001);
//
//   gi_0 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(i_uint, j_uint)), SGVecUInt_Load_Const(0b11));
//   gi_1 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(j_uint, k_uint)), SGVecUInt_Load_Const(0b11));
//   gi_2 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(k_uint, i_uint)), SGVecUInt_Load_Const(0b11));
//   gi_3 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(SGVecUInt_Xor(i_uint, j_uint), k_uint)), SGVecUInt_Load_Const(0b11));
//
//   gradient_0.y = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_0, SGVecUInt_ONE)));
//   gradient_1.y = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_1, SGVecUInt_ONE)));
//   gradient_2.y = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_2, SGVecUInt_ONE)));
//   gradient_3.y = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_3, SGVecUInt_ONE)));
//
//   xor_magic = SGVec_Load_Const(0b10110010000101011100111110011111);
//
//   gi_0 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(i_uint, j_uint)), SGVecUInt_Load_Const(0b11));
//   gi_1 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(j_uint, k_uint)), SGVecUInt_Load_Const(0b11));
//   gi_2 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(k_uint, i_uint)), SGVecUInt_Load_Const(0b11));
//   gi_3 = SGVecUInt_And(SGVecUInt_Xor(xor_magic, SGVecUInt_Xor(SGVecUInt_Xor(i_uint, j_uint), k_uint)), SGVecUInt_Load_Const(0b11));
//
//   gradient_0.z = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_0, SGVecUInt_ONE)));
//   gradient_1.z = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_1, SGVecUInt_ONE)));
//   gradient_2.z = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_2, SGVecUInt_ONE)));
//   gradient_3.z = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVecUInt(SGVecUInt_Sub_SGVecUInt(gi_3, SGVecUInt_ONE)));
//
//   const SGVec t_0 = SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Load_Const(0.5), SGVec_Mult_SGVec(x_0, x_0)), SGVec_Mult_SGVec(y_0, y_0)), SGVec_Mult_SGVec(z_0, z_0));
//   const SGVec t_1 = SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Load_Const(0.5), SGVec_Mult_SGVec(x_1, x_1)), SGVec_Mult_SGVec(y_1, y_1)), SGVec_Mult_SGVec(z_1, z_1));
//   const SGVec t_2 = SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Load_Const(0.5), SGVec_Mult_SGVec(x_2, x_2)), SGVec_Mult_SGVec(y_2, y_2)), SGVec_Mult_SGVec(z_2, z_2));
//   const SGVec t_3 = SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Sub_SGVec(SGVec_Load_Const(0.5), SGVec_Mult_SGVec(x_3, x_3)), SGVec_Mult_SGVec(y_3, y_3)), SGVec_Mult_SGVec(z_3, z_3));
//
//   const SGVec n_0 = SGVec_Ternary(SGVec_Less_Than(t_0, SGVec_ZERO),
//     SGVec_ZERO,
//     SGVec_Mult_SGVec(
//       SGVec_Mult_SGVec(SGVec_Mult_SGVec(SGVec_Mult_SGVec(t_0, t_0), t_0), t_0),
//       SGVec3D_dot(gradient_0, (SGVec3D_t) {
//         .x = x_0,
//         .y = y_0,
//         .z = z_0
//       })
//     )
//   );
//
//   const SGVec n_1 = SGVec_Ternary(SGVec_Less_Than(t_1, SGVec_ZERO),
//     SGVec_ZERO,
//     SGVec_Mult_SGVec(
//       SGVec_Mult_SGVec(SGVec_Mult_SGVec(SGVec_Mult_SGVec(t_1, t_1), t_1), t_1),
//       SGVec3D_dot(gradient_1, (SGVec3D_t) {
//         .x = x_1,
//         .y = y_1,
//         .z = z_1
//       })
//     )
//   );
//
//   const SGVec n_2 = SGVec_Ternary(SGVec_Less_Than(t_2, SGVec_ZERO),
//     SGVec_ZERO,
//     SGVec_Mult_SGVec(
//       SGVec_Mult_SGVec(SGVec_Mult_SGVec(SGVec_Mult_SGVec(t_2, t_2), t_2), t_2),
//       SGVec3D_dot(gradient_2, (SGVec3D_t) {
//         .x = x_2,
//         .y = y_2,
//         .z = z_2
//       })
//     )
//   );
//
//   const SGVec n_3 = SGVec_Ternary(SGVec_Less_Than(t_3, SGVec_ZERO),
//     SGVec_ZERO,
//     SGVec_Mult_SGVec(
//       SGVec_Mult_SGVec(SGVec_Mult_SGVec(SGVec_Mult_SGVec(t_3, t_3), t_3), t_3),
//       SGVec3D_dot(gradient_3, (SGVec3D_t) {
//         .x = x_3,
//         .y = y_3,
//         .z = z_3
//       })
//     )
//   );
//
//   return SGVec_Mult_Float(SGVec_Add_SGVec(SGVec_Add_SGVec(SGVec_Add_SGVec(n_0, n_1), n_2), n_3), 32.0);
// }
