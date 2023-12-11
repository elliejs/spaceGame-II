#ifndef WORLD_SERVER_H
#define WORLD_SERVER_H

#include <sys/time.h>

#include "../math/vector_3d.h"
#include "../math/fast_list.h"
#include "../utils/semaphore.h"
#include "chunk.h"
#include "world_db.h"

#define CHUNK_POW 10
#define CHUNK_SIZE (1 << CHUNK_POW)
#define NOISE_DOMAIN_SIZE CHUNK_SIZE << 2

#include "../objects/object.h"

#define NUM_CHUNKS 2 * CUBE_NUM

typedef
struct world_snapshot_s {
  long long time;
  chunk_t * chunks[NUM_CHUNKS];
  chunk_t ship_chunks[CUBE_NUM];
  object_t * self;
}
world_snapshot_t;


typedef
struct world_server_s {
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

void request_player(unsigned int id, off_t user_offset);
void request_player_save(unsigned int id, off_t user_index);
void request_player_end(unsigned int id);

extern world_server_t * world_server;
extern SGVec3D_t cube_offsets[NUM_CHUNKS];

inline
SGVec3D_t get_cube_offset(unsigned int cube_idx) {
  return cube_offsets[cube_idx];
}

inline
long long time_millis(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

#endif /* end of include guard: WORLD_SERVER_H */
