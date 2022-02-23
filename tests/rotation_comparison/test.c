#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef
struct vec3_s {
  float x;
  float y;
  float z;
}
vec3;

typedef
struct quaternion_s {
  float x;
  float y;
  float z;
  float s;
}
quaternion_t;


static inline
vec3 purify(quaternion_t a) {
  return (vec3) {
    a.x,
    a.y,
    a.z
  };
}

static inline
quaternion_t vec3toQ(vec3 xyz, float s) {
  return (quaternion_t) {
    xyz.x,
    xyz.y,
    xyz.z,
    s
  };
}

static inline
vec3 vaddv(vec3 a, vec3 b) {
  return (vec3) {
    a.x + b.x,
    a.y + b.y,
    a.z + b.z
  };
}

static inline
vec3 vmultd(vec3 v, float d) {
  return (vec3) {
    v.x * d,
    v.y * d,
    v.z * d
  };
}

static inline
vec3 cross(vec3 a, vec3 b) {
  return (vec3) {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

static inline
float dot(vec3 a, vec3 b) {
  return
    a.x * b.x +
    a.y * b.y +
    a.z * b.z;
}

static inline
quaternion_t qmultq(quaternion_t a, quaternion_t b) {
  float
    a_s = a.s,
    b_s = b.s;
  vec3
    a_v = purify(a),
    b_v = purify(b);

  return vec3toQ(
    vaddv(
      vaddv(
        vmultd(b_v, a_s),
        vmultd(a_v, b_s)
      ),
      cross(a_v, b_v)
    ),                              //vect
    (a_s * b_s) - dot(a_v, b_v)     //scalar
  );
}

static inline
quaternion_t negate(quaternion_t a) {
  return (quaternion_t) {
    -a.x,
    -a.y,
    -a.z,
    a.s
  };
}

vec3 rotate(vec3 point, vec3 axis, float rad) {
  quaternion_t q = vec3toQ(vmultd(axis, sin(rad)), cos(rad));
  quaternion_t p = vec3toQ(point, 0.);
  quaternion_t r = qmultq(qmultq(q, p), negate(q));
  return purify(r);
}

int main(int argc, char const *argv[]) {
  srand(0);
  volatile float rots[4];
  for(int i = 0; i < 4; i++) {
    rots[i] = ((rand() % 1000) / 1000.) * 2. * M_PI;
  }

  volatile vec3 axis = (vec3) {
    .x = 0.,
    .y = 1.,
    .z = 0.
  };

  volatile vec3 point = (vec3) {
    .x = 0.,
    .y = 0.,
    .z = 1.
  };

  for(int i = 0; i < 4; i++) {
    volatile vec3 ret = rotate(point, axis, rots[i]);
    printf("v%d (rotation by %f rad): %f %f %f\n",
      i,
      rots[i],
      ret.x,
      ret.y,
      ret.z);
  }

  return 0;
}
