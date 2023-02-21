#ifndef USER_DB_H
#define USER_DB_H

#include "../math/aa_tree.h"
#include "../math/vector_3d.h"
#include "../world/world_db.h"

#define MIN_DB_SIZE MAX_CLIENTS
#define MAX_USERNAME_LEN 32

off_t login(char * username, char * password);
void start_user_db(void);

#endif /* end of include guard: USER_DB_H */
