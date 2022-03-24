#include <stdlib.h>
#include <stdio.h>

#include "render.h"
#include "../objects/object.h"
#include "../color/palette_helper.h"

#define STEP_MAX 1000
#define HIT_DIST 0.1
#define MAX_DIST 2.0 * CHUNK_SIZE

typedef
struct march_result_s {
  SGVec dists;
  SGVec3D_t point;
  SGVecUInt obj_idx;
  SGVecUInt chunk_idx;
  SGVecUInt validity;
}
march_result_t;

march_result_t ray_march(SGVec3D_t origin, SGVec3D_t rays, world_snapshot_t * snapshot) {
  SGVec dist_tot = SGVec_ZERO;

  SGVecUInt obj_idx = SGVecUInt_ZERO;
  SGVecUInt chunk_idx = SGVecUInt_ZERO;
  SGVecUInt hit_dist_test;
  SGVec3D_t point = origin;
  int i;
  for(i = 0; i < STEP_MAX; i++) {
    SGVec dist_step = SGVec_Load_Const(MAX_DIST);

    for(int c = 0; c < CUBE_NUM; c++) {
      for(unsigned int j = 0; j < snapshot->chunks[c]->num_objects; j++) {
        object_t * o = snapshot->chunks[c]->objects + j;
        SGVec dist_candidate = o->SGVec_distance(o, point);
        dist_step = SGVec_Minimum(dist_candidate, dist_step);

        SGVecUInt dist_ternary = SGVec_Less_Than(dist_candidate, dist_step);
        obj_idx = SGVecUInt_Ternary(dist_ternary, SGVecUInt_Load_Const(j), obj_idx);
        chunk_idx = SGVecUInt_Ternary(dist_ternary, SGVecUInt_Load_Const(c), chunk_idx);

      }
    }
    // for(unsigned int j = 0; j < snapshot.num_ships; j++) {
    //   object_t * o = snapshot.ships + j;
    //   SGVec dist_candidate = o->distance(o, point);
    //   dist_step = SGVec_Minimum(dist_step, dist_candidate);
    //
    //   SGVecUInt dist_ternary = SGVec_Less_Than(dist_candidate, dist_step);
    //   obj_idx = SGVecUInt_Ternary(dist_ternary, SGVecUInt_Load_Const(j), obj_idx);
    //   chunk_idx = SGVecUInt_Ternary(dist_ternary, SGVecUInt_Load_Const(c), chunk_idx);
    // }
    dist_step = SGVec_Mult_Float(dist_step, 0.5);
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
  printf("finding satisfied march took [%d] steps\n", i);
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

static inline
SGVec3D_t SGVec3D_normalize(SGVec3D_t vec) {
  SGVec recip_magnitude =
    SGVec_Recip_Sqrt(
      SGVec_Add_Mult_SGVec(
        SGVec_Add_Mult_SGVec(
          SGVec_Mult_SGVec(vec.x, vec.x),
          vec.y,
          vec.y
        ),
        vec.z,
        vec.z
      )
    );
  return (SGVec3D_t) {
    .x = SGVec_Mult_SGVec(vec.x, recip_magnitude),
    .y = SGVec_Mult_SGVec(vec.y, recip_magnitude),
    .z = SGVec_Mult_SGVec(vec.z, recip_magnitude)
  };
}

#define RAW_PIXEL_BLACK (raw_pixel_t) {.fore = OKLAB_BLACK, .back = OKLAB_BLACK, .shape = 0}
#define RAW_PIXEL_WHITE (raw_pixel_t) {.fore = (oklab_t) {100, 0, 0}, .back = OKLAB_BLACK, .shape = 15}

raw_pixel_t rays_to_pixel(SGVec3D_t rays, world_snapshot_t * snapshot) {
  //outgoing march
  march_result_t march_result = ray_march(snapshot->self->SGVec_origin, rays, snapshot);

  if (lanes_false(march_result.validity)) {
    // printf("nothing in that pixel\n");
    return
    RAW_PIXEL_BLACK;
    // RAW_PIXEL_WHITE;
  }

  uint32_t obj_idx[4];    SGVecUInt_Store_Array(obj_idx, march_result.obj_idx);
  uint32_t chunk_idx[4];  SGVecUInt_Store_Array(chunk_idx, march_result.chunk_idx);
  uint32_t validity[4];   SGVecUInt_Store_Array(validity, march_result.validity);
  float point_x[4];       SGVec_Store_Array(point_x, march_result.point.x);
  float point_y[4];       SGVec_Store_Array(point_y, march_result.point.y);
  float point_z[4];       SGVec_Store_Array(point_z, march_result.point.z);
  //
  // object_t * hit_objs[4];
  // for(int i = 0; i < 4; i++) {
  //   hit_objs[i] = snapshot->chunks[chunk_idx[i]]->objects + obj_idx[i];
  // }

  //// surface normals
  // SGVec3D_t normals = normal(hit_objs, march_result.point);
  float normal_arr_x[4] = {0.};
  float normal_arr_y[4] = {0.};
  float normal_arr_z[4] = {0.};

  float l_arr[4] = {0.};
  float a_arr[4] = {0.};
  float b_arr[4] = {0.};
  for (int i = 0; i < 4; i++) {
    if(validity[i]) {
      float3D_t point = (float3D_t) {point_x[i], point_y[i], point_z[i]};
      object_t * o = snapshot->chunks[chunk_idx[i]]->objects + obj_idx[i];
      oklab_t lab = o->get_color(o, point);
      l_arr[i] = lab.l;
      a_arr[i] = lab.a;
      b_arr[i] = lab.b;

      float3D_t normal_vec = o->float_normal(o, point);
      normal_arr_x[i] = normal_vec.x;
      normal_arr_y[i] = normal_vec.y;
      normal_arr_z[i] = normal_vec.z;
    }
  }
  //lighting/coloring
  SGVecOKLAB_t color = (SGVecOKLAB_t) {
    .l = SGVec_Load_Array(l_arr),
    .a = SGVec_Load_Array(a_arr),
    .b = SGVec_Load_Array(b_arr)
  };

  SGVec3D_t normals = (SGVec3D_t) {
    .x = SGVec_Load_Array(normal_arr_x),
    .y = SGVec_Load_Array(normal_arr_y),
    .z = SGVec_Load_Array(normal_arr_z)
  };

  march_result.point.x = SGVec_Add_SGVec(march_result.point.x, SGVec_Mult_Float(normals.x, 0.5));
  march_result.point.y = SGVec_Add_SGVec(march_result.point.y, SGVec_Mult_Float(normals.y, 0.5));
  march_result.point.z = SGVec_Add_SGVec(march_result.point.z, SGVec_Mult_Float(normals.z, 0.5));

  // printf("normals: x0 x1 x2 x3: %f %f %f %f\nnormals: y0 y1 y2 y3: %f %f %f %f\nnormals: z0 z1 z2 z3: %f %f %f %f\n", normal_arr_x[0], normal_arr_x[1], normal_arr_x[2], normal_arr_x[3], normal_arr_y[0], normal_arr_y[1], normal_arr_y[2], normal_arr_y[3], normal_arr_z[0], normal_arr_z[1], normal_arr_z[2], normal_arr_z[3]);
  // SGVec3D_t normals = (SGVec3D_t) {
  //   .x = SGVec_Load_Const(0.),
  //   .y = SGVec_Load_Const(1.),
  //   .z = SGVec_Load_Const(0.)
  // };

  for(int c = 0; c < CUBE_NUM; c++) {
    for(int l = 0; l < snapshot->chunks[c]->num_lights; l++) {
      object_t * light = snapshot->chunks[c]->lights[l];
      SGVec3D_t ray_to_light = SGVec3D_normalize((SGVec3D_t) {
        .x = SGVec_Sub_SGVec(light->SGVec_origin.x, march_result.point.x),
        .y = SGVec_Sub_SGVec(light->SGVec_origin.y, march_result.point.y),
        .z = SGVec_Sub_SGVec(light->SGVec_origin.z, march_result.point.z)
      });
      // printf("ray_to_light: x0 x1 x2 x3: %f %f %f %f\nray_to_light: y0 y1 y2 y3: %f %f %f %f\nray_to_light: z0 z1 z2 z3: %f %f %f %f\n\n", SGVec_Get_Lane(ray_to_light.x, 0), SGVec_Get_Lane(ray_to_light.x, 1), SGVec_Get_Lane(ray_to_light.x, 2), SGVec_Get_Lane(ray_to_light.x, 3), SGVec_Get_Lane(ray_to_light.y, 0), SGVec_Get_Lane(ray_to_light.y, 1), SGVec_Get_Lane(ray_to_light.y, 2), SGVec_Get_Lane(ray_to_light.y, 3), SGVec_Get_Lane(ray_to_light.z, 0), SGVec_Get_Lane(ray_to_light.z, 1), SGVec_Get_Lane(ray_to_light.z, 2), SGVec_Get_Lane(ray_to_light.z, 3));
      SGVec alignment =
      SGVec_Add_Mult_SGVec(
        SGVec_Add_Mult_SGVec(
          SGVec_Mult_SGVec(normals.x, ray_to_light.x),
          normals.y,
          ray_to_light.y
        ),
        normals.z,
        ray_to_light.z
      );
      // printf("alignment: %f %f %f %f\n", SGVec_Get_Lane(alignment, 0), SGVec_Get_Lane(alignment, 1), SGVec_Get_Lane(alignment, 2), SGVec_Get_Lane(alignment, 3));
      SGVecUInt aligned = SGVec_Gtr_Than(alignment, SGVec_ZERO);
      // printf("aligned: %x %x %x %x\n", SGVecUInt_Get_Lane(aligned, 0), SGVecUInt_Get_Lane(aligned, 1), SGVecUInt_Get_Lane(aligned, 2), SGVecUInt_Get_Lane(aligned, 3));
      SGVecUInt valid_aligned = SGVecUInt_And(aligned, march_result.validity);
      // printf("valid_aligned: %x %x %x %x\n", SGVecUInt_Get_Lane(valid_aligned, 0), SGVecUInt_Get_Lane(valid_aligned, 1), SGVecUInt_Get_Lane(valid_aligned, 2), SGVecUInt_Get_Lane(valid_aligned, 3));

      // if (lanes_false(valid_aligned)) {
      //   // printf("dot product failed\n");
      //   continue;
      // }
      // if (!lanes_true(valid_aligned)) return RAW_PIXEL_BLACK;

      // printf("have rays\n");
      //we have some rays to bother with!
      SGVec dists = light->SGVec_distance(light, march_result.point);
      march_result_t light_march = ray_march(march_result.point, ray_to_light, snapshot);
      // printf("dists: %f %f %f %f\nmarch_dists: %f %f %f %f\n", SGVec_Get_Lane(dists, 0), SGVec_Get_Lane(dists, 1), SGVec_Get_Lane(dists, 2), SGVec_Get_Lane(dists, 3), SGVec_Get_Lane(light_march.dists, 0), SGVec_Get_Lane(light_march.dists, 1), SGVec_Get_Lane(light_march.dists, 2), SGVec_Get_Lane(light_march.dists, 3));
      SGVecUInt unobstructed_rays = SGVecUInt_And(
        SGVec_Less_Than(
          SGVec_Sub_SGVec(
            dists,
            light_march.dists
          ),
          SGVec_Load_Const(1.)
        ),
        valid_aligned
      );
      // if (lanes_false(unobstructed_rays)) continue;
      // if (!lanes_true(unobstructed_rays))     return RAW_PIXEL_BLACK;
      // printf("have unobstructed rays\n");
      // printf("unobstructed_rays: %x %x %x %x\n\n", SGVecUInt_Get_Lane(unobstructed_rays, 0), SGVecUInt_Get_Lane(unobstructed_rays, 1), SGVecUInt_Get_Lane(unobstructed_rays, 2), SGVecUInt_Get_Lane(unobstructed_rays, 3));

      SGVecOKLAB_t star_color = light->star.get_lighting(&(light->star), dists);
      SGVec sub_pixel_luminence = SGVecUInt_Ternary(unobstructed_rays, SGVec_Mult_SGVec(star_color.l, alignment), SGVec_ZERO);
      color = (SGVecOKLAB_t) {
        .l = SGVec_Add_SGVec(color.l, sub_pixel_luminence),
        .a = SGVec_Add_Mult_SGVec(color.a, sub_pixel_luminence, star_color.a),
        .b = SGVec_Add_Mult_SGVec(color.b, sub_pixel_luminence, star_color.b)
      };
    }
  }

  float color_l[4];  SGVec_Store_Array(color_l, color.l);
  float color_a[4];  SGVec_Store_Array(color_a, color.a);
  float color_b[4];  SGVec_Store_Array(color_b, color.b);

  unsigned char shape = 0x00;
  oklab_t fore = OKLAB_BLACK;
  oklab_t back = OKLAB_BLACK;

  int num_fore = 0;

  if (lanes_true(march_result.validity)) { //all pixels colored
    SGVecShort max_l_temp = SGVecShort_Fold_Max(SGVec_Bottom_Short(color.l), SGVec_Top_Short(color.l));
    float max_l = SGVecShort_Get_Lane(SGVecShort_Fold_Max(max_l_temp, max_l_temp), 0);

    SGVecShort min_l_temp = SGVecShort_Fold_Min(SGVec_Bottom_Short(color.l), SGVec_Top_Short(color.l));
    float min_l = SGVecShort_Get_Lane(SGVecShort_Fold_Min(min_l_temp, min_l_temp), 0);

    float avg_l = (min_l + max_l) / (float) 2.;
    // printf("l_arr[0-3] lab_l[0-3], max_l, min_l, avg_l: %f %f %f %f / %f %f %f %f / %f %f %f\n", l_arr[0], l_arr[1], l_arr[2], l_arr[3], color_l[0], color_l[1], color_l[2], color_l[3], max_l, min_l, avg_l);

    for (int i = 0; i < 4; i++) {
      float i_l = color_l[i];
      if (i_l >= avg_l) {
        shape |= 1 << i;
        num_fore++;

        fore.l += i_l;
        fore.a += color_a[i] * i_l;
        fore.b += color_b[i] * i_l;
      } else {
        back.l += i_l;
        back.a += color_a[i] * i_l;
        back.b += color_b[i] * i_l;
      }
    }
  } else { //some black, prioritize edges over color accuracy
    for (int i = 0; i < 4; i++) {
      if (validity[i]) {
        shape |= 1 << i;
        num_fore++;

        float i_l = color_l[i];
        fore.l += i_l;
        fore.a += color_a[i] * i_l;
        fore.b += color_b[i] * i_l;
      }
    }
  }

  if(num_fore > 0) {
    fore.l /= (float) num_fore;
    // fore.a /= num_fore;
    // fore.b /= num_fore;
  }
  if(num_fore < 4) {
    back.l /= (float) (4 - num_fore);
  }

  // printf("raw-pixel: fore: [L: %f, a: %f, b: %f]", fore.l, fore.a, fore.b);
  // printf(" back: [L: %f, a: %f, b: %f]\n", back.l, back.a, back.b);
  return (raw_pixel_t) {
    .fore = fore,
    .back = back,
    .shape = shape
  };
}
