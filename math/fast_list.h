#ifndef FAST_LIST_H
#define FAST_LIST_H

#define FAST_LIST_T(T, MAX)                     \
  struct {                                      \
    unsigned int num;                           \
    T data[MAX];                                \
  }

  #define FAST_LIST_LAZY_T(T)                   \
  struct {                                      \
    unsigned int num;                           \
    T * data;                                   \
  }

#define PUSH_FAST_LIST(FL, X)                   \
FL.data[FL.num++] = X

#define POP_FAST_LIST(FL, X)                    \
  for (unsigned int i = 0; i < FL.num; i++) {   \
    if (FL.data[i] == X) {                      \
      FL.data[i] = FL.data[--FL.num];           \
      break;                                    \
    }                                           \
  }

#endif /* end of include guard: FAST_LIST_H */
