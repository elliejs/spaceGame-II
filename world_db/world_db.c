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
  world_db->active_ids.num = 0;
  world_db->chunk_cache.num = 0;
}

#define CHUNK_ORIGIN_ID (chunk_id_t) { \
  .x = 0, \
  .y = 0, \
  .z = 0  \
}

void request_player(unsigned int id) {
  printf("requesting player\n");
  MTX_LOCK(&(world_db->player_mtxs[id]));
  world_db->players[id].self = create_ship((SGVec3D_t) {
    .x = SGVec_Load_Const(20.),
    .y = SGVec_Load_Const(20.),
    .z = SGVec_Load_Const(20.)
  });
  world_db->players[id].chunk_id = CHUNK_ORIGIN_ID;
  MTX_UNLOCK(&(world_db->player_mtxs[id]));

  MTX_LOCK(&(world_db->active_ids_mtx));
  PUSH_FAST_LIST(world_db->active_ids, id);
  MTX_UNLOCK(&(world_db->active_ids_mtx));

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
//
// bool chunk_ids_contains(unsigned int * container, unsigned int x) {
//   for (int i = 0; i < CUBE_NUM; i++) {
//     if (container[i] == x) return true;
//   }
//   return false;
// }

bool in_neighborhood(chunk_id_t a, chunk_id_t b) {
  return
  (-1 <= (a.x - b.x) && (a.x - b.x) <= 1)
  &&
  (-1 <= (a.y - b.y) && (a.y - b.y) <= 1)
  &&
  (-1 <= (a.z - b.z) && (a.z - b.z) <= 1)
  ;
}

world_snapshot_t request_snapshot(unsigned int id) {
  world_snapshot_t snapshot;
  snapshot.chunks[CUBE_NUM] = &(snapshot.ship_chunk);
  // snapshot.self = &(world_db->players[id].self);
  snapshot.ship_chunk.lights = malloc(0);
  snapshot.ship_chunk.num_lights = 0;

  // unsigned int accept_chunk_ids[CUBE_NUM];
  MTX_LOCK(world_db->player_mtxs + id);
  chunk_id_t current_chunk_id = world_db->players[id].chunk_id;
  // gather_acceptable_chunks(world_db->players[id].chunk_id, accept_chunk_ids);
  MTX_UNLOCK(world_db->player_mtxs + id);

  MTX_LOCK(&(world_db->active_ids_mtx));
  snapshot.ship_chunk.objects = malloc(world_db->active_ids.num * sizeof(object_t));
  snapshot.ship_chunk.num_objects = 0;

  for (int i = 0; i < world_db->active_ids.num; i++) {
    unsigned int iter_id = world_db->active_ids.data[i];
    MTX_LOCK(world_db->player_mtxs + iter_id);
    if (in_neighborhood(current_chunk_id, world_db->players[iter_id].chunk_id)) {
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
    .x =
      SGVec_Add_Mult_SGVec(
        SGVec_Add_SGVec(
          world_db->players[id].self.origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Load_Const(amt),
        world_db->players[id].self.ship.orientation.forward.x
      ),
    .y =
      SGVec_Add_Mult_SGVec(
        SGVec_Add_SGVec(
          world_db->players[id].self.origin.y,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Load_Const(amt),
        world_db->players[id].self.ship.orientation.forward.y
      ),
    .z =
      SGVec_Add_Mult_SGVec(
        SGVec_Add_SGVec(
          world_db->players[id].self.origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Load_Const(amt),
        world_db->players[id].self.ship.orientation.forward.z
      )
  };

  float x = SGVec_Get_Lane(world_db->players[id].self.origin.x, 0);
  float y = SGVec_Get_Lane(world_db->players[id].self.origin.y, 0);
  float z = SGVec_Get_Lane(world_db->players[id].self.origin.z, 0);

  world_db->players[id].chunk_id.x += (z >= 2 * CHUNK_SIZE) - (x < CHUNK_SIZE);
  world_db->players[id].chunk_id.y += (y >= 2 * CHUNK_SIZE) - (y < CHUNK_SIZE);
  world_db->players[id].chunk_id.z += (x >= 2 * CHUNK_SIZE) - (z < CHUNK_SIZE);

  world_db->players[id].self.origin = (SGVec3D_t) {
    .x =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_db->players[id].self.origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_db->players[id].self.origin.x,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        world_db->players[id].self.origin.x
      ),
    .y =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_db->players[id].self.origin.y,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_db->players[id].self.origin.y,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        world_db->players[id].self.origin.y
      ),
    .z =
      SGVec_Ternary(
        SGVec_Gtr_Or_Eq_Than(
          world_db->players[id].self.origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        SGVec_Sub_SGVec(
          world_db->players[id].self.origin.z,
          SGVec_Load_Const((float) CHUNK_SIZE)
        ),
        world_db->players[id].self.origin.z
      )
  };

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
