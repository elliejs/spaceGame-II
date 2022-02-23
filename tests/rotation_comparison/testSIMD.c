#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "render/render.h"

#include "utils/types/types.h"

int main(int argc, char const *argv[]) {
  srand(0);
  volatile float rots[4];
  for(int i = 0; i < 4; i++) {
    rots[i] = ((rand() % 1000) / 1000.) * 2. * M_PI;
  }
  const float rots_sin_arr[4] = {sin(rots[0]), sin(rots[1]), sin(rots[2]), sin(rots[3])};
  const float rots_cos_arr[4] = {cos(rots[0]), cos(rots[1]), cos(rots[2]), cos(rots[3])};

  volatile SGVec rots_sin = SGVec_Load_Array(rots_sin_arr);
  volatile SGVec rots_cos = SGVec_Load_Array(rots_cos_arr);

  volatile SGVec3D_t axis = (SGVec3D_t) {
    .x = SGVec_Load_Const(0.),
    .y = SGVec_Load_Const(1.),
    .z = SGVec_Load_Const(0.)
  };

  volatile SGVec3D_t point = (SGVec3D_t) {
    .x = SGVec_Load_Const(0.),
    .y = SGVec_Load_Const(0.),
    .z = SGVec_Load_Const(1.)
  };

  volatile SGVec3D_t ret = rot_vec3d(rots_sin, rots_cos, axis, point);


  printf("v%d (rotation by %f rad): %f %f %f\n",
    0,
    rots[0],
    vgetq_lane_f32(ret.x, 0),
    vgetq_lane_f32(ret.y, 0),
    vgetq_lane_f32(ret.z, 0));

  printf("v%d (rotation by %f rad): %f %f %f\n",
    1,
    rots[1],
    vgetq_lane_f32(ret.x, 1),
    vgetq_lane_f32(ret.y, 1),
    vgetq_lane_f32(ret.z, 1));

  printf("v%d (rotation by %f rad): %f %f %f\n",
    2,
    rots[2],
    vgetq_lane_f32(ret.x, 2),
    vgetq_lane_f32(ret.y, 2),
    vgetq_lane_f32(ret.z, 2));

  printf("v%d (rotation by %f rad): %f %f %f\n",
    3,
    rots[3],
    vgetq_lane_f32(ret.x, 3),
    vgetq_lane_f32(ret.y, 3),
    vgetq_lane_f32(ret.z, 3));


  return 0;
}
