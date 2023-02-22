#ifndef USER_DB_H
#define USER_DB_H

#include "../math/aa_tree.h"
#include "../math/vector_3d.h"
#include "../world/world_db.h"

#define MIN_DB_SIZE MAX_CLIENTS
#define MAX_USERPASS_LEN 32

off_t login(char const * username, char const * password);
void update_user_data(off_t user_offset, object_t * object);
void get_user_data(off_t user_offset, object_t * object);

void start_user_db(void);
void end_user_db(void);

#endif /* end of include guard: USER_DB_H */
