#ifndef SHIP_H
#define SHIP_H

#include "../../math/vector_3d.h"
#include "../../world/chunk.h"
#include "../../users/user_db.h"



typedef
struct ship_s {
  SGVec4D_t attitude;
  SGVec height;
  SGVec la;
  SGVec lb;
  unsigned int vision;
  chunk_coord_t abs_coord;
}
ship_t;

#include "../object.h"

object_t create_ship(SGVec3D_t origin, chunk_coord_t abs_coord);
void load_ship(object_t * ship, user_data_t * user_data);
user_data_t export_ship(object_t * self);

#endif /* end of include guard: SHIP_H */
