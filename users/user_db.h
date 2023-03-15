#ifndef USER_DB_H
#define USER_DB_H

#include "../math/aa_tree.h"
#include "../math/vector_3d.h"
#include "../world/chunk.h"

#define MIN_DB_SIZE MAX_CLIENTS
#define DB_INC_SIZE MIN_DB_SIZE
#define MAX_USERPASS_LEN 32

typedef
struct user_data_s {
  SGVec3D_t origin;
  chunk_coord_t abs_coord;
  SGVec4D_t attitude;
}
user_data_t;

off_t login(char const * username, char const * password);
void update_user_data(off_t user_offset, user_data_t * user_data);
user_data_t get_user_data(off_t user_offset);

void start_user_db(void);
void end_user_db(void);

#endif /* end of include guard: USER_DB_H */
