#ifndef RENDER_H
#define RENDER_H

#include "../math/vector_3d.h"
#include "../world/world_server.h"
#define FOV 45

#define STEP_MAX 1000
#define HIT_DIST 0.1
#define MAX_DIST 2.0 * CHUNK_SIZE

typedef
struct raw_pixel_s {
  oklab_t fore;
  oklab_t back;
  unsigned char shape;
}
raw_pixel_t;

raw_pixel_t rays_to_pixel(SGVec3D_t rays, world_snapshot_t * snapshot);

#endif /* end of include guard: RENDER_H */
