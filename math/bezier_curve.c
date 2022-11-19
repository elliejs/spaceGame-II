// typedef
// struct SGVecBezier_s {
//   SGVec3D_t ** points;
//   int num_points[2];
//   // SGVec * coeffs;
// }
// SGVecBezier_t;
//
// SGVecBezier_t SGVec_create_bezier_solid(SGVec3D_t ** points, int num_points_t, int num_points_u) {
//   SGVecBezier_t ret = (SGVecBezier_t) {
//     .points = points,
//     .num_points = {num_points_t, num_points_u}
//     // .coeffs = malloc(num_points * sizeof(SGVec))
//   };
//
//   // for(int i = 0; i < num_points; i++) {
//   //   ret.coeffs[i] = binom_coeff(num_points, i);
//   // }
//
//   return ret;
// }
//
// static inline
// int fact(int n) {
//   int c = n;
//   int r = n;
//   while(c --> 1) r *= c;
//   return r;
// }
//
// static inline
// SGVec binom_coeff(int n, int i) {
//   return SGVec_Load_Const(fact(n) / (fact(i) * fact(n-i)));
// }
//
// /*
// b{i,n}(x)
// = {n choose i} * x^i * (1-x)^(n-i)
// */
// static inline
// float bernstein(int i, int n, float x) {
//   return binom_coeff(n, i) * powf(x, i) * powf(1 - x, n - i);
// }
//
// typedef enum parametric_axis_e {T, U} parametric_axis_t;
//
// /*
// B(t,u)
// = SUM(i->0..m) SUM(j->0..n) {b{i,m}(t) * b{j,n}(u) * P{i,j}}
// */
// static inline
// SGVec3D_t bezier(SGVecBezier_t * self, float * tu_point) {
//   SGVec ret_x = SGVec_ZERO;
//   SGVec ret_y = SGVec_ZERO;
//   SGVec ret_z = SGVec_ZERO;
//
//   for(int i = 0; i < self->num_points[T]; i++) {
//     float coeff_t = bernstein(i, self->num_points[T], tu_point[T]);
//     for(int j = 0; j < self->num_points[U]; j++) {
//       float coeff_u = bernstein(j, self->num_points[U], tu_point[U]);
//       ret_x += coeff_t * coeff_u * self->points[i][j].x;
//       ret_y += coeff_t * coeff_u * self->points[i][j].y;
//       ret_z += coeff_t * coeff_u * self->points[i][j].z;
//     }
//   }
//
//   return (SGVec3D_t) {
//     .x = ret_x,
//     .y = ret_y,
//     .z = ret_z
//   };
// }
//
// /*
// B(t,u) d/dt
// = m * SUM(j->0..n){b{j,n}(u) * SUM(i->0..m-1){b{i,m-1}(t) * (P{i+1,j} - P{i,j})}}
// */
// static inline
// SGVec3D_t bezier_d(SGVecBezier_t * self, float * tu_point, parametric_axis_t wrt) {
//   SGVec ret_x = SGVec_ZERO;
//   SGVec ret_y = SGVec_ZERO;
//   SGVec ret_z = SGVec_ZERO;
//
//   for(int j = 0; j < self->num_points[!wrt]; j++) {
//     SGVec ret_x_partial = SGVec_ZERO;
//     SGVec ret_y_partial = SGVec_ZERO;
//     SGVec ret_z_partial = SGVec_ZERO;
//     for(int i = 0; i < self->num_points[wrt] - 1; i++) {
//       float coeff_wrt = bernstein(i, self->num_points[wrt] - 1, tu_point[wrt]);
//       ret_x_partial += coeff_wrt * (self->points[i + !wrt][j + wrt].x - self->points[i][j].x);
//       ret_y_partial += coeff_wrt * (self->points[i + !wrt][j + wrt].y - self->points[i][j].y);
//       ret_z_partial += coeff_wrt * (self->points[i + !wrt][j + wrt].z - self->points[i][j].z);
//     }
//     float coeff_NOTwrt = bernstein(j, self->num_points[!wrt], tu_point[!wrt]);
//     ret_x += coeff_NOTwrt * ret_x_partial;
//     ret_y += coeff_NOTwrt * ret_y_partial;
//     ret_z += coeff_NOTwrt * ret_z_partial;
//   }
//   return (SGVec3D_t) {
//     .x = self->num_points[wrt] * ret_x,
//     .y = self->num_points[wrt] * ret_y,
//     .z = self->num_points[wrt] * ret_z
//   };
// }
//
// /*
// B(t,u) d/dt d/dt
// = m * (m-1) * SUM(j->0..n){b{j,n}(u) * SUM(i->0..m-2) {b{i,m-2}(t) * (P{i+2,j} - 2*P{i+1,j} + P{i,j})}}
// */
// static inline
// SGVec3D_t bezier_dd(SGVecBezier_t * self, float * tu_point, parametric_axis_t wrt) {
//   SGVec ret_x = SGVec_ZERO;
//   SGVec ret_y = SGVec_ZERO;
//   SGVec ret_z = SGVec_ZERO;
//
//   for(int j = 0; j < self->num_points[!wrt]; j++) {
//     SGVec ret_x_partial = SGVec_ZERO;
//     SGVec ret_y_partial = SGVec_ZERO;
//     SGVec ret_z_partial = SGVec_ZERO;
//     for(int i = 0; i < self->num_points[wrt] - 2; i++) {
//       float coeff_wrt = bernstein(i, self->num_points[wrt] - 2, tu_point[wrt]);
//       ret_x_partial += coeff_wrt * (self->points[i + 2 * !wrt][j + 2 * wrt].x - (2 * self->points[i + !wrt][j + wrt]) + self->points[i][j].x);
//       ret_y_partial += coeff_wrt * (self->points[i + 2 * !wrt][j + 2 * wrt].y - (2 * self->points[i + !wrt][j + wrt]) + self->points[i][j].y);
//       ret_z_partial += coeff_wrt * (self->points[i + 2 * !wrt][j + 2 * wrt].z - (2 * self->points[i + !wrt][j + wrt]) + self->points[i][j].z);
//     }
//     float coeff_NOTwrt = bernstein(j, self->num_points[!wrt], tu_point[!wrt]);
//     ret_x += coeff_NOTwrt * ret_x_partial;
//     ret_y += coeff_NOTwrt * ret_y_partial;
//     ret_z += coeff_NOTwrt * ret_z_partial;
//   }
//
//   return (SGVec3D_t) {
//     .x = self->num_points[wrt] * (self->num_points[wrt] - 1) * ret_x,
//     .y = self->num_points[wrt] * (self->num_points[wrt] - 1) * ret_y,
//     .z = self->num_points[wrt] * (self->num_points[wrt] - 1) * ret_z
//   };
// }
//
// /*
// Distance-Squared Function First Derivative:
//   (B(t) - P)^2 d/dt
//   ... f(g(t)) d/dt = f'(g(t))*g'(t)
//   ... f(g) = g^2
//   ... g(t) = B(t) - P
//   ... f'(g) = 2g
//   ... g'(t) = B'(t) - P
//   = (2g(t))*(B'(t) - P)
//   = 2(B(t) - P)*(B'(t) - P)
//   = (2B(t).x - 2x)*(B'(t).x - x)
//   = (2B(t).x - 2x)B'(t) - (2B(t).x - 2x)x
//   = 2*B(t)*B'(t) - 2*P*B'(t) - 2*P*B(t) + 2*P^2
//   = 2(B(t)*B'(t) - P*B'(t) - P*B(t) + P^2)
// */
// float bezier_dist_squared_d(SGVecBezier_t * self, float * tu_point, SGVec3D_t point, parametric_axis_t wrt) {
//   SGVec3D_t bezier_tu = bezier(self, tu_point);
//   SGVec3D_t bezier_tu_dwrt = bezier_d(self, tu_point, wrt);
//
//   return (SGVec3D_t) {
//     .x = 2 * (bezier_tu.x * bezier_tu_dwrt.x - point.x * bezier_tu_dwrt.x - point.x * bezier_tu.x + p.x * p.x),
//     .y = 2 * (bezier_tu.y * bezier_tu_dwrt.y - point.y * bezier_tu_dwrt.y - point.y * bezier_tu.y + p.y * p.y),
//     .z = 2 * (bezier_tu.z * bezier_tu_dwrt.z - point.z * bezier_tu_dwrt.z - point.z * bezier_tu.z + p.z * p.z)
//   };
// }
//
// /*
// Distance-Squared Function Second Derivative:
//   (2*B(t)*B'(t) - 2*P*B'(t) - 2*P*B(t) + 2*P^2) d/dt
//   = 2*B(t)*B'(t) d/dt - 2*P*B'(t) d/dt - 2*P*B(t) d/dt + 2*P^2 d/dt
//   = 2*B(t)*B'(t) d/dt - 2*P*B''(t) - 2*P*B'(t)
//   ... f(t)g(t) d/dt = f(t)g'(t) + f'(t)g(t)
//   ... f(t) = B(t)
//   ... g(t) = B'(t)
//   ... f'(t) = B'(t)
//   ... g'(t) = B''(t)
//   = 2(B(t)B''(t) + B'(t)B'(t)) - 2*P*B''(t) - 2*P*B'(t)
//   = 2B(t)B''(t) + 2B'(t)^2 - 2*P*B''(t) - 2*P*B'(t)
//   = 2(B(t)B''(t) + B'(t)^2 - P(B''(t) + B'(t)))
// */
// float bezier_dist_squared_dd(SGVecBezier_t * self, float * tu_point, SGVec3D_t point, parametric_axis_t wrt) {
//   SGVec3D_t bezier_tu = bezier(self, tu_point);
//   SGVec3D_t bezier_tu_dwrt = bezier_d(self, tu_point, wrt);
//   SGVec3D_t bezier_tu_ddwrt = bezier_dd(self, tu_point, wrt);
//
//   return (SGVec3D_t) {
//     .x = 2 * (bezier_tu.x * bezier_tu_ddwrt.x + bezier_tu_dwrt.x * bezier_tu_dwrt.x - point.x * (bezier_tu_ddwrt.x + bezier_tu_dwrt.x)),
//     .y = 2 * (bezier_tu.y * bezier_tu_ddwrt.y + bezier_tu_dwrt.y * bezier_tu_dwrt.y - point.y * (bezier_tu_ddwrt.y + bezier_tu_dwrt.y)),
//     .z = 2 * (bezier_tu.z * bezier_tu_ddwrt.z + bezier_tu_dwrt.z * bezier_tu_dwrt.z - point.z * (bezier_tu_ddwrt.z + bezier_tu_dwrt.z))
//   };
// }
//
// float newton_raphson_bezier_dist_squared_d(SGVecBezier_t * self, float * tu_point, SGVec3D_t point, parametric_axis_t wrt) {
//   float dist_squared_tu_dwrt = bezier_dist_squared_d(self, tu_point, point, wrt);
//   float dist_squared_tu_ddwrt = bezier_dist_squared_dd(self, tu_point, point, wrt);
//   return tu_point[wrt] - (dist_squared_tu_dwrt / dist_squared_tu_ddwrt);
// }
//
// // inline bool positive_double_dt_at_t(SGVecBezier_t * self, float t, SGVec3D_t point) {
// //   SGVec3D_t B_t = B(self, t);
// //   SGVec3D_t B_t_dt = B_dt(self, t);
// //   SGVec3D_t B_t_dt_dt = B_dt_dt(self, t);
// //   return
// //     ( B_t    * B_t_dt_dt
// //     + B_t_dt * B_t_dt
// //     - point  * ( B_t_dt_dt
// //                + B_t_dt    )
// //     )
// //     > 0;
// // }
//
// bool positive_bezier_dist_squared_dd(SGVecBezier_t * self, float * tu_point, SGVec3D_t point) {
//   return
//     bezier_dist_squared_dd(self, tu_point, T) > 0
//     &&
//     bezier_dist_squared_dd(self, tu_point, U) > 0
//     ;
// }
//
// SGVec SGVec_bezier_distance(SGVecBezier_t * self, SGVec3D_t point) {
//
// }
