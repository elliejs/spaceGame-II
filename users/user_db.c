#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#include "user_db.h"
#include "../utils/semaphore.h"
#include "../objects/ship/ship.h"
#include "../world/world_server.h"

typedef
struct user_s {
  aa_node_t node;
  char username[MAX_USERPASS_LEN];
  char password[MAX_USERPASS_LEN];
  off_t data_off;
}
user_t;

typedef
struct user_index_s {
  unsigned int num_users;
  off_t search_root;
  user_t backing_nodes[1];
}
user_index_t;

typedef
struct user_db_s {
  aa_tree_t tree;
  unsigned int max_users;

  pthread_rwlock_t rwlock_index;
  pthread_rwlock_t rwlock_data;
  int user_index_fd;
  int user_data_fd;

  user_index_t * user_index;
  user_data_t * user_data;

}
user_db_t;

user_db_t * user_db = NULL;
static long PAGESIZE;

compare_t user_comparator(void * a, void * b) {
  user_t * a_item = (user_t *) a;
  user_t * b_item = (user_t *) b;
  int cmp = strncmp(a_item->username, b_item->username, MAX_USERPASS_LEN);
  return cmp > 0 ? GT : cmp < 0 ? LT : EQ ;
}

unsigned int get_num_users() {
  unsigned int ret;
  RWLOCK_RLOCK(&(user_db->rwlock_index));
  ret = user_db->user_index->num_users;
  RWLOCK_RUNLOCK(&(user_db->rwlock_index));
  return ret;
}

