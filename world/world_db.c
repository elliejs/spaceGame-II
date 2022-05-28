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

void generate_chunk(unsigned int encoded_id, cache_item_t * item) {


  item->chunk.num_objects = 2;
  item->chunk.num_lights = 1;

  item->objects[0] = create_planet(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(20.),
      .y = SGVec_Load_Const(20.),
      .z = SGVec_Load_Const(100.)
    },
    SGVec_Load_Const(50.)
  );
  item->objects[1] = create_star(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(100),
      .y = SGVec_Load_Const(100),
      .z = SGVec_Load_Const(50.)
    },
    SGVec_Load_Const(50.)
  );

  item->lights[0] = item->objects + 1;
}

void destroy_chunk(chunk_t * chunk) {
  chunk->num_objects = 0;
  chunk->num_lights = 0;
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
  cache_item_t dummy = (cache_item_t) {
    .prev = NULL,
    .next = NULL,
    .encoded_id = encoded_id
  };
  MTX_LOCK(&(world_server->world_db.db_mtx));
  aa_node_t * cache_node;
  cache_item_t * item = world_server->world_db.tail;

  if (find(&(world_server->world_db.search_tree), (void *) &dummy, &cache_node)) {
    item = (cache_item_t *) cache_node->data;
  } else {
    if (item->instantiated) {
      delete(&(world_server->world_db.search_tree), (void *) item);
      destroy_chunk(&(item->chunk));
    } else {
      item->instantiated = true;
    }
    item->encoded_id = encoded_id;
    generate_chunk(encoded_id, item);
    insert(&(world_server->world_db.search_tree), (void *) item, &(item->search_node));
  }

  promote(item);







  // // printf("\t[world_db %u]: searching cache...\n", encoded_id);
  // printf("%d\n", __LINE__);
  // cache_item_t * item = (cache_item_t *) cache_node->data;
  // if (!item->prev && !item->next) {
  //   // printf("\t[world_db %u]: NOT FOUND! Inserting\n", encoded_id);
  //   item = world_server->world_db.tail;
  //   if (item->instantiated) {
  //     // if(item->encoded_id != encoded_id) {
  //     dummy.encoded_id = item->encoded_id;
  //     // printf("\t[world_db %u]: deleting from search tree %p\n", encoded_id, item);
  //     delete(&(world_server->world_db.search_tree), (void *) &dummy);
  //     // }
  //     // printf("BBBBBBB\n");
  //     // printf("\t[world_db %u]: deleting old chunk at %p\n", encoded_id, item);
  //     destroy_chunk(&(item->data));
  //   } else {
  //     item->instantiated = true;
  //   }
  //   // printf("CCCCCC\n");
  //   item->data = generate_chunk(encoded_id);
  //   // printf("\t[world_db %u]: generated data in %p\n", encoded_id, item);
  //   item->encoded_id = encoded_id;
  //   cache_node->data = (void *) item;
  // }
  //
  // promote(item);
  // // printf("\t[world_db %u]: promoted %p to the head %p\n", encoded_id, item, world_server->world_db.head);
  MTX_UNLOCK(&(world_server->world_db.db_mtx));
  return &(item->chunk);
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
  world_db->backing_data[0] = (cache_item_t) {
    .prev = NULL,
    .chunk = (chunk_t) {
      .objects = world_db->backing_data[0].objects,
      .lights = world_db->backing_data[0].lights,
      .num_objects = 0,
      .num_lights = 0
    },
    .next = world_db->backing_data + 1,

    .instantiated = false
  };

  for(int i = 1; i < CACHE_LEN - 1; i++) {
    world_db->backing_data[i] = (cache_item_t) {
      .prev = world_db->backing_data + i - 1,
      .chunk = (chunk_t) {
        .objects = world_db->backing_data[i].objects,
        .lights = world_db->backing_data[i].lights,
        .num_objects = 0,
        .num_lights = 0
      },
      .next = world_db->backing_data + i + 1,

      .instantiated = false
    };
  }

  world_db->backing_data[CACHE_LEN - 1] = (cache_item_t) {
    .prev = world_db->backing_data + CACHE_LEN - 2,
    .chunk = (chunk_t) {
      .objects = world_db->backing_data[CACHE_LEN - 1].objects,
      .lights = world_db->backing_data[CACHE_LEN - 1].lights,
      .num_objects = 0,
      .num_lights = 0
    },
    .next = NULL,

    .instantiated = false
  };
  world_db->tail = world_db->backing_data + CACHE_LEN - 1;

  world_db->search_tree = (aa_tree_t) {
    .comparator = cache_comparator,
    .nil = (aa_node_t) {
      .left = &(world_db->search_tree.nil),
      .right = &(world_db->search_tree.nil),
      .data = NULL,
      .level = 0
    },
    .root = &(world_db->search_tree.nil)
  };

}

void end_world_db(world_db_t * world_db) {

}
