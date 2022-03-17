#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "world_db.h"

void push_fast_list(fast_list_t * fast_list, unsigned int data) {
  fast_list->data[fast_list->num++] = data;
}

void pop_fast_list(fast_list_t * fast_list, unsigned int data) {
  for (int i = 0; i < fast_list->num; i++) {
    if (fast_list->data[i] == data) {
      fast_list->data[i] = fast_list->data[--fast_list->num];
      return;
    }
  }
}

static
world_db_t * world_db = NULL;

void start_world_db() {
  if (world_db != NULL) {
    printf("WORLD_DB: Already instantiated, not double-mallocing\n");
    return;
  }
  world_db = (world_db_t *) mmap(NULL, sizeof(world_db_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  MTX_INIT(world_db->active_ids_mtx);
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_INIT(world_db->player_mtxs[i]);
}

void point_to_chunk_id(float3D_t point, unsigned int * id, float3D_t * origin) {
  unsigned int point_origin_x = (int) roundf(point.x) >> CHUNK_POW;
  origin->x = (float) point_origin_x;
  unsigned int point_origin_y = (int) roundf(point.y) >> CHUNK_POW;
  origin->y = (float) point_origin_y;
  unsigned int point_origin_z = (int) roundf(point.z) >> CHUNK_POW;
  origin->z = (float) point_origin_y;

  *id = (((point_origin_z << CHUNK_POW) | point_origin_y) << CHUNK_POW) | point_origin_x;
}

void request_player(unsigned int id) {
  MTX_LOCK(world_db->active_ids_mtx);
  push_fast_list(&(world_db->active_ids), id);
  MTX_UNLOCK(world_db->active_ids_mtx);

  MTX_LOCK(world_db->player_mtxs[id]);
  world_db->players[id].self = create_ship((float3D_t) {
    .x = 10.5,
    .y = 10.5,
    .z = 10.5
  });
  point_to_chunk_id(world_db->players[id].self.float_origin, &(world_db->players[id].chunk_id), &(world_db->players[id].chunk_origin));
  MTX_UNLOCK(world_db->player_mtxs[id]);
}

void request_player_end(unsigned int id) {
  MTX_LOCK(world_db->active_ids_mtx);
  pop_fast_list(&(world_db->active_ids), id);
  MTX_UNLOCK(world_db->active_ids_mtx);
}

void end_world_db() {
  MTX_DESTROY(world_db->active_ids_mtx);
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_DESTROY(world_db->player_mtxs[i]);
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
  world_snapshot_t snapshot = (world_snapshot_t) {
    .num_ships = 0
  };

  unsigned int accept_chunk_ids[CUBE_NUM];

  MTX_LOCK(world_db->player_mtxs[id]);
  snapshot.ships[snapshot.num_ships++] = world_db->players[id].self;
  snapshot.self = snapshot.ships;
  gather_acceptable_chunks(world_db->players[id].chunk_id, accept_chunk_ids);
  MTX_UNLOCK(world_db->player_mtxs[id]);

  MTX_LOCK(world_db->active_ids_mtx);
  for (int i = 0; i < world_db->active_ids.num; i++) {
    unsigned int iter_id = world_db->active_ids.data[i];
    if (iter_id == id) continue;

    MTX_LOCK(world_db->player_mtxs[iter_id]);
    if (chunk_ids_contains(accept_chunk_ids, world_db->players[iter_id].chunk_id))
      snapshot.ships[snapshot.num_ships++] = world_db->players[iter_id].self;
    MTX_UNLOCK(world_db->player_mtxs[iter_id]);

  }
  MTX_UNLOCK(world_db->active_ids_mtx);

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

  snapshot.chunks[0]->objects[0] = create_planet((float3D_t) {
    .x = 0.,
    .y = 0.,
    .z = 0.
  });
  snapshot.chunks[0]->objects[1] = create_star((float3D_t) {
    .x = 0.5,
    .y = 0.5,
    .z = -1005
  });
  snapshot.chunks[0]->lights[0] = snapshot.chunks[0]->objects + 1;

  return snapshot;
}

// void request_thrust(unsigned int id) {
//   MTX_LOCK(world_db->player_mtxs[id]);
//   world_db->players[id].self ...
//   point_to_chunk_id(world_db->players[id].self->origin, &world_db->players[id].chunk_id, &world_db->players[id].chunk_origin);
//   MTX_UNLOCK(world_db->player_mtxs[id]);
// }
// void request_yaw(unsigned int id) {
//   MTX_LOCK(world_db->player_mtxs[id]);
//   world_db->players[id].self ...
//   MTX_UNLOCK(world_db->player_mtxs[id]);
// }
// void request_pitch(unsigned int id) {
//   MTX_LOCK(world_db->player_mtxs[id]);
//   world_db->players[id].self ...
//   MTX_UNLOCK(world_db->player_mtxs[id]);
// }
// void request_roll(unsigned int id) {
//   MTX_LOCK(world_db->player_mtxs[id]);
//   world_db->players[id].self ...
//   MTX_UNLOCK(world_db->player_mtxs[id]);
// }
