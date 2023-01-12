#include "ship.h"
#include "../../world/world_control.h"

static
SGVec distance(object_t * self, SGVec3D_t point, unsigned int cube_idx) {
  SGVec3D_t cube_offset = get_cube_offset(cube_idx);

  point = (SGVec3D_t) {
    .x = SGVec_Sub_SGVec(point.x, SGVec_Add_SGVec(self->origin.x, cube_offset.x)),
    .y = SGVec_Sub_SGVec(point.y, SGVec_Add_SGVec(self->origin.y, cube_offset.y)),
    .z = SGVec_Sub_SGVec(point.z, SGVec_Add_SGVec(self->origin.z, cube_offset.z))
  };

  point = rot_vec3d(SGVec4D_Invert(self->ship.rot_quats.up), point);
  point = rot_vec3d(SGVec4D_Invert(self->ship.rot_quats.forward), point);
  // point = rot_vec3d(SGVec4D_Invert(self->ship.rot_quats.right), point);

  point = (SGVec3D_t) {
    .x = SGVec_Absolute(point.x),
    .y = SGVec_Absolute(point.y),
    .z = SGVec_Absolute(point.z)
  };

  SGVec lalb_dot_lalb = SGVec_Add_SGVec(
    SGVec_Mult_SGVec(self->ship.la, self->ship.la),
    SGVec_Mult_SGVec(self->ship.lb, self->ship.lb)
  );

  SGVec f = SGVec_Maximum(
    SGVec_Minimum(
      SGVec_Mult_SGVec(
        SGVec_Sub_SGVec(
          SGVec_Mult_SGVec(self->ship.la, SGVec_Sub_SGVec(self->ship.la, SGVec_Mult_SGVec(point.x, SGVec_Load_Const(2)))),
          SGVec_Mult_SGVec(self->ship.lb, SGVec_Sub_SGVec(self->ship.lb, SGVec_Mult_SGVec(point.y, SGVec_Load_Const(2))))
        ),
        SGVec_Reciprocal(lalb_dot_lalb)
      ),
      SGVec_ONE
    ),
    SGVec_Load_Const(-1.)
  );

  SGVec sign = SGVec_Ternary(SGVec_Gtr_Or_Eq_Than(
    SGVec_Sub_SGVec(
      SGVec_Add_SGVec(
          SGVec_Mult_SGVec(
            point.x,
            self->ship.lb
          ),
          SGVec_Mult_SGVec(
            point.z,
            self->ship.la
          )
      ),
      SGVec_Mult_SGVec(
        self->ship.la,
        self->ship.lb
      )
    ),
  SGVec_ZERO), SGVec_ONE, SGVec_Load_Const(-1.));
  SGVec q_x_prelength_x = SGVec_Sub_SGVec(
    point.x,
    SGVec_Mult_SGVec(
      SGVec_Mult_Float(self->ship.la, 0.5),
      SGVec_Sub_SGVec(SGVec_ONE, f)
    )
  );
  SGVec q_x_prelength_y = SGVec_Sub_SGVec(
    point.z,
    SGVec_Mult_SGVec(
      SGVec_Mult_Float(self->ship.lb, 0.5),
      SGVec_Add_SGVec(SGVec_ONE, f)
    )
  );
  SGVec q_x = SGVec_Mult_SGVec(
    SGVec_Reciprocal(SGVec_Recip_Sqrt(
      SGVec_Add_SGVec(
        SGVec_Mult_SGVec(q_x_prelength_x, q_x_prelength_x),
        SGVec_Mult_SGVec(q_x_prelength_y, q_x_prelength_y)
      )
    )),
    sign);
  SGVec q_y = SGVec_Sub_SGVec(point.y, self->ship.height);

  return SGVec_Add_SGVec(
    SGVec_Minimum(SGVec_Maximum(q_x, q_y), SGVec_ZERO),
    SGVec_Reciprocal(SGVec_Recip_Sqrt(
      SGVec_Add_SGVec(
        SGVec_Mult_SGVec(SGVec_Maximum(SGVec_ZERO, q_x), SGVec_Maximum(SGVec_ZERO, q_x)),
        SGVec_Mult_SGVec(SGVec_Maximum(SGVec_ZERO, q_y), SGVec_Maximum(SGVec_ZERO, q_y))
      )
    ))
  );
}

static
SGVecOKLAB_t color(object_t * self, SGVec3D_t point) {
  const oklab_t basis = linear_srgb_to_oklab((rgb_t) {0.0, 0.5, 0.0});
  return (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(basis.l),
    .a = SGVec_Load_Const(basis.a),
    .b = SGVec_Load_Const(basis.b)
  };
}

object_t create_ship(SGVec3D_t origin, chunk_coord_t abs_coord) {
  return (object_t) {
    .origin = origin,
    .radius = SGVec_Load_Const(30),
    .distance = distance,
    .color = color,
    .ship = (ship_t) {
      .orientation = DEFAULT_ORIENTATION,
      .la = SGVec_Load_Const(24.),
      .lb = SGVec_Load_Const(12.),
      .height = SGVec_Load_Const(1.),
      .rot_quats = DEFAULT_ROT_QUAT,
      .vision = STANDARD,
      .abs_coord = abs_coord
    }
  };
}
