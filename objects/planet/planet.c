#include <stdio.h>

#include "planet.h"

#include "../../world_db/world_db.h"

static const float denom = ((float) 1.) / ((float) (1 << 5));

static inline
float fibonacci_hash(int x) {
  float ret = ((float) ((x * 2654435769) & ((2<<10) - 1))) * denom;
  printf("FIBOHASH: %f\n", ret);
  return ret;
}

static inline
SGVec SGVec_fibonacci_hash(SGVecUInt x) {
  SGVec ret = SGVec_Mult_Float(SGVec_Cast_SGVecUInt(SGVecUInt_And(SGVecUInt_Mult_UInt(x, 2654435769), SGVecUInt_Load_Const((1<<5) - 1))), denom);

  // printf("SGVEC FIBOHASH: %f %f %f %f\n", SGVec_Get_Lane(ret, 0), SGVec_Get_Lane(ret, 1), SGVec_Get_Lane(ret, 2), SGVec_Get_Lane(ret, 3));
  return ret;

}

float sph(float3D_t i, float3D_t f, float3D_t c) {
  // random radius at grid vertex i+c
  int hash_index = (((((int) (c.z + i.z)) << CHUNK_POW) | ((int) (c.y + i.y))) << CHUNK_POW) | ((int) (c.x + i.x));
  float rad = 0.0;//0.5*fibonacci_hash(hash_index);
  // distance to sphere at grid vertex i+c
  return float3D_distance(f, c) - rad;
}

SGVec SGVec_sph(SGVec3D_t i, SGVec3D_t f, SGVec3D_t c) {
  // random radius at grid vertex i+c
  SGVecUInt hash_index = (
    SGVecUInt_Shift_Left(
      SGVecUInt_Shift_Left(SGVecUInt_Cast_SGVec(SGVec_Add_SGVec(c.z, i.z)), CHUNK_POW)
      | SGVecUInt_Cast_SGVec(SGVec_Add_SGVec(c.y, i.y)),
      CHUNK_POW
    )
    | SGVecUInt_Cast_SGVec(SGVec_Add_SGVec(c.x, i.x))
  );

  SGVec rad = SGVec_Mult_Float(SGVec_fibonacci_hash(hash_index), 0.5);
  // distance to sphere at grid vertex i+c
  SGVec ret = SGVec_Sub_SGVec(SGVec3D_distance(f, c), rad);
  // printf("SGVEC sph_dist: %f %f %f %f\n", SGVec_Get_Lane(ret, 0), SGVec_Get_Lane(ret, 1), SGVec_Get_Lane(ret, 2), SGVec_Get_Lane(ret, 3));
  return ret;
}

float sdBase(float3D_t p) {
  float3D_t i;
  float3D_t f = (float3D_t) {
    .x = modff(p.x, &(i.x)),
    .y = modff(p.y, &(i.y)),
    .z = modff(p.z, &(i.z))
  };
  // distance to the 8 corners spheres
  return fmin(fmin(fmin(sph(i,f,(float3D_t){0,0,0}),
                        sph(i,f,(float3D_t){0,0,1})),
                   fmin(sph(i,f,(float3D_t){0,1,0}),
                        sph(i,f,(float3D_t){0,1,1}))),
              fmin(fmin(sph(i,f,(float3D_t){1,0,0}),
                        sph(i,f,(float3D_t){1,0,1})),
                   fmin(sph(i,f,(float3D_t){1,1,0}),
                        sph(i,f,(float3D_t){1,1,1}))));
}

SGVec SGVec_sdBase(SGVec3D_t p) {
  SGVec3D_t i = (SGVec3D_t) {
    .x = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVec(p.x)),
    .y = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVec(p.y)),
    .z = SGVec_Cast_SGVecInt(SGVecInt_Cast_SGVec(p.z))
  };
  SGVec3D_t f = (SGVec3D_t) {
    .x = SGVec_Sub_SGVec(p.x, i.x),
    .y = SGVec_Sub_SGVec(p.y, i.y),
    .z = SGVec_Sub_SGVec(p.z, i.z),
  };

  // printf("p.x1, i.x1, f.x1 [%f %f %f]\n", SGVec_Get_Lane(p.x, 0), SGVec_Get_Lane(i.x, 0), SGVec_Get_Lane(f.x, 0));

  // distance to the 8 corners spheres
  return SGVec_Minimum(SGVec_Minimum(SGVec_Minimum(SGVec_sph(i,f,(SGVec3D_t){SGVec_ZERO,SGVec_ZERO,SGVec_ZERO}),
                                                   SGVec_sph(i,f,(SGVec3D_t){SGVec_ZERO,SGVec_ZERO,SGVec_ONE})),
                                     SGVec_Minimum(SGVec_sph(i,f,(SGVec3D_t){SGVec_ZERO,SGVec_ONE, SGVec_ZERO}),
                                                   SGVec_sph(i,f,(SGVec3D_t){SGVec_ZERO,SGVec_ONE, SGVec_ONE}))),
                       SGVec_Minimum(SGVec_Minimum(SGVec_sph(i,f,(SGVec3D_t){SGVec_ONE, SGVec_ZERO,SGVec_ZERO}),
                                                   SGVec_sph(i,f,(SGVec3D_t){SGVec_ONE, SGVec_ZERO,SGVec_ONE})),
                                     SGVec_Minimum(SGVec_sph(i,f,(SGVec3D_t){SGVec_ONE, SGVec_ONE, SGVec_ZERO}),
                                                   SGVec_sph(i,f,(SGVec3D_t){SGVec_ONE, SGVec_ONE, SGVec_ONE}))));
}

static
SGVec SGVec_distance(object_t * self, SGVec3D_t point) {
  return SGVec_sdBase(point);
  return SGVec_Sub_SGVec(SGVec3D_distance(self->SGVec_origin, point), self->planet.SGVec_radius);
}

static
float float_distance(object_t * self, float3D_t point) {
  return sdBase(point);
  return float3D_distance(self->float_origin, point) - self->planet.float_radius;
}

static
oklab_t get_color(object_t * self, float3D_t point) {
  return (oklab_t) {0.2, 0., 0.}; //linear_srgb_to_oklab((rgb_t) {0., 1., 0.});
}

object_t create_planet(float3D_t origin) {
  return (object_t) {
    .float_origin = origin,
    .SGVec_origin = (SGVec3D_t) {
      .x = SGVec_Load_Const(origin.x),
      .y = SGVec_Load_Const(origin.y),
      .z = SGVec_Load_Const(origin.z)
    },
    .SGVec_distance = SGVec_distance,
    .float_distance = float_distance,
    .get_color = get_color,
    .planet = (planet_t) {
      .SGVec_radius = SGVec_Load_Const(50.),
      .float_radius = 50.
    }
  };
}
