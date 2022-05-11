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

march_result_t ray_march(SGVec3D_t origin, SGVec3D_t rays, world_snapshot_t * snapshot, object_t * self_ship) {
  SGVec dist_tot = SGVec_ZERO;

  SGVecUInt obj_idx = SGVecUInt_ZERO;
  SGVecUInt chunk_idx = SGVecUInt_ZERO;
  SGVecUInt hit_dist_test;
  SGVec3D_t point = origin;
  int i;
  for(i = 0; i < STEP_MAX; i++) {
    SGVec dist_step = SGVec_Load_Const(MAX_DIST);

    for(int c = 0; c < CUBE_NUM + 1; c++) {
      for(unsigned int j = 0; j < snapshot->chunks[c]->num_objects; j++) {
        object_t * o = snapshot->chunks[c]->objects + j;
        if (o == self_ship) continue;
        SGVec dist_candidate = o->distance(o, point);
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
  // printf("finding satisfied march took [%d] steps\n", i);
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

  uint32_t obj_idx[4];    SGVecUInt_Store_Array(obj_idx, march_result.obj_idx);
  uint32_t chunk_idx[4];  SGVecUInt_Store_Array(chunk_idx, march_result.chunk_idx);
  uint32_t validity[4];   SGVecUInt_Store_Array(validity, march_result.validity);
  // float point_x[4];       SGVec_Store_Array(point_x, march_result.point.x);
  // float point_y[4];       SGVec_Store_Array(point_y, march_result.point.y);
  // float point_z[4];       SGVec_Store_Array(point_z, march_result.point.z);
  //
  // object_t * hit_objs[4];
  // for(int i = 0; i < 4; i++) {
  //   hit_objs[i] = snapshot->chunks[chunk_idx[i]]->objects + obj_idx[i];
  // }

  //// surface normals
  // SGVec3D_t normals = normal(hit_objs, march_result.point);

  SGVecOKLAB_t colors;
  SGVec3D_t normals;

  // float normal_arr_x[4] = {0.};
  // float normal_arr_y[4] = {0.};
  // float normal_arr_z[4] = {0.};

  // float l_arr[4] = {0.};
  // float a_arr[4] = {0.};
  // float b_arr[4] = {0.};
  /// EWWWWWWWWWWWWWWWW
  if (validity[0]) {
    // float3D_t point = (float3D_t) {point_x[i], point_y[i], point_z[i]};
    object_t * o = snapshot->chunks[chunk_idx[0]]->objects + obj_idx[0];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point);
    SGVec3D_t obj_normal = o->normal(o, march_result.point);

    steal_lane(colors.l, obj_color.l, 0);
    steal_lane(colors.a, obj_color.a, 0);
    steal_lane(colors.b, obj_color.b, 0);

    steal_lane(normals.x, obj_normal.x, 0);
    steal_lane(normals.y, obj_normal.y, 0);
    steal_lane(normals.z, obj_normal.z, 0);
  }
  if (validity[1]) {
    object_t * o = snapshot->chunks[chunk_idx[1]]->objects + obj_idx[1];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point);
    SGVec3D_t obj_normal = o->normal(o, march_result.point);

    steal_lane(colors.l, obj_color.l, 1);
    steal_lane(colors.a, obj_color.a, 1);
    steal_lane(colors.b, obj_color.b, 1);

    steal_lane(normals.x, obj_normal.x, 1);
    steal_lane(normals.y, obj_normal.y, 1);
    steal_lane(normals.z, obj_normal.z, 1);
  }
  if (validity[2]) {
    object_t * o = snapshot->chunks[chunk_idx[2]]->objects + obj_idx[2];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point);
    SGVec3D_t obj_normal = o->normal(o, march_result.point);

    steal_lane(colors.l, obj_color.l, 2);
    steal_lane(colors.a, obj_color.a, 2);
    steal_lane(colors.b, obj_color.b, 2);

    steal_lane(normals.x, obj_normal.x, 2);
    steal_lane(normals.y, obj_normal.y, 2);
    steal_lane(normals.z, obj_normal.z, 2);
  }
  if (validity[3]) {
    object_t * o = snapshot->chunks[chunk_idx[3]]->objects + obj_idx[3];
    SGVecOKLAB_t obj_color = o->color(o, march_result.point);
    SGVec3D_t obj_normal = o->normal(o, march_result.point);

    steal_lane(colors.l, obj_color.l, 3);
    steal_lane(colors.a, obj_color.a, 3);
    steal_lane(colors.b, obj_color.b, 3);

    steal_lane(normals.x, obj_normal.x, 3);
    steal_lane(normals.y, obj_normal.y, 3);
    steal_lane(normals.z, obj_normal.z, 3);
  }
      // printf("obj[%d %d] color: %f %f %f\n", chunk_idx[i], obj_idx[i], lab.l, lab.a, lab.b);
      // l_arr[i] = lab.l;
      // a_arr[i] = lab.a;
      // b_arr[i] = lab.b;

      // normal_arr_x[i] = normal_vec.x;
      // normal_arr_y[i] = normal_vec.y;
      // normal_arr_z[i] = normal_vec.z;
  //lighting/coloring
  // SGVecOKLAB_t color = (SGVecOKLAB_t) {
  //   .l = SGVec_Load_Array(l_arr),
  //   .a = SGVec_Load_Array(a_arr),
  //   .b = SGVec_Load_Array(b_arr)
  // };
  //
  // SGVec3D_t normals = (SGVec3D_t) {
  //   .x = SGVec_Load_Array(normal_arr_x),
  //   .y = SGVec_Load_Array(normal_arr_y),
  //   .z = SGVec_Load_Array(normal_arr_z)
  // };


  // march_result.point.x = SGVec_Add_SGVec(march_result.point.x, SGVec_Mult_Float(normals.x, 10));
  // march_result.point.y = SGVec_Add_SGVec(march_result.point.y, SGVec_Mult_Float(normals.y, 10));
  // march_result.point.z = SGVec_Add_SGVec(march_result.point.z, SGVec_Mult_Float(normals.z, 10));

  // printf("normals: x0 x1 x2 x3: %f %f %f %f\nnormals: y0 y1 y2 y3: %f %f %f %f\nnormals: z0 z1 z2 z3: %f %f %f %f\n", normal_arr_x[0], normal_arr_x[1], normal_arr_x[2], normal_arr_x[3], normal_arr_y[0], normal_arr_y[1], normal_arr_y[2], normal_arr_y[3], normal_arr_z[0], normal_arr_z[1], normal_arr_z[2], normal_arr_z[3]);
  // SGVec3D_t normals = (SGVec3D_t) {
  //   .x = SGVec_Load_Const(0.),
  //   .y = SGVec_Load_Const(1.),
  //   .z = SGVec_Load_Const(0.)
  // };

  for(int c = 0; c < CUBE_NUM; c++) {
    for(int l = 0; l < snapshot->chunks[c]->num_lights; l++) {
      object_t * light = snapshot->chunks[c]->lights[l];
      // SGVecUInt light_isNOT_self = SGVecUInt_Load_Array(((uint32_t []) {
      //   (light == snapshot->chunks[chunk_idx[0]]->objects + obj_idx[0]) ? 0 : ~0,
      //   (light == snapshot->chunks[chunk_idx[1]]->objects + obj_idx[1]) ? 0 : ~0,
      //   (light == snapshot->chunks[chunk_idx[2]]->objects + obj_idx[2]) ? 0 : ~0,
      //   (light == snapshot->chunks[chunk_idx[3]]->objects + obj_idx[3]) ? 0 : ~0
      // }));

      SGVec3D_t ray_to_light =
      SGVec3D_normalize(
        (SGVec3D_t) {
        .x = SGVec_Sub_SGVec(light->origin.x, march_result.point.x),
        .y = SGVec_Sub_SGVec(light->origin.y, march_result.point.y),
        .z = SGVec_Sub_SGVec(light->origin.z, march_result.point.z)
      }
    )
    ;
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

      if (lanes_false(valid_aligned)) {
        // printf("dot product failed\n");
        continue;
      }
      // if (!lanes_true(valid_aligned)) return RAW_PIXEL_BLACK;

      // printf("have rays\n");
      //we have some rays to bother with!

      march_result.point.x = SGVec_Add_SGVec(march_result.point.x, SGVec_Mult_Float(normals.x, 2. * HIT_DIST));
      march_result.point.y = SGVec_Add_SGVec(march_result.point.y, SGVec_Mult_Float(normals.y, 2. * HIT_DIST));
      march_result.point.z = SGVec_Add_SGVec(march_result.point.z, SGVec_Mult_Float(normals.z, 2. * HIT_DIST));

      SGVec dists = light->distance(light, march_result.point);
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

      SGVecOKLAB_t star_color = light->star.radiance(&(light->star), dists);
      SGVec sub_pixel_luminence = SGVecUInt_Ternary(unobstructed_rays, alignment, SGVec_ZERO);
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
    SGVec mag_squared = SGVec_Add_Mult_SGVec(
      SGVec_Add_Mult_SGVec(
        SGVec_Mult_SGVec(colors.l, colors.l),
        colors.a,
        colors.a
      ),
      colors.b,
      colors.b
    );
    SGVecShort max_mag_temp = SGVecShort_Fold_Max(SGVec_Bottom_Short(mag_squared), SGVec_Top_Short(mag_squared));
    float max_mag = SGVecShort_Get_Lane(SGVecShort_Fold_Max(max_mag_temp, max_mag_temp), 0);

    SGVecShort min_mag_temp = SGVecShort_Fold_Min(SGVec_Bottom_Short(mag_squared), SGVec_Top_Short(mag_squared));
    float min_mag = SGVecShort_Get_Lane(SGVecShort_Fold_Min(min_mag_temp, min_mag_temp), 0);

    float avg_mag = (min_mag + max_mag) / (float) 2.;

    // SGVecShort max_mag_temp = SGVecShort_Fold_Max(SGVec_Bottom_Short(color.l), SGVec_Top_Short(color.l));
    // float max_mag = SGVecShort_Get_Lane(SGVecShort_Fold_Max(max_mag_temp, max_mag_temp), 0);
    //
    // SGVecShort min_mag_temp = SGVecShort_Fold_Min(SGVec_Bottom_Short(color.l), SGVec_Top_Short(color.l));
    // float min_mag = SGVecShort_Get_Lane(SGVecShort_Fold_Min(min_mag_temp, min_mag_temp), 0);
    //
    // float avg_mag = (min_mag + max_mag) / (float) 2.;

    // printf("l_arr[0-3] lab_l[0-3], max_l, min_l, avg_l: %f %f %f %f / %f %f %f %f / %f %f %f\n", l_arr[0], l_arr[1], l_arr[2], l_arr[3], color_l[0], color_l[1], color_l[2], color_l[3], max_l, min_l, avg_l);

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
