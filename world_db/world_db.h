#ifndef WORLD_DB_H
#define WORLD_DB_H

#include "../math/vector_3d.h"
#include "../utils/semaphore.h"

#include "../objects/object.h"

#define MAX_CLIENTS 256

#define CUBE_NUM 27

#define CHUNK_POW 6
#define CHUNK_SIZE (1 << CHUNK_POW)

// typedef
// struct fast_list_s {
//   unsigned int num;
//   unsigned void * data;
// }
// fast_list_t;

#define FAST_LIST_T(T, MAX)                   \
  struct {                                    \
    unsigned int num;                         \
    T data[MAX];                              \
  }

#define PUSH_FAST_LIST(FL, X)                 \
  FL.data[FL.num++] = X

#define POP_FAST_LIST(FL, X)                  \
  for(unsigned int i = 0; i < FL.num; i++) {  \
    if(FL.data[i] == X) {                     \
      FL.data[i] = FL.data[--FL.num];         \
      break;                                  \
    }                                         \
  }

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
  chunk_t * chunks[CUBE_NUM + 1];
  chunk_t ship_chunk;
  
  object_t * self;
}
world_snapshot_t;

typedef
struct world_db_s {
  // unsigned long time;
  // pthread_mutex_t time_mtx;

  FAST_LIST_T(unsigned int, MAX_CLIENTS) active_ids;
  pthread_mutex_t active_ids_mtx;

  pthread_mutex_t player_mtxs[MAX_CLIENTS];
  struct {
    object_t self;
    unsigned int chunk_id;
    float3D_t chunk_origin;
  } players[MAX_CLIENTS];
  FAST_LIST_T(chunk_t, MAX_CLIENTS * CUBE_NUM) chunk_cache;
}
world_db_t;

void start_world_db();
void end_world_db();

world_snapshot_t request_snapshot(unsigned int id);

void request_player(unsigned int id);
void request_player_end(unsigned int id);

void point_to_chunk_id(float3D_t point, unsigned int * id, float3D_t * origin);

void request_thrust(unsigned int id, float amt);
void request_yaw(unsigned int id, float amt);
void request_pitch(unsigned int id, float amt);
void request_roll(unsigned int id, float amt);

#endif /* end of include guard: WORLD_DB_H */
