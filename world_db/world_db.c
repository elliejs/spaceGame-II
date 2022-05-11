#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "world_db.h"

static
world_db_t * world_db = NULL;

void start_world_db() {
  if (world_db != NULL) {
    printf("WORLD_DB: Already instantiated, not double-mallocing\n");
    return;
  }

  world_db = (world_db_t *) mmap(NULL, sizeof(world_db_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  MTX_INIT(&(world_db->active_ids_mtx));
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_INIT(world_db->player_mtxs + i);
}

// void point_to_chunk_id(float3D_t point, unsigned int * id, float3D_t * origin) {
//   unsigned int point_origin_x = (int) roundf(point.x) >> CHUNK_POW;
//   origin->x = (float) point_origin_x;
//   unsigned int point_origin_y = (int) roundf(point.y) >> CHUNK_POW;
//   origin->y = (float) point_origin_y;
//   unsigned int point_origin_z = (int) roundf(point.z) >> CHUNK_POW;
//   origin->z = (float) point_origin_y;
//
//   *id = (((point_origin_z << CHUNK_POW) | point_origin_y) << CHUNK_POW) | point_origin_x;
// }

unsigned int calc_chunk_id(SGVec3D_t point) {
  return
    (((int) SGVec_Get_Lane(point.z, 0) & (CHUNK_SIZE - 1)) << (CHUNK_POW * 2))
    |
    (((int) SGVec_Get_Lane(point.y, 0) & (CHUNK_SIZE - 1)) << CHUNK_POW)
    |
    (((int) SGVec_Get_Lane(point.x, 0) & (CHUNK_SIZE - 1)))
    ;
}


void request_player(unsigned int id) {
  printf("requesting player\n");
  MTX_LOCK(&(world_db->active_ids_mtx));
  PUSH_FAST_LIST(world_db->active_ids, id);
  MTX_UNLOCK(&(world_db->active_ids_mtx));

  MTX_LOCK(&(world_db->player_mtxs[id]));
  world_db->players[id].self = create_ship((SGVec3D_t) {
    .x = SGVec_Load_Const(20.),
    .y = SGVec_Load_Const(20.),
    .z = SGVec_Load_Const(20.)
  });
  world_db->players[id].chunk_id = calc_chunk_id(world_db->players[id].self.origin);
  // point_to_chunk_id(world_db->players[id].self.float_origin, &(world_db->players[id].chunk_id), &(world_db->players[id].chunk_origin));
  MTX_UNLOCK(&(world_db->player_mtxs[id]));
  printf("player created\n");
}

void request_player_end(unsigned int id) {
  MTX_LOCK(&(world_db->active_ids_mtx));
  POP_FAST_LIST(world_db->active_ids, id);
  MTX_UNLOCK(&(world_db->active_ids_mtx));
}

void end_world_db() {
  MTX_DESTROY(&(world_db->active_ids_mtx));
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_DESTROY(world_db->player_mtxs + i);
  munmap(world_db, sizeof(world_db_t));
}

static
void gather_acceptable_chunks(unsigned int center_id, unsigned int * chunk_id_storage) {
  int idx = 0;
  for(int i = -(CHUNK_SIZE * CHUNK_SIZE); i <= CHUNK_SIZE * CHUNK_SIZE; i += CHUNK_SIZE * CHUNK_SIZE) {
    for(int j = -CHUNK_SIZE; j <= CHUNK_SIZE; j += CHUNK_SIZE) {
      for(int k = -1; k <= 1; k += 1) {
        chunk_id_storage[idx++] = (center_id + i + j + k + (1 << (CHUNK_POW * 3))) % (1 << (CHUNK_POW * 3));
      }
    }
  }
}

bool chunk_ids_contains(unsigned int * container, unsigned int x) {
  for (int i = 0; i < CUBE_NUM; i++) {
    if (container[i] == x) return true;
  }
  return false;
}

world_snapshot_t request_snapshot(unsigned int id) {
  world_snapshot_t snapshot;
  snapshot.chunks[CUBE_NUM] = &(snapshot.ship_chunk);
  // snapshot.self = &(world_db->players[id].self);
  snapshot.ship_chunk.lights = malloc(0);
  snapshot.ship_chunk.num_lights = 0;

  unsigned int accept_chunk_ids[CUBE_NUM];
  MTX_LOCK(world_db->player_mtxs + id);
  gather_acceptable_chunks(world_db->players[id].chunk_id, accept_chunk_ids);
  MTX_UNLOCK(world_db->player_mtxs + id);

  MTX_LOCK(&(world_db->active_ids_mtx));
  snapshot.ship_chunk.objects = malloc(world_db->active_ids.num * sizeof(object_t));
  snapshot.ship_chunk.num_objects = 0;

  for (int i = 0; i < world_db->active_ids.num; i++) {
    unsigned int iter_id = world_db->active_ids.data[i];
    MTX_LOCK(world_db->player_mtxs + iter_id);
    if (chunk_ids_contains(accept_chunk_ids, world_db->players[iter_id].chunk_id)) {
      if (iter_id == id) {
        snapshot.self = snapshot.ship_chunk.objects + snapshot.ship_chunk.num_objects;
      }
      snapshot.ship_chunk.objects[snapshot.ship_chunk.num_objects++] = world_db->players[iter_id].self;
    }
    MTX_UNLOCK(world_db->player_mtxs + iter_id);
  }
  MTX_UNLOCK(&(world_db->active_ids_mtx));
  //REMOVE LATER TESTING ONLY
  for (int i = 1; i < CUBE_NUM; i++) {
    snapshot.chunks[i] = malloc(1 * sizeof(chunk_t));
    *(snapshot.chunks[i]) = (chunk_t) {
      .num_objects = 0,
      .objects = malloc(0 * sizeof(object_t)),
      .num_lights = 0,
      .lights = malloc(0 * sizeof(object_t *))
    };
  }

  snapshot.chunks[0] = malloc(1 * sizeof(chunk_t));
  *(snapshot.chunks[0]) = (chunk_t) {
    .num_objects = 2,
    .objects = malloc(2 * sizeof(object_t)),

    .num_lights = 1,
    .lights = malloc(1 * sizeof(object_t *))
  };

  snapshot.chunks[0]->objects[0] = create_planet((SGVec3D_t) {
    .x = SGVec_Load_Const(20.),
    .y = SGVec_Load_Const(20.),
    .z = SGVec_Load_Const(100.)
  }, SGVec_Load_Const(50.));
  snapshot.chunks[0]->objects[1] = create_star((SGVec3D_t) {
    .x = SGVec_Load_Const(100),
    .y = SGVec_Load_Const(100),
    .z = SGVec_Load_Const(50.),
  }, SGVec_Load_Const(50.));
  snapshot.chunks[0]->lights[0] = snapshot.chunks[0]->objects + 1;

  return snapshot;
}

void request_thrust(unsigned int id, float amt) {
  MTX_LOCK(world_db->player_mtxs + id);
  world_db->players[id].self.origin = (SGVec3D_t) {
    .x = SGVec_Add_Mult_SGVec(world_db->players[id].self.origin.x, SGVec_Load_Const(amt), world_db->players[id].self.ship.orientation.forward.x),
    .y = SGVec_Add_Mult_SGVec(world_db->players[id].self.origin.y, SGVec_Load_Const(amt), world_db->players[id].self.ship.orientation.forward.y),
    .z = SGVec_Add_Mult_SGVec(world_db->players[id].self.origin.z, SGVec_Load_Const(amt), world_db->players[id].self.ship.orientation.forward.z)
  };

  float z = SGVec_Get_Lane(world_db->players[id].self.origin.z, 0);
  float y = SGVec_Get_Lane(world_db->players[id].self.origin.y, 0);
  float x = SGVec_Get_Lane(world_db->players[id].self.origin.x, 0);

  world_db->players[id].chunk_id += ((z >= CHUNK_SIZE) - (z < 0.)) * (1 << CHUNK_POW * 2);
  world_db->players[id].chunk_id += ((y >= CHUNK_SIZE) - (y < 0.)) * (1 << CHUNK_POW);
  world_db->players[id].chunk_id += ((x >= CHUNK_SIZE) - (x < 0.)) * (1);
  // world_db->players[id].self.float_origin = (float3D_t) {
  //   .x = world_db->players[id].self.float_origin.x + (amt * SGVec_Get_Lane(world_db->players[id].self.ship.orientation.forward.x, 0)),
  //   .y = world_db->players[id].self.float_origin.y + (amt * SGVec_Get_Lane(world_db->players[id].self.ship.orientation.forward.y, 0)),
  //   .z = world_db->players[id].self.float_origin.z + (amt * SGVec_Get_Lane(world_db->players[id].self.ship.orientation.forward.z, 0))
  // };
  // point_to_chunk_id(world_db->players[id].self.float_origin, &(world_db->players[id].chunk_id), &(world_db->players[id].chunk_origin));
  MTX_UNLOCK(world_db->player_mtxs + id);
}
void request_yaw(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_db->player_mtxs + id);
  world_db->players[id].self.ship.orientation.forward = rot_vec3d(amt_sin, amt_cos, world_db->players[id].self.ship.orientation.up, world_db->players[id].self.ship.orientation.forward);
  world_db->players[id].self.ship.orientation.right   = rot_vec3d(amt_sin, amt_cos, world_db->players[id].self.ship.orientation.up, world_db->players[id].self.ship.orientation.right);
  MTX_UNLOCK(world_db->player_mtxs + id);
}
void request_pitch(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_db->player_mtxs + id);
  world_db->players[id].self.ship.orientation.forward = rot_vec3d(amt_sin, amt_cos, world_db->players[id].self.ship.orientation.right, world_db->players[id].self.ship.orientation.forward);
  world_db->players[id].self.ship.orientation.up      = rot_vec3d(amt_sin, amt_cos, world_db->players[id].self.ship.orientation.right, world_db->players[id].self.ship.orientation.up);
  MTX_UNLOCK(world_db->player_mtxs + id);
}
void request_roll(unsigned int id, float amt) {
  SGVec amt_cos = SGVec_Load_Const(cosf(amt));
  SGVec amt_sin = SGVec_Load_Const(sinf(amt));
  MTX_LOCK(world_db->player_mtxs + id);
  world_db->players[id].self.ship.orientation.up = rot_vec3d(amt_sin, amt_cos, world_db->players[id].self.ship.orientation.forward, world_db->players[id].self.ship.orientation.up);
  world_db->players[id].self.ship.orientation.right   = rot_vec3d(amt_sin, amt_cos, world_db->players[id].self.ship.orientation.forward, world_db->players[id].self.ship.orientation.right);
  MTX_UNLOCK(world_db->player_mtxs + id);
}
