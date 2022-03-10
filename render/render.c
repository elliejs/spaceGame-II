#include "render.h"
#include "../objects/object.h"
#include "../color/palette_helper.h"

#define STEP_MAX 1000
#define HIT_DIST 0.01
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
  SGVec dist_tot = SGVec_Load_Const(0.);

  SGVecUInt obj_idx = SGVecUInt_Load_Const(0);
  SGVecUInt chunk_idx = SGVecUInt_Load_Const(0);
  SGVecUInt hit_dist_test;
  SGVec3D_t point = origin;

  for(int i = 0; i < STEP_MAX; i++) {
    SGVec dist_step = SGVec_Load_Const(MAX_DIST);

    for(int c = 0; c < CUBE_NUM; c++) {
      for(unsigned int j = 0; j < snapshot->chunks[c]->num_objects; j++) {
        object_t * o = snapshot->chunks[c]->objects + j;
        SGVec dist_candidate = o->distance(o, point);
        dist_step = SGVec_Minimum(dist_step, dist_candidate);

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

    dist_tot = SGVec_Add_SGVec(dist_tot, dist_step);
    point = (SGVec3D_t) {
      .x = SGVec_Add_Mult_SGVec(origin.x, rays.x, dist_tot),
      .y = SGVec_Add_Mult_SGVec(origin.y, rays.y, dist_tot),
      .z = SGVec_Add_Mult_SGVec(origin.z, rays.z, dist_tot)
    };

    SGVecUInt max_dist_test = SGVec_Gtr_Or_Eq_Than(dist_tot, SGVec_Load_Const(MAX_DIST));
    hit_dist_test = SGVec_Less_Than(dist_step, SGVec_Load_Const(HIT_DIST)); //SGVec_Mult_SGVec(SGVec_Load_Const(dist_tot), SGVec_Load_Const(HIT_DIST));
    SGVecUInt complete_test = SGVecUInt_Or(max_dist_test, hit_dist_test);

    if (lanes_true(complete_test)) break;
  }

  SGVecUInt valid_hits_ternary = hit_dist_test;
  return (march_result_t) {
    .dists = dist_tot,
    .obj_idx = SGVecUInt_Ternary(valid_hits_ternary, obj_idx, SGVecUInt_Load_Const(0)),
    .chunk_idx = SGVecUInt_Ternary(valid_hits_ternary, chunk_idx, SGVecUInt_Load_Const(0)),
    .validity = valid_hits_ternary,
    .point = point
  };
}

static inline
SGVec3D_t normalize(SGVec3D_t vec) {
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

// IMPLEMENT NEVER? MAYBE LATER?
// vec3 tetrahedral_normal( in vec3 & p ) // for function f(p)
// {
//     const float h = 0.0001; // replace by an appropriate value
//     const vec2 k = vec2(1,-1);
//     return normalize( k.xyy*f( p + k.xyy*h ) +
//                       k.yyx*f( p + k.yyx*h ) +
//                       k.yxy*f( p + k.yxy*h ) +
//                       k.xxx*f( p + k.xxx*h ) );
// }
static inline
SGVec generic_distance(object_t ** objs, SGVec3D_t point) {
  SGVec dists = SGVecUInt_Load_Const(MAX_DIST);
  for(int i = 0; i < 4; i++) {
    dists = SGVec_Minimum(dists, objs[i]->distance(objs[i], point));
  }
  return dists;
}

SGVec3D_t normal(object_t ** objs, SGVec3D_t point) {
  const SGVec normal_epsilon = SGVec_Load_Const(0.0001);

  return normalize((SGVec3D_t) {
    .x = SGVec_Sub_SGVec(
      generic_distance(objs, (SGVec3D_t) {SGVec_Add_SGVec(point.x, normal_epsilon), point.y, point.z}),
      generic_distance(objs, (SGVec3D_t) {SGVec_Sub_SGVec(point.x, normal_epsilon), point.y, point.z})
    ),
    .y = SGVec_Sub_SGVec(
      generic_distance(objs, (SGVec3D_t) {point.x, SGVec_Add_SGVec(point.y, normal_epsilon), point.z}),
      generic_distance(objs, (SGVec3D_t) {point.x, SGVec_Sub_SGVec(point.y, normal_epsilon), point.z})
    ),
    .z = SGVec_Sub_SGVec(
      generic_distance(objs, (SGVec3D_t) {point.x, point.y, SGVec_Add_SGVec(point.z, normal_epsilon)}),
      generic_distance(objs, (SGVec3D_t) {point.x, point.y, SGVec_Sub_SGVec(point.z, normal_epsilon)})
    )
  });
}

#define RAW_PIXEL_BLACK (raw_pixel_t) {.fore = OKLAB_BLACK, .back = OKLAB_BLACK, .shape = 0}

raw_pixel_t rays_to_pixel(SGVec3D_t rays, world_snapshot_t * snapshot) {
  //outgoing march
  march_result_t march_result = ray_march(snapshot->self->origin, rays, snapshot);

  if (lanes_false(march_result.validity)) return RAW_PIXEL_BLACK;

  uint32_t obj_idx[4]; SGVecUInt_Store_Array(obj_idx, march_result.obj_idx);
  uint32_t chunk_idx[4]; SGVecUInt_Store_Array(chunk_idx, march_result.chunk_idx);

  object_t * hit_objs[4];
  for(int i = 0; i < 4; i++) {
    hit_objs[i] = snapshot->chunks[chunk_idx[i]]->objects + obj_idx[i];
  }

  //surface normals
  SGVec3D_t normals = normal(hit_objs, march_result.point);

  //lighting/coloring
  SGVecOKLAB_t color = (SGVecOKLAB_t) {
    .l = SGVec_Load_Const(0.),
    .a = SGVec_Load_Const(0.),
    .b = SGVec_Load_Const(0.)
  };

  for(int c = 0; c < CUBE_NUM; c++) {
    for(int l = 0; l < snapshot->chunks[c]->num_lights; l++) {
      object_t * light = snapshot->chunks[c]->lights[l];
      SGVec3D_t ray_to_light = normalize((SGVec3D_t) {
        .x = SGVec_Sub_SGVec(light->origin.x, march_result.point.x),
        .y = SGVec_Sub_SGVec(light->origin.y, march_result.point.y),
        .z = SGVec_Sub_SGVec(light->origin.z, march_result.point.z)
      });
      SGVecUInt alignment = SGVec_Gtr_Or_Eq_Than(
        SGVec_Add_Mult_SGVec(
          SGVec_Add_Mult_SGVec(
            SGVec_Mult_SGVec(
              normals.x,
              ray_to_light.x
            ),
            normals.y,
            ray_to_light.y
          ),
          normals.z,
          ray_to_light.z
        ),
        SGVec_Load_Const(0.)
      );
      SGVecUInt valid_aligned = SGVecUInt_And(alignment, march_result.validity);
      if (lanes_false(valid_aligned)) continue;

      //we have some rays to bother with!
      SGVec dists = light->distance(light, march_result.point);
      march_result_t light_march = ray_march(march_result.point, ray_to_light, snapshot);
      SGVecUInt unobstructed_rays = SGVecUInt_And(
        SGVec_Less_Than(
          SGVec_Sub_SGVec(
            dists,
            light_march.dists
          ),
          SGVec_Load_Const(2. * HIT_DIST)
        ),
        valid_aligned
      );
      if (lanes_false(unobstructed_rays)) continue;

      SGVecOKLAB_t star_color = light->star.get_lighting(&(light->star), dists);
      SGVec sub_pixel_luminence =  SGVecUInt_Ternary(unobstructed_rays, star_color.l, SGVec_Load_Const(0.));
      color = (SGVecOKLAB_t) {
        .l = SGVec_Add_SGVec(color.l,      sub_pixel_luminence),
        .a = SGVec_Add_Mult_SGVec(color.a, sub_pixel_luminence, star_color.a),
        .b = SGVec_Add_Mult_SGVec(color.b, sub_pixel_luminence, star_color.b)
      };
    }
  }

  color = normalize_SGVecOKLAB(color);


  float color_l[4];  SGVec_Store_Array(color_l, color.l);
  float color_a[4];  SGVec_Store_Array(color_a, color.a);
  float color_b[4];  SGVec_Store_Array(color_b, color.b);

  unsigned int shape = 0;
  oklab_t fore = OKLAB_BLACK;
  oklab_t back = OKLAB_BLACK;

  if (lanes_true(march_result.validity)) { //all pixels colored
    SGVecShort max_l_temp = SGVecShort_Fold_Max(SGVec_Bottom_Short(color.l), SGVec_Top_Short(color.l));
    float max_l = SGVecShort_Get_Lane(SGVecShort_Fold_Max(max_l_temp, max_l_temp), 0);

    SGVecShort min_l_temp = SGVecShort_Fold_Min(SGVec_Bottom_Short(color.l), SGVec_Top_Short(color.l));
    float min_l = SGVecShort_Get_Lane(SGVecShort_Fold_Min(min_l_temp, min_l_temp), 0);

    float avg_l = (min_l + max_l) / (float) 2.;

    for (int i = 0; i < 4; i++) {
      float i_l = color_l[i];
      if (i_l >= avg_l) {
        shape |= 1 << i;

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
    uint32_t validity[4]; SGVecUInt_Store_Array(validity, march_result.validity);
    for (int i = 0; i < 4; i++) {
      if (validity[i]) {
        shape |= 1 << i;

        float i_l = color_l[i];
        fore.l += i_l;
        fore.a += color_a[i] * i_l;
        fore.b += color_b[i] * i_l;
      }
    }
  }

  return (raw_pixel_t) {
    .fore = fore,
    .back = back,
    .shape = shape
  };
}
