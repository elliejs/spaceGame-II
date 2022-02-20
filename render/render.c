#include "../utils/types/types.h"
#include "render.h"

static const SGFloat fov = (M_PI / 180.) * FOV;

inline
SGVec3D_t rot_vec3d(SGVec rots_sin, SGVec rots_cos, SGVec3D axis, SGVec3D p) {
  //Q
  SGVec q_x = SGVec_Mult_SGVec(axis.x, rots_sin);
  SGVec q_y = SGVec_Mult_SGVec(axis.y, rots_sin);
  SGVec q_z = SGVec_Mult_SGVec(axis.z, rots_sin);
  SGVec q_s = SGVec_Load_Array(rots_cos);

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
pixel_t render_pixel() {
  
}
