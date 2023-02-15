#ifndef ASTROGAMMON_SERVER_H
#define ASTROGAMMON_SERVER_H

#include <stdbool.h>
#include "../math/fast_list.h"
#include "../utils/semaphore.h"
#include "rulesets/ruleset.h"

#define MAX_ACTIVE_GAMES 256
#define MAX_PLAYERS 8

typedef
struct card_s {
  unsigned int value;
  unsigned int suit;
  unsigned int sign;
}
card_t;

typedef
struct player_s {
  card_t ** hand;
  unsigned int bought_num;
  unsigned int purse;
  int game_progress;

  FAST_LIST_LAZY_T(card_t *) selection;
  unsigned int selection_ranking;
  int selection_points;


  pthread_mutex_t player_mtx;
  pthread_cond_t player_cond;
  bool took_action;

  char bet_str[6]; //TODO FIXME
  unsigned int bet_strlen;
  unsigned int bet;
  unsigned int requested_sign;

  bool game_valid;
  bool trick_valid;

}
player_t;

typedef
enum game_phase_e {
  JOIN,
  ANTE,
  BID,
  BUY,
  SELECT,
  NUM_PHASES
}
game_phase_t;

typedef
struct game_s {
  unsigned int num_players;
  player_t ** players;

  unsigned int dealer;
  card_t * deck;
  unsigned int deck_idx;

  unsigned int trick_pot;
  unsigned int game_pot;

  unsigned int big_ante;
  unsigned int little_ante;
  unsigned int current_bid;

  unsigned int hand_sign;
  card_t ** river;
  ruleset_t ruleset;
  char uid[64];
  game_phase_t phase;
  pthread_mutex_t game_mtx;

  pthread_t pid;
}
game_t;

typedef
struct astrogammon_server_s {
  FAST_LIST_T(game_t *, MAX_ACTIVE_GAMES) active_games;
  pthread_mutex_t active_game_mtx;
}
astrogammon_server_t;

game_t * find_game(char * game_id);
char * create_game(ruleset_t ruleset);
void destroy_game(game_t * game);
void select_card(player_t * player, card_t * card);
void * gameflow(void * data);
void start_astrogammon_server();

#endif /* end of include guard: ASTROGAMMON_SERVER_H */
