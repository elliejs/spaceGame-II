#ifndef WORLD_SERVER_H
#include "world_server.h"
#endif

#ifndef WORLD_DB_H
#define WORLD_DB_H

#include "../math/aa_tree.h"

#define CACHE_LEN (MAX_CLIENTS * CUBE_NUM)

typedef
struct cache_item_s {
  struct cache_item_s * prev;
  chunk_t chunk;
  object_t objects[MAX_OBJECTS + MAX_LIGHTS];
  object_t * lights[MAX_LIGHTS];
  unsigned int encoded_id;
  struct cache_item_s * next;

  aa_node_t search_node;

  bool instantiated;
}
cache_item_t;

typedef
struct world_db_s {
  cache_item_t backing_data[CACHE_LEN];
  cache_item_t * head;
  cache_item_t * tail;
  aa_tree_t search_tree;

  pthread_mutex_t db_mtx;
}
world_db_t;

#include "world_server.h"

void gather_chunks(chunk_t ** chunk_storage, chunk_id_t chunk_id);
void start_world_db(world_db_t * world_db);
void end_world_db(world_db_t * world_db);
unsigned int encode_chunk_id(chunk_id_t id);

#endif /* end of include guard: WORLD_DB_H */
