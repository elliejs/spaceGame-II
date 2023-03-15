#include "world_server.h"
#include "world_control.h"

void request_thrust(unsigned int id, float amt) {
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].origin = (SGVec3D_t) {
    .x =
      SGVec_Add_Mult_SGVec(
        world_server->players[id].origin.x,
        SGVec_Load_Const(amt),
        world_server->players[id].ship.frame.forward.x
      ),
    .y =
      SGVec_Add_Mult_SGVec(
        world_server->players[id].origin.y,
        SGVec_Load_Const(amt),
        world_server->players[id].ship.frame.forward.y
      ),
    .z =
      SGVec_Add_Mult_SGVec(
        world_server->players[id].origin.z,
        SGVec_Load_Const(amt),
        world_server->players[id].ship.frame.forward.z
      )
  };

  float x = SGVec_First_Lane(world_server->players[id].origin.x);
  float y = SGVec_First_Lane(world_server->players[id].origin.y);
  float z = SGVec_First_Lane(world_server->players[id].origin.z);

  world_server->players[id].ship.abs_coord.x += (x >= CHUNK_SIZE) - (x < 0);
  world_server->players[id].ship.abs_coord.y += (y >= CHUNK_SIZE) - (y < 0);
  world_server->players[id].ship.abs_coord.z += (z >= CHUNK_SIZE) - (z < 0);

  world_server->players[id].origin = (SGVec3D_t) {
    .x =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_server->players[id].origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_server->players[id].origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Ternary(
          SGVec_Less_Than(
            world_server->players[id].origin.x,
            SGVec_ZERO
          ),
          SGVec_Add_SGVec(
            world_server->players[id].origin.x,
            SGVec_Load_Const((float) CHUNK_SIZE)
          ),
          world_server->players[id].origin.x
        )
      ),
    .y =
    SGVec_Ternary(
      SGVec_Gtr_Or_Eq_Than(
        world_server->players[id].origin.y,
        SGVec_Load_Const((float) CHUNK_SIZE)
      ),
      SGVec_Sub_SGVec(
        world_server->players[id].origin.y,
        SGVec_Load_Const((float) CHUNK_SIZE)
      ),
      SGVec_Ternary(
        SGVec_Less_Than(
          world_server->players[id].origin.y,
          SGVec_ZERO
        ),
        SGVec_Add_SGVec(
          world_server->players[id].origin.y,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        world_server->players[id].origin.y
      )
    ),
    .z =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_server->players[id].origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_server->players[id].origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Ternary(
          SGVec_Less_Than(
            world_server->players[id].origin.z,
            SGVec_ZERO
          ),
          SGVec_Add_SGVec(
            world_server->players[id].origin.z,
            SGVec_Load_Const((float) CHUNK_SIZE)
          ),
          world_server->players[id].origin.z
        )
      )
  };
  MTX_UNLOCK(world_server->player_mtxs + id);
  // printf("after movement:\n");
  // printf("\tchunk_id:\n\t\t%d\n\t\t%d\n\t\t%d\n",
  //   world_server->players[id].chunk_id.x,
  //   world_server->players[id].chunk_id.y,
  //   world_server->players[id].chunk_id.z);
  // printf("\tlocation:\n\t\t%f\n\t\t%f\n\t\t%f\n",
  //   SGVec_Get_Lane(world_server->players[id].origin.x, 0),
  //   SGVec_Get_Lane(world_server->players[id].origin.y, 0),
  //   SGVec_Get_Lane(world_server->players[id].origin.z, 0));
}
void request_yaw(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_server->player_mtxs + id);
  SGVec4D_t rot_quat = prepare_rot_quat(amt_sin, amt_cos, world_server->players[id].ship.frame.up);
  world_server->players[id].ship.orientation = SGVec4D_Mult_SGVec4D(rot_quat, world_server->players[id].ship.orientation);
  world_server->players[id].ship.frame.forward = rot_vec3d(rot_quat, world_server->players[id].ship.frame.forward);
  world_server->players[id].ship.frame.right = rot_vec3d(rot_quat, world_server->players[id].ship.frame.right);
  MTX_UNLOCK(world_server->player_mtxs + id);
}
void request_pitch(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_server->player_mtxs + id);
  SGVec4D_t rot_quat = prepare_rot_quat(amt_sin, amt_cos, world_server->players[id].ship.frame.right);
  world_server->players[id].ship.orientation = SGVec4D_Mult_SGVec4D(rot_quat, world_server->players[id].ship.orientation);
  world_server->players[id].ship.frame.forward = rot_vec3d(rot_quat, world_server->players[id].ship.frame.forward);
  world_server->players[id].ship.frame.up = rot_vec3d(rot_quat, world_server->players[id].ship.frame.up);
  MTX_UNLOCK(world_server->player_mtxs + id);
}
void request_roll(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_server->player_mtxs + id);
  SGVec4D_t rot_quat = prepare_rot_quat(amt_sin, amt_cos, world_server->players[id].ship.frame.forward);
  world_server->players[id].ship.orientation = SGVec4D_Mult_SGVec4D(rot_quat, world_server->players[id].ship.orientation);
  world_server->players[id].ship.frame.up = rot_vec3d(rot_quat, world_server->players[id].ship.frame.up);
  world_server->players[id].ship.frame.right = rot_vec3d(rot_quat, world_server->players[id].ship.frame.right);
  MTX_UNLOCK(world_server->player_mtxs + id);
}

void request_vision(unsigned int id, vision_mode_t mode) {
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].ship.vision ^= world_server->players[id].ship.vision | mode;
  MTX_UNLOCK(world_server->player_mtxs + id);
}
