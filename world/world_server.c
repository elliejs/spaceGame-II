#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "world_server.h"

world_server_t * world_server = NULL;
SGVec3D_t chunk_offsets[CUBE_NUM];

void start_world_server() {
  if (world_server != NULL) {
    printf("[world_server]: Already instantiated, not double-mallocing\n");
    return;
  }

  int i = 0;
  for(int x = -1; x <= 1; x++) {
    for(int y = -1; y <= 1; y++) {
      for(int z = -1; z <= 1; z++) {
        chunk_offsets[i++] = (SGVec3D_t) {
          .x = SGVec_Load_Const(x * CHUNK_SIZE),
          .y = SGVec_Load_Const(y * CHUNK_SIZE),
          .z = SGVec_Load_Const(z * CHUNK_SIZE)
        };
      }
    }
  }

  world_server = (world_server_t *) mmap(NULL, sizeof(world_server_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  start_world_db(&(world_server->world_db));
  MTX_INIT(&(world_server->active_ids_mtx));
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_INIT(world_server->player_mtxs + i);
  world_server->active_ids.num = 0;
}

void request_player(unsigned int id) {
  printf("requesting player\n");
  MTX_LOCK(world_server->player_mtxs + id);
  world_server->players[id].self = create_ship((SGVec3D_t) {
    .x = SGVec_Load_Const(20.),
    .y = SGVec_Load_Const(20.),
    .z = SGVec_Load_Const(20.)
  });
  world_server->players[id].chunk_id = CHUNK_ORIGIN_ID;
  MTX_UNLOCK(world_server->player_mtxs + id);

  MTX_LOCK(&(world_server->active_ids_mtx));
  PUSH_FAST_LIST(world_server->active_ids, id);
  MTX_UNLOCK(&(world_server->active_ids_mtx));

  printf("player created\n");
}

void request_player_end(unsigned int id) {
  MTX_LOCK(&(world_server->active_ids_mtx));
  POP_FAST_LIST(world_server->active_ids, id);
  MTX_UNLOCK(&(world_server->active_ids_mtx));
}

void end_world_server() {
  MTX_DESTROY(&(world_server->active_ids_mtx));
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_DESTROY(world_server->player_mtxs + i);
  end_world_db(&(world_server->world_db));
  munmap(world_server, sizeof(world_server_t));
}

bool in_neighborhood(chunk_id_t a) {
  return
  (0 <= (a.x + 1) && (a.x + 1) <= 2)
  &&
  (0 <= (a.y + 1) && (a.y + 1) <= 2)
  &&
  (0 <= (a.z + 1) && (a.z + 1) <= 2)
  ;
}

world_snapshot_t request_snapshot(unsigned int id) {
  world_snapshot_t snapshot;

  // snapshot.chunks[CUBE_NUM] = &(snapshot.ship_chunk);
  // snapshot.self = &(world_server->players[id].self);

  // unsigned int accept_chunk_ids[CUBE_NUM];
  MTX_LOCK(world_server->player_mtxs + id);
  chunk_id_t current_chunk_id = world_server->players[id].chunk_id;
  // gather_acceptable_chunks(world_server->players[id].chunk_id, accept_chunk_ids);
  MTX_UNLOCK(world_server->player_mtxs + id);

  MTX_LOCK(&(world_server->active_ids_mtx));
  // snapshot.ship_chunk.objects = malloc(world_server->active_ids.num * sizeof(object_t));
  // snapshot.ship_chunk.num_objects = 0;
  for(int i = 0; i < CUBE_NUM; i++) {
    snapshot.ship_chunks[i] = (chunk_t) {
      .num_objects = 0,
      .objects = malloc(world_server->active_ids.num * sizeof(object_t)),
      .num_lights = 0,
      .lights = malloc(0)
    };

    snapshot.chunks[CUBE_NUM + i] = snapshot.ship_chunks + i;
  }

  for (int i = 0; i < world_server->active_ids.num; i++) {
    unsigned int iter_id = world_server->active_ids.data[i];
    MTX_LOCK(world_server->player_mtxs + iter_id);
    chunk_id_t ship_chunk_id = world_server->players[iter_id].chunk_id;
    chunk_id_t rel_chunk_id = (chunk_id_t) {
      .x = ship_chunk_id.x - current_chunk_id.x,
      .y = ship_chunk_id.y - current_chunk_id.y,
      .z = ship_chunk_id.z - current_chunk_id.z
    };
    if (in_neighborhood(rel_chunk_id)) {
      // printf("rel_chunk_idx: %d %d %d\n", (int) rel_chunk_id.x, (int) rel_chunk_id.y, (int) rel_chunk_id.z);
      int ship_chunk_idx = CUBE_NUM / 2
      + 1 * (int) rel_chunk_id.x
      + 3 * (int) rel_chunk_id.y
      + 9 * (int) rel_chunk_id.z;

      if (iter_id == id) {
        snapshot.self = snapshot.ship_chunks[ship_chunk_idx].objects + snapshot.ship_chunks[ship_chunk_idx].num_objects;
      }
      snapshot.ship_chunks[ship_chunk_idx].objects[snapshot.ship_chunks[ship_chunk_idx].num_objects++] = world_server->players[iter_id].self;
    }
    MTX_UNLOCK(world_server->player_mtxs + iter_id);
  }
  MTX_UNLOCK(&(world_server->active_ids_mtx));

  gather_chunks(snapshot.chunks, current_chunk_id);

  return snapshot;
}
