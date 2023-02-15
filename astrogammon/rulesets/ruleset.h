#ifndef RULESET_H
#define RULESET_H

#include <stdbool.h>

#define MAX_PATTERNS 16

typedef
struct ruleset_s {
  unsigned int pattern_num;
  bool (* pattern_ranking[MAX_PATTERNS])(unsigned int selected_num, unsigned int * occurrence_table, unsigned int wild_num);
  unsigned int buy_num;
  unsigned int deal_num;
  unsigned int river_add_num;
  unsigned int river_open_num;
  unsigned int suit_num;
  unsigned int sign_num;
  unsigned int suit_len;
  unsigned int wild_num;
  unsigned int big_ante;
  unsigned int little_ante;
  unsigned int buy_in_purse;
  unsigned int end_points;
}
ruleset_t;

#include "classic.h"

#endif /* end of include guard: RULESET_H */
