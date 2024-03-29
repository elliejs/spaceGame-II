#include <stdlib.h>
#include <stdio.h>

#include "render.h"
#include "../objects/object.h"
#include "../color/palette_helper.h"
#include "../world/world_control.h"

typedef
struct march_result_s {
  SGVec dists;
  SGVec3D_t point;
  SGVecUInt obj_idx;
  SGVecUInt chunk_idx;
  SGVecUInt validity;
}
march_result_t;

bool out_of_sight(SGVec3D_t origin, SGVec3D_t ray, object_t * object) {
  return false;
  SGVec3D_t object_vec = SGVec3D_normalize(SGVec3D_Sub_SGVec3D(object->origin, origin));
  SGVec3D_t eye_vec = SGVec3D_normalize(SGVec3D_Sub_SGVec3D(ray, origin));

  SGVec dot = SGVec3D_dot(object_vec, eye_vec);
  //if the dot between the eye ray and the direction to the object is negative
  // then it's out of view
  if (lanes_true(SGVec_Less_Or_Eq_Than(dot, SGVec_ZERO)))
    return true;

  SGVec object_perceptual_tan_theta =
    SGVec_Mult_SGVec(object->radius, SGVec_Reciprocal(SGVec3D_distance(origin, object->origin)));

  SGVec cross = SGVec3D_distance(SGVec3D_cross(object_vec, eye_vec), SGVec3D_ZERO);
  SGVec self_to_object_tan_theta =
    SGVec_Mult_SGVec(cross, SGVec_Reciprocal(dot));
  //if the object's occluded tan degrees is less than the ray to object tan degrees
  // then it's out of view
  // else it leaks into view.
  //This is possible to measure without taking out the tan because tan is a monotonic function
  return lanes_true(SGVec_Less_Than(self_to_object_tan_theta, object_perceptual_tan_theta));
}

march_result_t ray_march(SGVec3D_t origin, SGVec3D_t rays, world_snapshot_t * snapshot, object_t * self_ship) {
  SGVec dist_tot = SGVec_ZERO;

  SGVecUInt obj_idx = SGVecUInt_ZERO;
  SGVecUInt chunk_idx = SGVecUInt_ZERO;
  SGVecUInt hit_dist_test;
  SGVec3D_t point = origin;
  int steps;
  int skipped = 0;
  for(steps = 0; steps < STEP_MAX; steps++) {
    SGVec dist_step = SGVec_Load_Const(MAX_DIST);
    for(int c = 0; c < CUBE_NUM * 2; c++) {
      for(unsigned int j = 0; j < snapshot->chunks[c]->num_objects; j++) {
        object_t * o = snapshot->chunks[c]->objects + j;
        if (o == self_ship || out_of_sight(origin, rays, o)) {
          skipped++;
          continue;
        }
        SGVec dist_candidate = o->distance(o, point, c, snapshot->time);
        dist_step = SGVec_Minimum(dist_candidate, dist_step);
        SGVecUInt dist_ternary = SGVec_Less_Or_Eq_Than(dist_candidate, dist_step);
        obj_idx = SGVecUInt_Ternary(dist_ternary, SGVecUInt_Load_Const(j), obj_idx);
        chunk_idx = SGVecUInt_Ternary(dist_ternary, SGVecUInt_Load_Const(c), chunk_idx);
      }
    }


    // dist_step = SGVec_Mult_Float(dist_step, 0.5);
    dist_tot = SGVec_Add_SGVec(dist_tot, dist_step);
    point = (SGVec3D_t) {
      .x = SGVec_Add_Mult_SGVec(origin.x, rays.x, dist_tot),
      .y = SGVec_Add_Mult_SGVec(origin.y, rays.y, dist_tot),
      .z = SGVec_Add_Mult_SGVec(origin.z, rays.z, dist_tot)
    };

    SGVecUInt max_dist_test = SGVec_Gtr_Or_Eq_Than(dist_tot, SGVec_Load_Const(MAX_DIST));
    hit_dist_test = SGVec_Less_Than(SGVec_Absolute(dist_step), SGVec_Load_Const(HIT_DIST));

    if (lanes_true(SGVecUInt_Or(max_dist_test, hit_dist_test))) break;
  }

  // dist_tot = SGVec_Sub_SGVec(dist_tot, SGVec_Load_Const(3. * HIT_DIST));
  // if (steps >= STEP_MAX)
    // printf("couldn't satisfy the march :-(\n");
  // else
    // printf("finding satisfied march took [%d] steps\n", steps);
  // printf("Got to skip calculating a planet %d times this pixel\n", skipped);
  return (march_result_t) {
    .dists = dist_tot,
    .obj_idx = obj_idx,
    .chunk_idx = chunk_idx,
    .validity = hit_dist_test,
    .point = (SGVec3D_t) {
      .x = SGVec_Add_Mult_SGVec(origin.x, rays.x, dist_tot),
      .y = SGVec_Add_Mult_SGVec(origin.y, rays.y, dist_tot),
      .z = SGVec_Add_Mult_SGVec(origin.z, rays.z, dist_tot)
    }
  };
}

