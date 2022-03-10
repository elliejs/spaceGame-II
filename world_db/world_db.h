#ifndef WORLD_DB_H
#define WORLD_DB_H

#include "../math/vector_3d.h"
#include "../utils/semaphore.h"

#include "../objects/object.h"

#define MAX_CLIENTS 256

#define CUBE_NUM 27

#define CHUNK_POW 6
#define CHUNK_SIZE (1 << CHUNK_POW)

typedef
struct fast_list_s {
  unsigned int num;
  unsigned int data[MAX_CLIENTS];
}
fast_list_t;

typedef
struct chunk_s {
  unsigned int num_objects;
  object_t * objects;

  unsigned int num_lights;
  object_t ** lights;
}
chunk_t;

typedef
struct world_snapshot_s {
  // unsigned long time;
  chunk_t * chunks[CUBE_NUM];

  unsigned int num_ships;
  object_t ships[MAX_CLIENTS];
  object_t * self;
}
world_snapshot_t;

typedef
struct world_db_s {
  // unsigned long time;
  // pthread_mutex_t time_mtx;

  fast_list_t active_ids;
  pthread_mutex_t active_ids_mtx;

  pthread_mutex_t player_mtxs[MAX_CLIENTS];
  struct {
    object_t self;
    unsigned int chunk_id;
    SGVec3D_t chunk_origin;
  } players[MAX_CLIENTS];
}
world_db_t;

void start_world_db();
void end_world_db();

world_snapshot_t request_snapshot(unsigned int id);

void request_player(unsigned int id);
void request_player_end(unsigned int id);

#endif /* end of include guard: WORLD_DB_H */
