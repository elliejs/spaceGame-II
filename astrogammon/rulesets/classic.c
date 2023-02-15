#include "classic.h"

static
bool pair(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 2) return false;
  return occurrence_table[2 - num_wilds];
}

static
bool wild_pair(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 2) return false;
  return num_wilds == 2;
}

static
bool three_of_a_kind(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 3) return false;
  return occurrence_table[3 - num_wilds];
}

static
bool two_pair(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 4) return false;
  return occurrence_table[2] == 2 || num_wilds == 2;
}

static
bool four_of_a_kind(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 4) return false;
  return occurrence_table[4 - num_wilds];
}

static
bool full_house(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 5) return false;
  if (num_wilds == 1 && (occurrence_table[2] >= 2 || (occurrence_table[3] && occurrence_table[1]))) return true;
  if (num_wilds == 2 && occurrence_table[2] && occurrence_table[1]) return true;
  return false;
}

static
bool flush(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  return (select_num == 5 && occurrence_table[4]);
}

static
bool three_pair(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 6) return false;
  if (num_wilds == 3) return true;
  if (occurrence_table[2] == 3 - num_wilds) return true;
  return false;
}

static
bool wild_house(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  if (select_num != 5) return false;
  if (num_wilds == 4) return true;
  if (num_wilds == 3 && occurrence_table[2]) return true;
  if (num_wilds == 2 && occurrence_table[3]) return true;
  return false;
}

static
bool wild_flush(unsigned int select_num, unsigned int * occurrence_table, unsigned int num_wilds) {
  return (num_wilds == 5 && select_num == 5);
}

ruleset_t create_ruleset_classic() {
  return (ruleset_t) {
    .pattern_num = 10,
    .buy_num = 1,
    .deal_num = 2,
    .river_add_num = 2,
    .river_open_num = 1,
    .wild_num = 2,
    .little_ante = 1,
    .big_ante = 1,
    .suit_num = 3,
    .suit_len = 10,
    .sign_num = 2,
    .buy_in_purse = 500,
    .end_points = 500,
    .pattern_ranking = {
      wild_flush,
      wild_house,
      three_pair,
      flush,
      full_house,
      four_of_a_kind,
      two_pair,
      three_of_a_kind,
      wild_pair,
      pair,
    },
  };
}