bool reallocate_databases(bool initial) {
  fstore_t store = (fstore_t) {
    .fst_flags = F_ALLOCATECONTIG,
    .fst_posmode = F_PEOFPOSMODE,
    .fst_offset = 0,
    .fst_length = DB_INC_SIZE * sizeof(user_data_t),
  };

  printf("a\n");

  RWLOCK_WLOCK(&(user_db->rwlock_index));
  if (!initial && user_db->user_index->num_users < user_db->max_users) {
    printf("[user_db]: databases online.\n\tNUM: %u\tMAX: %d\n", user_db->user_index->num_users, user_db->max_users);
    RWLOCK_WUNLOCK(&(user_db->rwlock_index));
    return true;
  }

  if (user_db->user_index) munmap(user_db->user_index, user_db->user_index->num_users * sizeof(user_t) + sizeof(user_index_t));
  store.fst_flags = F_ALLOCATECONTIG;
  store.fst_length = DB_INC_SIZE * sizeof(user_t);
  store.fst_bytesalloc = 0;
  if (-1 == fcntl(user_db->user_index_fd, F_PREALLOCATE, &store)) {
    printf("[user_db: user_index.database]: Can't make more user room. Trying noncontiguous allocation.\n\tstrerror: %s\n", strerror(errno));
    store.fst_flags = 0;
    if (-1 == fcntl(user_db->user_index_fd, F_PREALLOCATE, &store)) {
      printf("[user_db: user_index.database]: Can't make more user room. DATABASE FULL.\n\tstrerror: %s\n", strerror(errno));
      return false;
    }
  }

  unsigned int added_index_slots = store.fst_bytesalloc / sizeof(user_t);

  RWLOCK_WLOCK(&(user_db->rwlock_data));
  if (user_db->user_data) munmap(user_db->user_data, user_db->user_index->num_users * sizeof(user_data_t));
  store.fst_flags = F_ALLOCATECONTIG;
  if (-1 == fcntl(user_db->user_data_fd, F_PREALLOCATE, &store)) {
    printf("[user_db: user_data.database]: Can't make more user room. Trying noncontiguous allocation.\n\tstrerror: %s\n", strerror(errno));
    store.fst_flags = 0;
    if (-1 == fcntl(user_db->user_data_fd, F_PREALLOCATE, &store)) {
      printf("[user_db: user_data.database]: Can't make more user room. DATABASE FULL.\n\tstrerror: %s\n", strerror(errno));
      return false;
    }
  }
  unsigned int added_data_slots = store.fst_bytesalloc / sizeof(user_data_t);


  user_db->max_users += fminl(added_data_slots, added_index_slots);

  user_db->user_index = mmap(NULL, user_db->max_users * sizeof(user_t) + sizeof(user_index_t), PROT_READ | PROT_WRITE, MAP_SHARED, user_db->user_index_fd, 0);
  rebase_aa_tree(&(user_db->tree), (void *) &(user_db->user_index->backing_nodes[0].node), (void *) user_db->user_index->backing_nodes);
  user_db->user_data = mmap(NULL, user_db->max_users * sizeof(user_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, user_db->user_data_fd, 0);
  RWLOCK_WUNLOCK(&(user_db->rwlock_data));
  RWLOCK_WUNLOCK(&(user_db->rwlock_index));

  return true;
}

void start_user_db(void) {
  if (user_db != NULL) {
    printf("[user_db]: Already instantiated, not double-mallocing\n");
    return;
  }

  PAGESIZE = sysconf(_SC_PAGESIZE);

  user_db = (user_db_t *) mmap(NULL, sizeof(user_db_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  RWLOCK_INIT(&(user_db->rwlock_data));
  RWLOCK_INIT(&(user_db->rwlock_index));
  bool index_exists = false;
  bool data_exists = false;
  user_db->user_index_fd = open("users/user_index.database", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if ((user_db->user_index_fd == -1) && (errno == EEXIST)) {
    user_db->user_index_fd = open("users/user_index.database", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    index_exists = true;
  }

  user_db->user_data_fd = open("users/user_data.database", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if ((user_db->user_data_fd == -1) && (errno == EEXIST)) {
    user_db->user_data_fd = open("users/user_data.database", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    data_exists = true;
  }

  if (index_exists ^ data_exists) {
    printf("[user_db]: users database corrupted.\n\tuser_index.database file %s exist.\n\tuser_data.database file %s exist.\n\tServer Corrupted. Delete all database files or restore from backup.",
           index_exists ? "DOES" : "DOES NOT",
           data_exists ? "DOES" : "DOES NOT");
  }
  unsigned int num_users_temp = 0;
  if (!index_exists && !data_exists) {
    printf("[user_db]: databases not found. creating new...\n");
  } else {
    read(user_db->user_index_fd, &(num_users_temp),  sizeof(unsigned int));
  }
  user_db->max_users = fmaxl(num_users_temp + DB_INC_SIZE, MIN_DB_SIZE);

  user_db->user_data = NULL;
  user_db->user_index = NULL;
  user_db->tree = create_aa_tree(NULL, sizeof(user_t), NULL, user_comparator);
  if (!reallocate_databases(true))
    return;

  if (!index_exists && !data_exists) {
    printf("[user_db]: Creating header data within the user_index database\n");
    RWLOCK_WLOCK(&(user_db->rwlock_index));
      ftruncate(user_db->user_index_fd, sizeof(user_index_t));
      user_db->user_index->num_users = 0;
      user_db->user_index->backing_nodes[0] = (user_t) {
        .data_off = 0,
        .node = (aa_node_t) {
          .left = 0,
          .right = 0,
          .level = 0,
        },
      };
      user_db->user_index->search_root = 0;
    RWLOCK_WUNLOCK(&(user_db->rwlock_index));
    uintptr_t page_aligned_offset = (uintptr_t) user_db->user_index % PAGESIZE;
    uintptr_t page_aligned_addr = (uintptr_t) user_db->user_index - page_aligned_offset;
    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_index_t), MS_ASYNC);
  }
}

void end_user_db(void) {
  RWLOCK_DESTROY(&(user_db->rwlock_data));
  RWLOCK_DESTROY(&(user_db->rwlock_index));
  // ftruncate(user_db->user_index_fd, user_db->user_index->num_users * sizeof(user_t) + sizeof(user_index_t));
  // ftruncate(user_db->user_data_fd, user_db->user_index->num_users * sizeof(user_data_t));
  munmap(user_db->user_index, user_db->max_users * sizeof(user_t) + sizeof(user_index_t));
  munmap(user_db->user_data, user_db->max_users * sizeof(user_data_t));
  munmap(user_db, sizeof(user_db_t));
  close(user_db->user_index_fd);
  close(user_db->user_data_fd);
}

enum auth_fail_e {
  WRONG_PASS = -2,
  NO_USRNAME = -1,
};

off_t get_user(char const * name, char const * pass) {
  user_t * result = NULL;
  user_t user;
  strncpy(user.username, name, MAX_USERPASS_LEN);
  RWLOCK_RLOCK(&(user_db->rwlock_index));
  if (aa_find(&(user_db->tree), user_db->user_index->search_root, (void *) &user, (void *) &result)) {
    if(!strncmp(result->password, pass, MAX_USERPASS_LEN)) {
      off_t ret = result->data_off;
      RWLOCK_RUNLOCK(&(user_db->rwlock_index));
      return ret;
    }
    RWLOCK_RUNLOCK(&(user_db->rwlock_index));
    return WRONG_PASS;
  }
  RWLOCK_RUNLOCK(&(user_db->rwlock_index));
  return NO_USRNAME;
}

void update_user_data(off_t user_offset, user_data_t * user_data) {
  RWLOCK_WLOCK(&(user_db->rwlock_data));
    memcpy(user_db->user_data + user_offset, user_data, sizeof(user_data_t));
    uintptr_t page_aligned_offset = (uintptr_t) (user_db->user_data + user_offset) % PAGESIZE;
    uintptr_t page_aligned_addr = (uintptr_t) (user_db->user_data + user_offset) - page_aligned_offset;
    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_data_t), MS_ASYNC);
  RWLOCK_WUNLOCK(&(user_db->rwlock_data));
}

user_data_t get_user_data(off_t user_offset) {
  user_data_t user_data;
  RWLOCK_RLOCK(&(user_db->rwlock_data));
    user_data = user_db->user_data[user_offset];
  RWLOCK_RUNLOCK(&(user_db->rwlock_data));
  return user_data;
}

off_t create_user(char const * username, char const * password) {
  off_t user_data_offset = -1;
  uintptr_t page_aligned_addr;
  uintptr_t page_aligned_offset;

  if (!reallocate_databases(false)) {
    return -1;
  }

  RWLOCK_WLOCK(&(user_db->rwlock_index));
    ftruncate(user_db->user_index_fd, ++(user_db->user_index->num_users) * sizeof(user_t) + sizeof(user_index_t));
    user_t * free_user = user_db->user_index->backing_nodes + user_db->user_index->num_users;
    user_data_offset = user_db->user_index->num_users - 1;
    free_user->data_off = user_data_offset;
    strncpy(free_user->username, username, MAX_USERPASS_LEN);
    strncpy(free_user->password, password, MAX_USERPASS_LEN);

    aa_insert(&(user_db->tree), &(user_db->user_index->search_root), user_db->user_index->num_users);

    page_aligned_offset = (uintptr_t) free_user % PAGESIZE;
    page_aligned_addr = (uintptr_t) free_user - page_aligned_offset;
    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_t), MS_ASYNC);

    page_aligned_offset = (uintptr_t) &(user_db->user_index) % PAGESIZE;
    page_aligned_addr = (uintptr_t) &(user_db->user_index) - page_aligned_offset;
    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_index_t), MS_ASYNC);

  RWLOCK_WLOCK(&(user_db->rwlock_data));
    ftruncate(user_db->user_data_fd, user_db->user_index->num_users * sizeof(user_data_t));
  RWLOCK_WUNLOCK(&(user_db->rwlock_data));
  RWLOCK_WUNLOCK(&(user_db->rwlock_index));

  user_data_t user_data = (user_data_t) {
    .origin = (SGVec3D_t) {
      .x = SGVec_Load_Const(CHUNK_SIZE / 2.),
      .y = SGVec_Load_Const(CHUNK_SIZE / 2.),
      .z = SGVec_Load_Const(CHUNK_SIZE / 2.)
    },
    .abs_coord = (chunk_coord_t) {0,0,0}
  };

  update_user_data(user_data_offset, &user_data);

  return user_data_offset;
}


off_t login(char const * username, char const * password) {
  // aa_error_check(&(user_db->tree), user_db->user_index->search_root, user_db->max_users);
  off_t user_offset = get_user(username, password);
  switch (user_offset) {
    case NO_USRNAME:
      printf("[user_db]: LOGIN: <%s:%s> %s\n", username, password, "CREATE");
      return create_user(username, password);

    case WRONG_PASS:
      printf("[user_db]: LOGIN: <%s:%s> %s\n", username, password, "WRONG PASS");
      return -1;
    default:
      printf("[user_db]: LOGIN: <%s:%s> %s\n", username, password, "SUCCESS");
      return user_offset;
  }
}
