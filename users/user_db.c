#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>


#include "user_db.h"
#include "../utils/semaphore.h"
#include "../objects/ship/ship.h"
#include "../world/world_server.h"

typedef
struct user_data_s {
  char password[MAX_USERPASS_LEN];
  object_t self;
}
user_data_t;

typedef
struct user_s {
  char username[MAX_USERPASS_LEN];
  off_t offset;
  aa_node_t node;
}
user_t;

typedef
struct user_index_s {
  unsigned int num_users;
  aa_tree_t data;
  user_t backing_nodes[];
}
user_index_t;

typedef
struct user_db_s {
  unsigned int num_users;
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

compare_t user_comparator(void * a, void * b) {
  user_t * a_item = (user_t *) a;
  user_t * b_item = (user_t *) b;
  int cmp = strncmp(a_item->username, b_item->username, MAX_USERPASS_LEN);
  return cmp > 0 ? GT : cmp < 0 ? LT : EQ ;
}

void start_user_db(void) {
    if (user_db != NULL) {
    printf("[user_db]: Already instantiated, not double-mallocing\n");
    return;
  }

  user_db = (user_db_t *) mmap(NULL, sizeof(user_db_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  RWLOCK_INIT(&(user_db->rwlock_data));
  RWLOCK_INIT(&(user_db->rwlock_index));

  user_db->user_index_fd = open("users/user_index.database", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  user_db->user_data_fd = open("users/user_data.database", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

  read(user_db->user_index_fd, &(user_db->num_users),  sizeof(unsigned int));
  user_db->max_users = fminl(user_db->num_users * 2, MIN_DB_SIZE);
  user_db->user_index = mmap(NULL, user_db->max_users * sizeof(user_t) + sizeof(user_index_t), PROT_READ | PROT_WRITE, MAP_SHARED, user_db->user_index_fd, 0);
  user_db->user_data = mmap(NULL, user_db->max_users * sizeof(user_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, user_db->user_data_fd, 0);
}

void end_user_db(void) {
  RWLOCK_DESTROY(&(user_db->rwlock_data));
  RWLOCK_DESTROY(&(user_db->rwlock_index));
  munmap(user_db->user_index, user_db->max_users * sizeof(user_t) + sizeof(user_index_t));
  munmap(user_db->user_data, user_db->max_users * sizeof(user_data_t));
  munmap(user_db, sizeof(user_db_t));
  close(user_db->user_index_fd);
  close(user_db->user_data_fd);
}

off_t get_user(char const * name) {
  aa_node_t * result = NULL;
  user_t user;
  memcpy(user.username, name, MAX_USERPASS_LEN);
  RWLOCK_RLOCK(&(user_db->rwlock_index));
  if (aa_find(&(user_db->user_index->data), (void *) &user, &result)) {
    RWLOCK_RUNLOCK(&(user_db->rwlock_index));
    return ((user_t *) result->data)->offset;
  } else {
    RWLOCK_RUNLOCK(&(user_db->rwlock_index));
    return -1;
  }
}

void update_user_data(off_t user_offset, object_t * object) {
  const long PAGESIZE = sysconf(_SC_PAGESIZE);
  RWLOCK_WLOCK(&(user_db->rwlock_data));
    memcpy(&(user_db->user_data[user_offset].self), object, sizeof(object_t));
    uintptr_t page_aligned_addr = (uintptr_t) user_db->user_data + user_offset / PAGESIZE;
    uintptr_t page_aligned_offset = (uintptr_t) user_db->user_data + user_offset % PAGESIZE;
    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_data_t), MS_ASYNC);
  RWLOCK_WUNLOCK(&(user_db->rwlock_data));
}

void get_user_data(off_t user_offset, object_t * object) {
  RWLOCK_RLOCK(&(user_db->rwlock_data));
    memcpy(object, &(user_db->user_data[user_offset].self), sizeof(object_t));
  RWLOCK_RUNLOCK(&(user_db->rwlock_data));
}

off_t create_user(char const * username, char const * password) {
  if (user_db->num_users >= user_db->max_users) {
    user_db->max_users *= 2;
    RWLOCK_WLOCK(&(user_db->rwlock_data));
      munmap(user_db->user_data, user_db->num_users * sizeof(user_data_t));
      user_db->user_data = mmap(NULL, user_db->max_users * sizeof(user_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, user_db->user_data_fd, 0);
    RWLOCK_WUNLOCK(&(user_db->rwlock_data));

    RWLOCK_WLOCK(&(user_db->rwlock_index));
      munmap(user_db->user_index, user_db->num_users * sizeof(user_t) + sizeof(user_index_t));
      user_db->user_index = mmap(NULL, user_db->max_users * sizeof(user_t) + sizeof(user_index_t), PROT_READ | PROT_WRITE, MAP_SHARED, user_db->user_index_fd, 0);
    RWLOCK_WUNLOCK(&(user_db->rwlock_index));
  }

  off_t user_offset = -1;
  uintptr_t page_aligned_addr;
  uintptr_t page_aligned_offset;
  const long PAGESIZE = sysconf(_SC_PAGESIZE);

  RWLOCK_WLOCK(&(user_db->rwlock_index));
    user_t * free_user = user_db->user_index->backing_nodes + user_db->user_index->num_users;
    memcpy(free_user->username, username, MAX_USERPASS_LEN);
    user_offset = free_user->offset = user_db->user_index->num_users;
    aa_insert(&(user_db->user_index->data), (void *) free_user, &(free_user->node));
    void * new_user_addr = (void *) free_user;
    page_aligned_addr = (uintptr_t) new_user_addr / PAGESIZE;
    page_aligned_offset = (uintptr_t) new_user_addr % PAGESIZE;

    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_t), MS_ASYNC);
    user_db->user_index->num_users++;
    page_aligned_addr = (uintptr_t) &(user_db->user_index->num_users) / PAGESIZE;
    page_aligned_offset = (uintptr_t) &(user_db->user_index->num_users) % PAGESIZE;
    msync((void *) page_aligned_addr, page_aligned_offset + sizeof(user_db->user_index->num_users), MS_ASYNC);
  RWLOCK_WUNLOCK(&(user_db->rwlock_index));


  RWLOCK_WLOCK(&(user_db->rwlock_index));
    memcpy(user_db->user_data[user_offset].password, password, MAX_USERPASS_LEN);
  RWLOCK_WUNLOCK(&(user_db->rwlock_index));

  object_t self = create_ship((SGVec3D_t) {
      .x = SGVec_Load_Const(CHUNK_SIZE / 2.),
      .y = SGVec_Load_Const(CHUNK_SIZE / 2.),
      .z = SGVec_Load_Const(CHUNK_SIZE / 2.)
    },
    (chunk_coord_t) {0,0,0});
  update_user_data(user_offset, &self);

  return user_offset;
}


off_t login(char const * username, char const * password) {
  off_t user_offset = get_user(username);
  if (user_offset == -1) {
    return create_user(username, password);
  } else {
      RWLOCK_RLOCK(&(user_db->rwlock_data));
      user_data_t * user_data = user_db->user_data + user_offset;
      if (!strncmp(password, user_data->password, MAX_USERPASS_LEN)) {
        RWLOCK_RUNLOCK(&(user_db->rwlock_data));
        return user_offset;
      }
      RWLOCK_RUNLOCK(&(user_db->rwlock_data));
  }
  return -1;
}
