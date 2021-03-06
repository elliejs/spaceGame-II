#include "world_server.h"

void request_thrust(unsigned int id, float amt) {
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].self.origin = (SGVec3D_t) {
    .x =
      SGVec_Add_Mult_SGVec(
        // SGVec_Add_SGVec(
          world_server->players[id].self.origin.x,
          // SGVec_Load_Const((float) CHUNK_SIZE)
        // ),
        SGVec_Load_Const(amt),
        world_server->players[id].self.ship.orientation.forward.x
      ),
    .y =
      SGVec_Add_Mult_SGVec(
        // SGVec_Add_SGVec(
          world_server->players[id].self.origin.y,
          // SGVec_Load_Const((float) CHUNK_SIZE)
        // ),
        SGVec_Load_Const(amt),
        world_server->players[id].self.ship.orientation.forward.y
      ),
    .z =
      SGVec_Add_Mult_SGVec(
        // SGVec_Add_SGVec(
          world_server->players[id].self.origin.z,
          // SGVec_Load_Const((float) CHUNK_SIZE)
        // ),
        SGVec_Load_Const(amt),
        world_server->players[id].self.ship.orientation.forward.z
      )
  };

  float x = SGVec_Get_Lane(world_server->players[id].self.origin.x, 0);
  float y = SGVec_Get_Lane(world_server->players[id].self.origin.y, 0);
  float z = SGVec_Get_Lane(world_server->players[id].self.origin.z, 0);

  world_server->players[id].chunk_id.x += (x >= CHUNK_SIZE) - (x < 0);
  world_server->players[id].chunk_id.y += (y >= CHUNK_SIZE) - (y < 0);
  world_server->players[id].chunk_id.z += (z >= CHUNK_SIZE) - (z < 0);



  world_server->players[id].self.origin = (SGVec3D_t) {
    .x =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_server->players[id].self.origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_server->players[id].self.origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Ternary(
          SGVec_Less_Than(
            world_server->players[id].self.origin.x,
            SGVec_ZERO
          ),
          SGVec_Add_SGVec(
            world_server->players[id].self.origin.x,
            SGVec_Load_Const((float) CHUNK_SIZE)
          ),
          world_server->players[id].self.origin.x
        )
      ),
    .y =
    SGVec_Ternary(
      SGVec_Gtr_Or_Eq_Than(
        world_server->players[id].self.origin.y,
        SGVec_Load_Const((float) CHUNK_SIZE)
      ),
      SGVec_Sub_SGVec(
        world_server->players[id].self.origin.y,
        SGVec_Load_Const((float) CHUNK_SIZE)
      ),
      SGVec_Ternary(
        SGVec_Less_Than(
          world_server->players[id].self.origin.y,
          SGVec_ZERO
        ),
        SGVec_Add_SGVec(
          world_server->players[id].self.origin.y,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        world_server->players[id].self.origin.y
      )
    ),
    .z =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_server->players[id].self.origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_server->players[id].self.origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Ternary(
          SGVec_Less_Than(
            world_server->players[id].self.origin.z,
            SGVec_ZERO
          ),
          SGVec_Add_SGVec(
            world_server->players[id].self.origin.z,
            SGVec_Load_Const((float) CHUNK_SIZE)
          ),
          world_server->players[id].self.origin.z
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
  //   SGVec_Get_Lane(world_server->players[id].self.origin.x, 0),
  //   SGVec_Get_Lane(world_server->players[id].self.origin.y, 0),
  //   SGVec_Get_Lane(world_server->players[id].self.origin.z, 0));
}
void request_yaw(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].self.ship.orientation.forward = rot_vec3d(amt_sin, amt_cos, world_server->players[id].self.ship.orientation.up, world_server->players[id].self.ship.orientation.forward);
  world_server->players[id].self.ship.orientation.right   = rot_vec3d(amt_sin, amt_cos, world_server->players[id].self.ship.orientation.up, world_server->players[id].self.ship.orientation.right);
  MTX_UNLOCK(world_server->player_mtxs + id);
}
void request_pitch(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].self.ship.orientation.forward = rot_vec3d(amt_sin, amt_cos, world_server->players[id].self.ship.orientation.right, world_server->players[id].self.ship.orientation.forward);
  world_server->players[id].self.ship.orientation.up      = rot_vec3d(amt_sin, amt_cos, world_server->players[id].self.ship.orientation.right, world_server->players[id].self.ship.orientation.up);
  MTX_UNLOCK(world_server->player_mtxs + id);
}
void request_roll(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].self.ship.orientation.up = rot_vec3d(amt_sin, amt_cos, world_server->players[id].self.ship.orientation.forward, world_server->players[id].self.ship.orientation.up);
  world_server->players[id].self.ship.orientation.right   = rot_vec3d(amt_sin, amt_cos, world_server->players[id].self.ship.orientation.forward, world_server->players[id].self.ship.orientation.right);
  MTX_UNLOCK(world_server->player_mtxs + id);
}
