#include <string.h>

#include "user_db.h"
#include "../utils/semaphore.h"

compare_t user_comparator(void * a, void * b) {
  user_t * a_item = (user_t *) a;
  user_t * b_item = (user_t *) b;
  int cmp = strcmp(a_item->username, b_item->username);
  return cmp > 0 ? GT : cmp < 0 ? LT : EQ ;
}

struct {
  aa_tree_t data;
  pthread_mutex_t mtx;
}
user_db;

void start_user_db(void) {
  user_db.data = (aa_tree_t) {
    .comparator = user_comparator,
    .nil = (aa_node_t) {
      .left = &(user_db.data.nil),
      .right = &(user_db.data.nil),
      .data = NULL,
      .level = 0
    },
    .root = &(user_db.data.nil)
  };

  MTX_INIT(&(user_db.mtx));
}

user_t * get_user(char * name) {
  aa_node_t * result = NULL;
  user_t querent = (user_t) {
    .username = name,
  };

  if (find(&(user_db.data), (void *) &querent, &result)){
    return (user_t *) result->data;
  }
  else return NULL;
}


bool login(char * username, char * password) {

}
