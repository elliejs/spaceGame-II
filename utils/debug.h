#ifndef DEBUG_H
#define DEBUG_H

#if defined(SG_DEBUG)

#include <stdlib.h>

extern const char BUBBLE_ARR[3];
#define BUBBLE_LEN 5

#define printf(FORMAT, ...) {                             \
  char bubbles[BUBBLE_LEN + 1];                           \
  printf("%s: "FORMAT, getBubbles(bubbles), __VA_ARGS__); \
}

inline
char * getBubbles(char * bubbles) {
  for (int i = 0; i < BUBBLE_LEN; i++) {
    bubbles[i] = BUBBLE_ARR[rand() % 3];
  }
  bubbles[BUBBLE_LEN] = '\0';
  return bubbles;
}

#endif

#endif /* end of include guard: DEBUG_H */
