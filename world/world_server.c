#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "world_server.h"
#include "../users/user_db.h"

world_server_t * world_server = NULL;
SGVec3D_t cube_offsets[CUBE_NUM * 2];

extern inline
SGVec3D_t get_cube_offset(unsigned int cube_idx);

extern inline
long long time_millis(void);

void start_world_server() {
  if (world_server != NULL) {
    printf("[world_server]: Already instantiated, not double-mallocing\n");
    return;
  }

  int i = 0;
  for(int z = -1; z <= 1; z++) {
    for(int y = -1; y <= 1; y++) {
      for(int x = -1; x <= 1; x++) {
        cube_offsets[i + CUBE_NUM] = cube_offsets[i] = (SGVec3D_t) {
          .x = SGVec_Load_Const(x * CHUNK_SIZE),
          .y = SGVec_Load_Const(y * CHUNK_SIZE),
          .z = SGVec_Load_Const(z * CHUNK_SIZE)
        };
        i++;
      }
    }
  }

  world_server = (world_server_t *) mmap(NULL, sizeof(world_server_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  start_world_db(&(world_server->world_db));
  MTX_INIT(&(world_server->active_ids_mtx));
  for (int i = 0; i < MAX_CLIENTS; i++) MTX_INIT(world_server->player_mtxs + i);
  world_server->active_ids.num = 0;
}

void request_player(unsigned int id, off_t user_offset) {
  MTX_LOCK(world_server->player_mtxs + id);
  user_data_t user_data = get_user_data(user_offset);
  load_ship(world_server->players + id, &user_data);
  MTX_UNLOCK(world_server->player_mtxs + id);

  MTX_LOCK(&(world_server->active_ids_mtx));
  PUSH_FAST_LIST(world_server->active_ids, id);
  MTX_UNLOCK(&(world_server->active_ids_mtx));
}

void request_player_save(unsigned int id, off_t user_index) {
  user_data_t user_data;
  MTX_LOCK(world_server->player_mtxs + id);
    user_data = export_ship(world_server->players + id);
  MTX_UNLOCK(world_server->player_mtxs + id);
  update_user_data(user_index, &user_data);
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

bool in_neighborhood(chunk_coord_t a) {
  return
  (0 <= (a.x + 1) && (a.x + 1) <= 2)
  &&
  (0 <= (a.y + 1) && (a.y + 1) <= 2)
  &&
  (0 <= (a.z + 1) && (a.z + 1) <= 2)
  ;
}

void destroy_snapshot(world_snapshot_t * snapshot) {
  for(int i = 0; i < CUBE_NUM; i++) {
    RWLOCK_RUNLOCK(&(snapshot->chunks[i]->rwlock));
    free(snapshot->ship_chunks[i].objects);
    free(snapshot->ship_chunks[i].lights);
  }
}

void request_snapshot(world_snapshot_t * snapshot, unsigned int id) {
  MTX_LOCK(world_server->player_mtxs + id);
  chunk_coord_t self_abs_coord = world_server->players[id].ship.abs_coord;
  MTX_UNLOCK(world_server->player_mtxs + id);

  MTX_LOCK(&(world_server->active_ids_mtx));
  for(int i = 0; i < CUBE_NUM; i++) {
    snapshot->ship_chunks[i] = (chunk_t) {
      .num_objects = 0,
      .objects = malloc(world_server->active_ids.num * sizeof(object_t)),
      .num_lights = 0,
      .lights = malloc(0)
    };

    snapshot->chunks[CUBE_NUM + i] = snapshot->ship_chunks + i;
  }

  for (int i = 0; i < world_server->active_ids.num; i++) {
    unsigned int iter_id = world_server->active_ids.data[i];
    MTX_LOCK(world_server->player_mtxs + iter_id);
    chunk_coord_t ship_chunk_coord = world_server->players[iter_id].ship.abs_coord;
    chunk_coord_t rel_chunk_coord = (chunk_coord_t) {
      .x = ship_chunk_coord.x - self_abs_coord.x,
      .y = ship_chunk_coord.y - self_abs_coord.y,
      .z = ship_chunk_coord.z - self_abs_coord.z
    };
    if (in_neighborhood(rel_chunk_coord)) {
      int ship_cube_idx = CUBE_NUM / 2
      + 1 * (int) rel_chunk_coord.x
      + 3 * (int) rel_chunk_coord.y
      + 9 * (int) rel_chunk_coord.z;

      if (iter_id == id) {
        snapshot->self = snapshot->ship_chunks[ship_cube_idx].objects + snapshot->ship_chunks[ship_cube_idx].num_objects;
      }
      snapshot->ship_chunks[ship_cube_idx].objects[snapshot->ship_chunks[ship_cube_idx].num_objects++] = world_server->players[iter_id];
    }
    MTX_UNLOCK(world_server->player_mtxs + iter_id);
  }
  MTX_UNLOCK(&(world_server->active_ids_mtx));

  gather_chunks(snapshot->chunks, self_abs_coord);
  snapshot->time = time_millis();
}
