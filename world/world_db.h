#ifndef WORLD_DB_H
#define WORLD_DB_H

#include <pthread.h>

#include "../objects/object.h"

#include "../math/aa_tree.h"

#define MAX_OBJECTS 100
#define MAX_LIGHTS  5
#define CACHE_LEN (MAX_CLIENTS * CUBE_NUM)
#define CUBE_NUM 27
#define MAX_CLIENTS 256


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
struct cache_item_s {
  //db_mtx protected
  aa_node_t node;

  struct cache_item_s * prev;
  struct cache_item_s * next;
  bool instantiated;

  unsigned int encoded_id;
  chunk_t chunk;
  off_t off;
}
cache_item_t;

typedef
struct world_db_s {
  pthread_mutex_t db_mtx;

  //db_mtx protected
  cache_item_t * head;
  cache_item_t * tail;
  aa_tree_t search_tree;
  off_t search_root;

  //db_cache_rwlock protected
  cache_item_t backing_data[CACHE_LEN + 1];
  object_t objects[CACHE_LEN][MAX_OBJECTS + MAX_LIGHTS];
  object_t * lights[CACHE_LEN][MAX_LIGHTS];
}
world_db_t;


void gather_chunks(chunk_t ** chunk_storage, chunk_coord_t abs_coord);
void start_world_db(world_db_t * world_db);
void end_world_db(world_db_t * world_db);
unsigned int encode_chunk_coord(chunk_coord_t abs_coord);

#endif /* end of include guard: WORLD_DB_H */
