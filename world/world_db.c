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

void generate_chunk(chunk_coord_t abs_coord, chunk_t * chunk) {
  chunk->num_objects = 2;
  chunk->num_lights = 1;
  unsigned int enc_chunk_id = encode_chunk_coord(abs_coord);
  srand(enc_chunk_id);
  oklab_t planet_color = linear_srgb_to_oklab((rgb_t) {((rand() % 100) / 100.), ((rand() % 100) / 100.), ((rand() % 100) / 100.)});
  chunk->objects[0] = create_planet(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(CHUNK_SIZE / 2.),
      .y = SGVec_Load_Const(CHUNK_SIZE / 2.),
      .z = SGVec_Load_Const(CHUNK_SIZE / 2.)
    },
    SGVec_Load_Const(50.),
    SGVec_Load_Const(20.),
    (SGVecOKLAB_t) {
      .l = SGVec_Load_Const(0.2),
      .a = SGVec_Load_Const(planet_color.a),
      .b = SGVec_Load_Const(planet_color.b)
    },
    (SGVec3D_t) {
      .x = SGVec_Load_Const(rand() % NOISE_DOMAIN_SIZE),
      .y = SGVec_Load_Const(rand() % NOISE_DOMAIN_SIZE),
      .z = SGVec_Load_Const(rand() % NOISE_DOMAIN_SIZE)
    }
  );
  chunk->objects[1] = create_star(
    (SGVec3D_t) {
      .x = SGVec_Load_Const(CHUNK_SIZE * 0.75),
      .y = SGVec_Load_Const(CHUNK_SIZE * 0.75),
      .z = SGVec_Load_Const(CHUNK_SIZE * 0.75)
    },
    SGVec_Load_Const(50.)
  );

  chunk->lights[0] = chunk->objects + 1;
  RWLOCK_INIT(&(chunk->rwlock));
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
//   printf("gather_chunk %u %u %u START\n", abs_coord.x, abs_coord.y, abs_coord.z);
  unsigned int encoded_id = encode_chunk_coord(abs_coord);
  cache_item_t dummy = (cache_item_t) {
    .prev = NULL,
    .next = NULL,
    .encoded_id = encoded_id
  };
  aa_node_t * cache_node;
  chunk_t * ret_chunk;
  MTX_LOCK(&(world_server->world_db.db_mtx));
  cache_item_t * item = world_server->world_db.tail;
//   printf("gather_chunk %u %u %u AA_FIND START\n", abs_coord.x, abs_coord.y, abs_coord.z);
  if (aa_find(cache_comparator, world_server->world_db.search_nodes + world_server->world_db.search_root, (void *) &dummy, &cache_node)) {
//     printf("gather_chunk %u %u %u AA_FIND TRUE\n", abs_coord.x, abs_coord.y, abs_coord.z);
    item = (cache_item_t *) cache_node->data;
  } else {
    printf("gather_chunk %u %u %u AA_FIND FALSE\n", abs_coord.x, abs_coord.y, abs_coord.z);
    if (item->instantiated) {
      printf("gather_chunk %u %u %u AA_DELETE START\n", abs_coord.x, abs_coord.y, abs_coord.z);
      aa_error_check(world_server->world_db.search_nodes, world_server->world_db.search_root, CACHE_LEN);
      aa_delete(cache_comparator, &(world_server->world_db.search_root), world_server->world_db.search_nodes, (void *) item);
      printf("gather_chunk %u %u %u AA_DELETE DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);
    } else {
      static int numitemsused = 0;
      numitemsused++;
      printf("cache utilization %f%%\n", (float) numitemsused / (float) CACHE_LEN);
      item->instantiated = true;
    }
    item->encoded_id = encoded_id;
//     printf("gather_chunk %u %u %u AA_INSERT START\n", abs_coord.x, abs_coord.y, abs_coord.z);
    aa_insert(cache_comparator, &(world_server->world_db.search_root), world_server->world_db.search_nodes, (void *) item, item->search_node);
//     printf("gather_chunk %u %u %u AA_INSERT DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);
    RWLOCK_WLOCK(&(item->chunk.rwlock));
//         printf("gather_chunk %u %u %u WLOCK\n", abs_coord.x, abs_coord.y, abs_coord.z);

    generate_chunk(abs_coord, &(item->chunk));
//         printf("gather_chunk %u %u %u GENCHUNK DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);

    RWLOCK_WUNLOCK(&(item->chunk.rwlock));
//         printf("gather_chunk %u %u %u WUNLOCK DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);

  }

  promote(item);
//     printf("gather_chunk %u %u %u PROMOTe DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);

  RWLOCK_RLOCK(&(item->chunk.rwlock));
//       printf("gather_chunk %u %u %u RLOCK DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);

  ret_chunk = &(item->chunk);
//       printf("gather_chunk %u %u %u ret chunk DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);

  MTX_UNLOCK(&(world_server->world_db.db_mtx));

//   printf("gather_chunk %u %u %u DONE\n", abs_coord.x, abs_coord.y, abs_coord.z);
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


  world_db->head = world_db->backing_data;
  world_db->backing_data[0] = (cache_item_t) {
    .prev = NULL,
    .chunk = (chunk_t) {
      .objects = world_db->backing_data[0].objects,
      .lights = world_db->backing_data[0].lights,
      .num_objects = 0,
      .num_lights = 0,
    },
    .next = world_db->backing_data + 1,
    .search_node = 1,
    .instantiated = false
  };
  RWLOCK_INIT(&(world_db->backing_data[0].chunk.rwlock));

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
      .search_node = i + 1,
      .instantiated = false
    };
    RWLOCK_INIT(&(world_db->backing_data[i].chunk.rwlock));
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
    .search_node = CACHE_LEN,
    .instantiated = false
  };
  RWLOCK_INIT(&(world_db->backing_data[CACHE_LEN - 1].chunk.rwlock));
  world_db->tail = world_db->backing_data + CACHE_LEN - 1;
  world_db->search_nodes[0] = (aa_node_t) {
    .left = 0,
    .right = 0,
    .data = NULL,
    .level = 0
  };
  world_db->search_root = 0;
//   world_db->search_tree = (aa_tree_t) {
//     .comparator = cache_comparator,
//     .nil = (aa_node_t) {
//       .left = &(world_db->search_tree.nil),
//       .right = &(world_db->search_tree.nil),
//       .data = NULL,
//       .level = 0
//     },
//     .root = &(world_db->search_tree.nil)
//   };
}

void end_world_db(world_db_t * world_db) {

}
