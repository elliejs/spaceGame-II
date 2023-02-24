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
  struct cache_item_s * prev;
  struct cache_item_s * next;
  off_t search_node;
  unsigned int encoded_id;
  bool instantiated;

  //db_cache_rwlock protected
  pthread_rwlock_t db_cache_item_rwlock;
  chunk_t chunk;
  object_t objects[MAX_OBJECTS + MAX_LIGHTS];
  object_t * lights[MAX_LIGHTS];
}
cache_item_t;

typedef
struct world_db_s {
  pthread_mutex_t db_mtx;

  //db_mtx protected
  cache_item_t * head;
  cache_item_t * tail;
//   aa_tree_t search_tree;

  //db_cache_rwlock protected
  cache_item_t backing_data[CACHE_LEN];
  aa_node_t search_nodes[CACHE_LEN + 1];
  off_t search_root;
}
world_db_t;


void gather_chunks(chunk_t ** chunk_storage, chunk_coord_t abs_coord);
void start_world_db(world_db_t * world_db);
void end_world_db(world_db_t * world_db);
unsigned int encode_chunk_coord(chunk_coord_t abs_coord);

#endif /* end of include guard: WORLD_DB_H */
