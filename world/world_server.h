#ifndef WORLD_SERVER_H
#define WORLD_SERVER_H

#include "../math/vector_3d.h"
#include "../math/fast_list.h"
#include "../utils/semaphore.h"
#include "chunk.h"

#define MAX_CLIENTS 256

#define CUBE_NUM 27

#define MAX_OBJECTS 100
#define MAX_LIGHTS  5

#define CHUNK_POW 10
#define CHUNK_SIZE (1 << CHUNK_POW)
#define NOISE_DOMAIN_SIZE CHUNK_SIZE << 2
extern SGVec3D_t chunk_offsets[CUBE_NUM];

#include "../objects/object.h"

typedef
struct chunk_s {
  pthread_rwlock_t rwlock;

  unsigned int num_objects;
  object_t * objects;

  unsigned int num_lights;
  object_t ** lights;
}
chunk_t;

typedef
struct world_snapshot_s {
  // unsigned long time;
  chunk_t * chunks[2 * CUBE_NUM];
  chunk_t ship_chunks[CUBE_NUM];
  object_t * self;
}
world_snapshot_t;

#include "world_db.h"

typedef
struct world_server_s {
  // unsigned long time;
  // pthread_mutex_t time_mtx;

  FAST_LIST_T(unsigned int, MAX_CLIENTS) active_ids;
  pthread_mutex_t active_ids_mtx;

  pthread_mutex_t player_mtxs[MAX_CLIENTS];
  object_t players[MAX_CLIENTS];

  world_db_t world_db;
}
world_server_t;

void start_world_server();
void end_world_server();

void request_snapshot(world_snapshot_t *, unsigned int id);
void destroy_snapshot(world_snapshot_t *);

void request_player(unsigned int id);
void request_player_end(unsigned int id);

extern world_server_t * world_server;
extern SGVec3D_t cube_offsets[CUBE_NUM * 2];

inline
SGVec3D_t get_cube_offset(unsigned int cube_idx) {
  return cube_offsets[cube_idx];
}

#endif /* end of include guard: WORLD_SERVER_H */
