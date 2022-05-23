#include "world_server.h"

void promote(cache_item_t * item) {
  //first stitch together around item's current location
  if (item->prev) item->prev->next = item->next;
  else world_server->world_db.head = item->next;
  if (item->next) item->next->prev = item->prev;
  else world_server->world_db.tail = item->prev;

  //then reconnect item where it should go
  item->prev = NULL;
  item->next = world_server->world_db.head;

  //finally reconfigure head to be item
  world_server->world_db.head->prev = item;
  world_server->world_db.head = item;
}

chunk_t generate_chunk(unsigned int encoded_id) {
  chunk_t gen = (chunk_t) {
    .objects = malloc(2 * sizeof(object_t)),
    .num_objects = 2,
    .lights = malloc(1 * sizeof(object_t *)),
    .num_lights = 1
  };

  gen.objects[0] = create_planet(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(20.),
      .y = SGVec_Load_Const(20.),
      .z = SGVec_Load_Const(100.)
    },
    SGVec_Load_Const(50.)
  );
  gen.objects[1] = create_star(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(100),
      .y = SGVec_Load_Const(100),
      .z = SGVec_Load_Const(50.)
    },
    SGVec_Load_Const(50.)
  );

  gen.lights[0] = gen.objects + 1;

  return gen;
}

void destroy_chunk(chunk_t * chunk) {
  free(chunk->objects);
  free(chunk->lights);
}

compare_t cache_comparator(void * a, void * b) {
  cache_item_t * a_item = (cache_item_t *) a;
  cache_item_t * b_item = (cache_item_t *) b;
  return
    a_item->encoded_id > b_item->encoded_id
    ? GT
    : (a_item->encoded_id < b_item->encoded_id
      ? LT
      : EQ)
  ;
}

unsigned int encode_chunk_id(chunk_id_t id) {
  return id.x ^ id.y ^ id.z;
}

chunk_t * gather_chunk(chunk_id_t id) {
  unsigned int encoded_id = encode_chunk_id(id);
  // printf("\t[world_db]: looking for chunk_id: %u\n", encoded_id);
  cache_item_t dummy = (cache_item_t) {
    .prev = NULL,
    .next = NULL,
    .encoded_id = encoded_id
  };

  MTX_LOCK(&(world_server->world_db.db_mtx));
  aa_node_t * cache_node = find_or_insert(&(world_server->world_db.search_tree), (void *) &dummy);
  // printf("\t[world_db %u]: searching cache...\n", encoded_id);

  cache_item_t * item = (cache_item_t *) cache_node->data;
  if (!item->prev && !item->next) {
    // printf("\t[world_db %u]: NOT FOUND! Inserting\n", encoded_id);
    item = world_server->world_db.tail;

    if(item->encoded_id != encoded_id) {
      dummy.encoded_id = item->encoded_id;
      // printf("\t[world_db %u]: deleting from search tree %p\n", encoded_id, item);
      delete(&(world_server->world_db.search_tree), (void *) &dummy);
    }
    // printf("BBBBBBB\n");
    // printf("\t[world_db %u]: deleting old chunk at %p\n", encoded_id, item);
    destroy_chunk(&(item->data));
    // printf("CCCCCC\n");
    item->data = generate_chunk(encoded_id);
    // printf("\t[world_db %u]: generated data in %p\n", encoded_id, item);
    item->encoded_id = encoded_id;
    cache_node->data = (void *) item;
  }

  promote(item);
  // printf("\t[world_db %u]: promoted %p to the head %p\n", encoded_id, item, world_server->world_db.head);
  MTX_UNLOCK(&(world_server->world_db.db_mtx));

  return &(item->data);
}

void gather_chunks(chunk_t ** chunk_storage, chunk_id_t chunk_id) {
  int x = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      for (int k = -1; k <= 1; k++) {
        chunk_storage[x++] = gather_chunk((chunk_id_t) {
          .x = chunk_id.x + (unsigned int) k,
          .y = chunk_id.y + (unsigned int) j,
          .z = chunk_id.z + (unsigned int) i
        });
      }
    }
  }
}

void start_world_db(world_db_t * world_db) {
  MTX_INIT(&(world_db->db_mtx));

  world_db->head = world_db->backing_data;
  world_db->backing_data[0].prev = NULL;
  world_db->backing_data[0].data = DEFAULT_CHUNK;
  world_db->backing_data[0].next = world_db->backing_data + 1;
  for(int i = 1; i < CACHE_LEN - 1; i++) {
    world_db->backing_data[i].prev = world_db->backing_data + i - 1;
    world_db->backing_data[i].data = DEFAULT_CHUNK;
    world_db->backing_data[i].next = world_db->backing_data + i + 1;
  }
  world_db->backing_data[CACHE_LEN - 1].prev = world_db->backing_data + CACHE_LEN - 2;
  world_db->backing_data[CACHE_LEN - 1].data = DEFAULT_CHUNK;
  world_db->backing_data[CACHE_LEN - 1].next = NULL;
  world_db->tail = world_db->backing_data + CACHE_LEN - 1;

  world_db->search_tree = create_tree(cache_comparator);
}

void end_world_db(world_db_t * world_db) {

}
