#ifndef WORLD_SERVER_H
#include "world_server.h"
#endif

#ifndef WORLD_DB_H
#define WORLD_DB_H

#include "../math/aa_tree.h"

#define CACHE_LEN (MAX_CLIENTS * CUBE_NUM)

typedef
struct cache_item_s {
  //db_mtx protected
  struct cache_item_s * prev;
  struct cache_item_s * next;
  aa_node_t search_node;
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
  aa_tree_t search_tree;

  //db_cache_rwlock protected
  cache_item_t backing_data[CACHE_LEN];
}
world_db_t;

#include "world_server.h"

void gather_chunks(chunk_t ** chunk_storage, chunk_coord_t abs_coord);
void start_world_db(world_db_t * world_db);
void end_world_db(world_db_t * world_db);
unsigned int encode_chunk_coord(chunk_coord_t abs_coord);

#endif /* end of include guard: WORLD_DB_H */
