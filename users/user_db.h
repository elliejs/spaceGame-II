#ifndef USER_DB_H
#define USER_DB_H

#include "../math/aa_tree.h"

typedef
struct user_data_s {
  char * password;
}
user_data_t;

typedef
struct user_s {
  char * username;
  user_data_t * data;
}
user_t;

user_t * get_user(char * name);

#endif /* end of include guard: USER_DB_H */