#define RAW_PIXEL_BLACK (raw_pixel_t) {.fore = OKLAB_BLACK, .back = OKLAB_BLACK, .shape = 0}
#define RAW_PIXEL_WHITE (raw_pixel_t) {.fore = (oklab_t) {1, 0, 0}, .back = OKLAB_BLACK, .shape = 15}

raw_pixel_t rays_to_pixel(SGVec3D_t rays, world_snapshot_t * snapshot) {
  //outgoing march
  march_result_t march_result = ray_march(snapshot->self->origin, rays, snapshot, snapshot->self);

  if (lanes_false(march_result.validity)) {
    // printf("nothing in that pixel\n");
    return
    RAW_PIXEL_BLACK;
    // RAW_PIXEL_WHITE;
  }

  uint32_t validity[4];  SGVecUInt_Store_Array(validity, march_result.validity);

  if (snapshot->self->ship.vision == RADAR) {
    float dists[4];      SGVec_Store_Array(dists, march_result.dists);
    oklab_t fore = OKLAB_BLACK;
    oklab_t back = OKLAB_BLACK;
    unsigned char shape = 0;
    float fore_num = 0;
    float fore_dist = 0;
    float L = 50;
    float C = 1.1;
//     float rainbow_dist = 1.;
    if (lanes_true(march_result.validity)) {
      float dist_avg = 0;
      float back_dist = 0;

      for (int i = 0; i < 4; i++) {
        dist_avg += dists[i];
      }
      dist_avg /= 4.;
      for (int i = 0; i < 4; i++) {
        if (dists[i] > dist_avg) {
          fore_dist += dists[i];
          shape |= 1 << i;
          fore_num++;
        } else {
          back_dist += dists[i];
        }
      }
      if (fore_num > 0) fore_dist /= fore_num;
      if (fore_num < 4) back_dist /= 4 - fore_num;
      fore = (oklab_t) {
        .l = fore_dist / L,
        .a = C,//C * (float) ((int) ((MAX_DIST - fore_dist) / (float) MAX_DIST)) * cosf((fore_dist / rainbow_dist) * 2 * M_PI),
        .b = C,//C * (float) ((int) ((MAX_DIST - fore_dist) / (float) MAX_DIST)) * sinf((fore_dist / rainbow_dist) * 2 * M_PI)
      };
      back = (oklab_t) {
        .l = back_dist / L,
        .a = C,//C * (float) (int) (MAX_DIST - back_dist) / MAX_DIST * cosf((back_dist / rainbow_dist) * 2 * M_PI),
        .b = C,//C * (float) (int) (MAX_DIST - back_dist) / MAX_DIST * sinf((back_dist / rainbow_dist) * 2 * M_PI)
      };
    } else {
      for (int i = 0; i < 4; i++) {
        if (validity[i]) {
          shape |= 1 << i;
          fore_dist += dists[i];
          fore_num++;
        }
      }
      if (fore_num > 0) fore_dist /= fore_num;
      fore = (oklab_t) {
        .l = fore_dist / L,
        .a = C,//C * (float) (int) (MAX_DIST - fore_dist) / MAX_DIST * cosf((fore_dist / rainbow_dist) * 2 * M_PI),
        .b = C,//C * (float) (int) (MAX_DIST - fore_dist) / MAX_DIST * sinf((fore_dist / rainbow_dist) * 2 * M_PI)
      };
    }

    return (raw_pixel_t) {
      .fore = fore,
      .back = back,
      .shape = shape
    };
  }


  uint32_t obj_idx[4];   SGVecUInt_Store_Array(obj_idx, march_result.obj_idx);
  uint32_t chunk_idx[4]; SGVecUInt_Store_Array(chunk_idx, march_result.chunk_idx);
  // if (chunk_idx[0] >= CUBE_NUM || chunk_idx[1] >= CUBE_NUM || chunk_idx[2] >= CUBE_NUM || chunk_idx[3] >= CUBE_NUM)
  //   printf("CHUNK_IDXS: %d, %d %d %d %d\n", CUBE_NUM, chunk_idx[0], chunk_idx[1], chunk_idx[2], chunk_idx[3]);

  SGVecOKLAB_t colors;
  SGVec3D_t normals;
  time_t time = snapshot->time;
  /// EWWWWWWWWWWWWWWWW
  if (validity[0]) {
    object_t * o = snapshot->chunks[chunk_idx[0]]->objects + obj_idx[0];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point, time);
    SGVec3D_t obj_normal = surface_normal(o, march_result.point, chunk_idx[0], time);

    steal_lane(colors.l, obj_color.l, 0);
    steal_lane(colors.a, obj_color.a, 0);
    steal_lane(colors.b, obj_color.b, 0);

    steal_lane(normals.x, obj_normal.x, 0);
    steal_lane(normals.y, obj_normal.y, 0);
    steal_lane(normals.z, obj_normal.z, 0);
  }
  if (validity[1]) {
    object_t * o = snapshot->chunks[chunk_idx[1]]->objects + obj_idx[1];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point, time);
    SGVec3D_t obj_normal = surface_normal(o, march_result.point, chunk_idx[1], time);

    steal_lane(colors.l, obj_color.l, 1);
    steal_lane(colors.a, obj_color.a, 1);
    steal_lane(colors.b, obj_color.b, 1);

    steal_lane(normals.x, obj_normal.x, 1);
    steal_lane(normals.y, obj_normal.y, 1);
    steal_lane(normals.z, obj_normal.z, 1);
  }
  if (validity[2]) {
    object_t * o = snapshot->chunks[chunk_idx[2]]->objects + obj_idx[2];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point, time);
    SGVec3D_t obj_normal = surface_normal(o, march_result.point, chunk_idx[2], time);

    steal_lane(colors.l, obj_color.l, 2);
    steal_lane(colors.a, obj_color.a, 2);
    steal_lane(colors.b, obj_color.b, 2);

    steal_lane(normals.x, obj_normal.x, 2);
    steal_lane(normals.y, obj_normal.y, 2);
    steal_lane(normals.z, obj_normal.z, 2);
  }
  if (validity[3]) {
    object_t * o = snapshot->chunks[chunk_idx[3]]->objects + obj_idx[3];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point, time);
    SGVec3D_t obj_normal = surface_normal(o, march_result.point, chunk_idx[3], time);

    steal_lane(colors.l, obj_color.l, 3);
    steal_lane(colors.a, obj_color.a, 3);
    steal_lane(colors.b, obj_color.b, 3);

    steal_lane(normals.x, obj_normal.x, 3);
    steal_lane(normals.y, obj_normal.y, 3);
    steal_lane(normals.z, obj_normal.z, 3);
  }

  march_result.point.x = SGVec_Add_SGVec(march_result.point.x, SGVec_Mult_Float(normals.x, 2. * HIT_DIST));
  march_result.point.y = SGVec_Add_SGVec(march_result.point.y, SGVec_Mult_Float(normals.y, 2. * HIT_DIST));
  march_result.point.z = SGVec_Add_SGVec(march_result.point.z, SGVec_Mult_Float(normals.z, 2. * HIT_DIST));

  for(int c = 0; c < CUBE_NUM; c++) {
    for(int l = 0; l < snapshot->chunks[c]->num_lights; l++) {
      object_t * light = snapshot->chunks[c]->lights[l];
      SGVec3D_t cube_offset = get_cube_offset(c);
      SGVec3D_t ray_to_light =
        SGVec3D_normalize(
          (SGVec3D_t) {
            .x = SGVec_Sub_SGVec(SGVec_Add_SGVec(light->origin.x, cube_offset.x), march_result.point.x),
            .y = SGVec_Sub_SGVec(SGVec_Add_SGVec(light->origin.y, cube_offset.y), march_result.point.y),
            .z = SGVec_Sub_SGVec(SGVec_Add_SGVec(light->origin.z, cube_offset.z), march_result.point.z)
          }
        );

      SGVec alignment = SGVec3D_dot(normals, ray_to_light);
      // printf("alignment: %f %f %f %f\n", SGVec_Get_Lane(alignment, 0), SGVec_Get_Lane(alignment, 1), SGVec_Get_Lane(alignment, 2), SGVec_Get_Lane(alignment, 3));
      SGVecUInt aligned = SGVec_Gtr_Than(alignment, SGVec_ZERO);
      // printf("aligned: %x %x %x %x\n", SGVecUInt_Get_Lane(aligned, 0), SGVecUInt_Get_Lane(aligned, 1), SGVecUInt_Get_Lane(aligned, 2), SGVecUInt_Get_Lane(aligned, 3));
      SGVecUInt valid_aligned = SGVecUInt_And(aligned, march_result.validity);
      // printf("valid_aligned: %x %x %x %x\n", SGVecUInt_Get_Lane(valid_aligned, 0), SGVecUInt_Get_Lane(valid_aligned, 1), SGVecUInt_Get_Lane(valid_aligned, 2), SGVecUInt_Get_Lane(valid_aligned, 3));

      if (lanes_false(valid_aligned)) {
        // printf("dot product failed\n");
        continue;
      }
      // if (!lanes_true(valid_aligned)) return RAW_PIXEL_BLACK;

      // printf("have rays\n");
      //we have some rays to bother with!


      SGVec dists = light->distance(light, march_result.point, c, time);
      // printf("trying to go to light\n");
      march_result_t light_march = ray_march(march_result.point, ray_to_light, snapshot, NULL);
      // printf("dists: %f %f %f %f\nmarch_dists: %f %f %f %f\n", SGVec_Get_Lane(dists, 0), SGVec_Get_Lane(dists, 1), SGVec_Get_Lane(dists, 2), SGVec_Get_Lane(dists, 3), SGVec_Get_Lane(light_march.dists, 0), SGVec_Get_Lane(light_march.dists, 1), SGVec_Get_Lane(light_march.dists, 2), SGVec_Get_Lane(light_march.dists, 3));
      SGVecUInt unobstructed_rays =
          SGVecUInt_And(
            SGVec_Less_Than(
              SGVec_Sub_SGVec(
                dists,
                light_march.dists
              ),
              SGVec_Load_Const(5. * HIT_DIST)
            ),
            valid_aligned
          );
      if (lanes_false(unobstructed_rays)) continue;
      // if (!lanes_true(unobstructed_rays))     return RAW_PIXEL_BLACK;
      // printf("have unobstructed rays\n");
      // printf("unobstructed_rays: %x %x %x %x\n\n", SGVecUInt_Get_Lane(unobstructed_rays, 0), SGVecUInt_Get_Lane(unobstructed_rays, 1), SGVecUInt_Get_Lane(unobstructed_rays, 2), SGVecUInt_Get_Lane(unobstructed_rays, 3));

      SGVecOKLAB_t star_color = light->star.radiance(&(light->star), dists, time);
      SGVec sub_pixel_luminence = SGVec_Ternary(unobstructed_rays, alignment, SGVec_ZERO);
      // SGVec sub_pixel_luminence = SGVec_Maximum(alignment, SGVec_ZERO);
      colors = (SGVecOKLAB_t) {
        .l = SGVec_Add_Mult_SGVec(colors.l, sub_pixel_luminence, star_color.l),
        .a = SGVec_Add_Mult_SGVec(colors.a, sub_pixel_luminence, star_color.a),
        .b = SGVec_Add_Mult_SGVec(colors.b, sub_pixel_luminence, star_color.b)
      };
    }
  }

  float color_l[4];  SGVec_Store_Array(color_l, colors.l);
  float color_a[4];  SGVec_Store_Array(color_a, colors.a);
  float color_b[4];  SGVec_Store_Array(color_b, colors.b);

  unsigned char shape = 0;
  oklab_t fore = OKLAB_BLACK;
  oklab_t back = OKLAB_BLACK;

  int num_fore = 0;

  if (lanes_true(march_result.validity)) { //all pixels colored
    SGVec mag_squared = SGVecOKLAB_dot(colors, colors);
    float mag_squared_arr[4];  SGVec_Store_Array(mag_squared_arr, mag_squared);
    float avg_mag = 0.;
    for (int i = 0; i < 4; i++) {avg_mag += mag_squared_arr[i];}
    avg_mag /= 4.;

    for (int i = 0; i < 4; i++) {
      float i_mag = color_l[i] * color_l[i] + color_a[i] * color_a[i] + color_b[i] * color_b[i];
      // float i_mag = fmax(color_l[i], 0.001);
      if (i_mag >= avg_mag) {
        shape |= 1 << i;
        num_fore++;

        fore.l += color_l[i];// * i_mag;
        fore.a += color_a[i] * i_mag;
        fore.b += color_b[i] * i_mag;
      } else {
        back.l += color_l[i];// * i_mag;
        back.a += color_a[i] * i_mag;
        back.b += color_b[i] * i_mag;
      }
    }
  } else { //some black, prioritize edges over color accuracy
    for (int i = 0; i < 4; i++) {
      if (validity[i]) {
        shape |= 1 << i;
        num_fore++;

        fore.l += color_l[i];// * color_l[i];
        fore.a += color_a[i] * color_l[i];
        fore.b += color_b[i] * color_l[i];
      }
    }
  }

  if(num_fore > 0) {
    fore.l /= (float) num_fore;
    fore.a /= (float) num_fore;
    fore.b /= (float) num_fore;
  }
  if(num_fore < 4) {
    back.l /= (float) (4 - num_fore);
    back.a /= (float) (4 - num_fore);
    back.b /= (float) (4 - num_fore);
  }

  // printf("raw-pixel: fore: [L: %f, a: %f, b: %f]", fore.l, fore.a, fore.b);
  // printf(" back: [L: %f, a: %f, b: %f]\n", back.l, back.a, back.b);
  return (raw_pixel_t) {
    .fore = fore,
    .back = back,
    .shape = shape
  };
}
