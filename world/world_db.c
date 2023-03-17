#include "world_server.h"

float rand_between(int min, int max) {
  int r = rand();
  r %= max - min;
  r += min;
  return r;
}


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

void generate_chunk(chunk_coord_t abs_coord, chunk_t * chunk) {
  chunk->num_objects = 2;
  chunk->num_lights = 1;
  unsigned int enc_chunk_id = encode_chunk_coord(abs_coord);
  srand(enc_chunk_id);
  chunk->objects[0] = create_planet(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(rand_between(0, CHUNK_SIZE)),
      .y = SGVec_Load_Const(rand_between(0, CHUNK_SIZE)),
      .z = SGVec_Load_Const(rand_between(0, CHUNK_SIZE))
    },
    SGVec_Load_Const(rand_between(10, 100)),
    SGVec_Load_Const(rand_between(5, 15)),
    (SGVecOKLAB_t) {
      .l = SGVec_Load_Const(0.2),
      .a = SGVec_Load_Const(rand_between(1, 50) / 50. - 0.5),
      .b = SGVec_Load_Const(rand_between(1, 50) / 50. - 0.5)
    },
    (SGVec3D_t) {
      .x = SGVec_Load_Const(rand_between(0, NOISE_DOMAIN_SIZE)),
      .y = SGVec_Load_Const(rand_between(0, NOISE_DOMAIN_SIZE)),
      .z = SGVec_Load_Const(rand_between(0, NOISE_DOMAIN_SIZE)),
    }
  );
  chunk->objects[1] = create_star(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(rand_between(0, CHUNK_SIZE)),
      .y = SGVec_Load_Const(rand_between(0, CHUNK_SIZE)),
      .z = SGVec_Load_Const(rand_between(0, CHUNK_SIZE))
    },
    SGVec_Load_Const(rand_between(10, 50))
  );

  chunk->lights[0] = chunk->objects + 1;
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

unsigned int encode_chunk_coord(chunk_coord_t abs_coord) {
  return abs_coord.x ^ abs_coord.y ^ abs_coord.z ^ 0xCAFEBABE;
}

chunk_t * gather_chunk(chunk_coord_t abs_coord) {
  chunk_t * ret_chunk;

  unsigned int encoded_id = encode_chunk_coord(abs_coord);
  cache_item_t dummy = (cache_item_t) {
    .encoded_id = encoded_id
  };

  MTX_LOCK(&(world_server->world_db.db_mtx));
  cache_item_t * item = world_server->world_db.tail;
  if (!aa_find(&(world_server->world_db.search_tree), world_server->world_db.search_root, (void *) &dummy, (void **) &item)) {
    if (item->instantiated) {
      aa_error_check(&(world_server->world_db.search_tree), world_server->world_db.search_root, CACHE_LEN);
      aa_delete(&(world_server->world_db.search_tree), &(world_server->world_db.search_root), (void *) item);
    } else {
      // static int numitemsused = 0;
      // numitemsused++;
      // printf("cache utilization %f%%\n", (float) numitemsused / (float) CACHE_LEN);
      item->instantiated = true;
    }
    item->encoded_id = encoded_id;
    RWLOCK_WLOCK(&(item->chunk.rwlock));
      generate_chunk(abs_coord, &(item->chunk));
    RWLOCK_WUNLOCK(&(item->chunk.rwlock));

    aa_insert(&(world_server->world_db.search_tree), &(world_server->world_db.search_root), item->off);
  }

  promote(item);
  RWLOCK_RLOCK(&(item->chunk.rwlock));
  ret_chunk = &(item->chunk);

  MTX_UNLOCK(&(world_server->world_db.db_mtx));

  return ret_chunk;
}

void gather_chunks(chunk_t ** chunk_storage, chunk_coord_t abs_coord) {
  int x = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      for (int k = -1; k <= 1; k++) {
        chunk_storage[x++] = gather_chunk((chunk_coord_t) {
          .x = abs_coord.x + (unsigned int) k,
          .y = abs_coord.y + (unsigned int) j,
          .z = abs_coord.z + (unsigned int) i
        });
      }
    }
  }
}

void start_world_db(world_db_t * world_db) {
  MTX_INIT(&(world_db->db_mtx));
  world_db->backing_data[0] = (cache_item_t) {
    .prev = NULL,
    .next = NULL,
    .instantiated = false,

    .off = 0,
    .node = (aa_node_t) {
      .left = 0,
      .right = 0,
      .level = 0,
    },
  };

  world_db->head = world_db->backing_data + 1;
  for(int i = 1; i <= CACHE_LEN; i++) {
    world_db->backing_data[i] = (cache_item_t) {
      .prev = world_db->backing_data + i - 1,
      .next = world_db->backing_data + i + 1,
      .instantiated = false,
      .off = i,
      .chunk = (chunk_t) {
        .objects = world_db->objects[i - 1],
        .lights = world_db->lights[i - 1],
        .num_objects = 0,
        .num_lights = 0
      },
    };
    RWLOCK_INIT(&(world_db->backing_data[i].chunk.rwlock));
  }
  world_db->backing_data[CACHE_LEN].next = NULL;
  world_db->tail = world_db->backing_data + CACHE_LEN;

  world_db->search_root = 0;
  world_db->search_tree = create_aa_tree(&(world_db->backing_data[0].node),
                                         sizeof(cache_item_t),
                                         &(world_db->backing_data[0]),
                                         cache_comparator);
}

void end_world_db(world_db_t * world_db) {

}
