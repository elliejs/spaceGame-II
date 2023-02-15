#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

#include "astrogammon_server.h"
#include "gui.h"

astrogammon_server_t * astrogammon_server = NULL;

void start_astrogammon_server() {
  astrogammon_server = (astrogammon_server_t *) mmap(NULL, sizeof(astrogammon_server_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  astrogammon_server->active_games.num = 0;
  MTX_INIT(&(astrogammon_server->active_game_mtx));
}

game_t * find_game(char * game_id) {
  game_t * game = NULL;
  MTX_LOCK(&(astrogammon_server->active_game_mtx));
  for (unsigned int i = 0; i < astrogammon_server->active_games.num; i++) {
    if (!strncmp(astrogammon_server->active_games.data[i]->uid, game_id, 64)) {
      game = astrogammon_server->active_games.data[i];
      break;
    }
  }
  MTX_UNLOCK(&(astrogammon_server->active_game_mtx));
  return (!game || game->num_players == MAX_PLAYERS) ? NULL : game;
}

void create_deck(game_t * game) {
  for (unsigned int sign = 0; sign < game->ruleset.sign_num; sign++) {
    for (unsigned int suit = 0; suit < game->ruleset.suit_num; suit++) {
      for (unsigned int i = 0; i < game->ruleset.suit_len; i++) {
        game->deck[
          sign * game->ruleset.suit_len * game->ruleset.suit_num
          + suit * game->ruleset.suit_len
          + i] = (card_t) {
            .value = i + 1,
            .suit = suit,
            .sign = sign
          };
      }
    }
  }

  unsigned int deck_len =
  game->ruleset.wild_num
  + game->ruleset.suit_num
  * game->ruleset.suit_len
  * game->ruleset.sign_num;

  for (int idx = deck_len - game->ruleset.wild_num; idx < deck_len; idx++) {
    game->deck[idx] = (card_t) {
      .value = 0,
      .suit = 0,
      .sign = game->ruleset.sign_num
    };
  }
}

char * create_game(ruleset_t ruleset) {
  unsigned int deck_len =
  ruleset.suit_num
  * ruleset.suit_len
  * ruleset.sign_num
  + ruleset.wild_num;

  unsigned int river_total_num =
  ruleset.river_open_num
  + ruleset.river_add_num;

  unsigned int max_hand_num =
  ruleset.deal_num
  + ruleset.buy_num;

  unsigned int total_game_alloc_size =
  sizeof(game_t)
  + sizeof(card_t) * deck_len
  + sizeof(card_t *) * river_total_num
  + sizeof(player_t *) * MAX_PLAYERS
  + sizeof(player_t) * MAX_PLAYERS
  + sizeof(card_t *) * MAX_PLAYERS * max_hand_num
  + sizeof(card_t *) * MAX_PLAYERS * (river_total_num + max_hand_num)
  ;

  game_t * game = (game_t *) mmap(NULL, total_game_alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  printf("publishing game lives at %p\n", game);
  char * member_alloc_idx = (char *) game;
  member_alloc_idx += sizeof(game_t);

  *game = (game_t) {
    .trick_pot = 0,
    .game_pot = 0,
    .num_players = 0,
    .deck_idx = 0,
    .big_ante = ruleset.big_ante,
    .little_ante = ruleset.little_ante,
    .ruleset = ruleset,
    .dealer = 0,
    .uid = "TEST-TEST-TEST\0"
  };

  MTX_INIT(&(game->game_mtx));

  game->deck = (card_t *) member_alloc_idx,
  member_alloc_idx += sizeof(card_t) * deck_len;
  game->river = (card_t **) member_alloc_idx;
  member_alloc_idx += sizeof(card_t *) * river_total_num;
  game->players = (player_t **) member_alloc_idx;
  member_alloc_idx += sizeof(player_t *) * MAX_PLAYERS;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++) {
    game->players[i] = (player_t *) member_alloc_idx;
    member_alloc_idx += sizeof(player_t);
    game->players[i]->hand = (card_t **) member_alloc_idx;
    printf("player %d's hand lives at %p\n", i, game->players[i]->hand);
    member_alloc_idx += sizeof(card_t *) * max_hand_num;
    game->players[i]->selection.num = 0;
    game->players[i]->selection.data = (card_t **) member_alloc_idx;
    member_alloc_idx += sizeof(card_t *) * (max_hand_num + river_total_num);
  }

  create_deck(game);
  MTX_LOCK(&(astrogammon_server->active_game_mtx));
  PUSH_FAST_LIST(astrogammon_server->active_games, game);
  MTX_UNLOCK(&(astrogammon_server->active_game_mtx));
  pthread_create(&(game->pid), NULL, gameflow, (void *) game);
  printf("done creating game %s\n", game->uid);
  return game->uid;
}

void shuffle(game_t * game, int num_shuffles) {
  game->deck_idx = 0;

  unsigned int deck_len =
    game->ruleset.wild_num
    + game->ruleset.suit_num
    * game->ruleset.suit_len
    * game->ruleset.sign_num;

  for (int i = 0; i < num_shuffles; i++) {
    int a = rand() % deck_len;
    int b = rand() % deck_len;
    if (a == b) { i--; continue; }
    card_t c = game->deck[a];
    game->deck[a] = game->deck[b];
    game->deck[b] = c;
  }
}

void select_card(player_t * player, card_t * card) {
  if (!card) return;
  for (unsigned int i = 0; i <= player->selection.num; i++) {
    if (i == player->selection.num) {
      player->selection.data[player->selection.num++] = card;
    }
    if (player->selection.data[i] == card) {
      player->selection.data[i] = player->selection.data[--player->selection.num];
    }
  }
}

unsigned int ranking_helper(game_t * game, unsigned int select_num, unsigned int * occurrence_bins, unsigned int wild_num) {
  for (unsigned int inverted_ranking = 0; inverted_ranking < game->ruleset.pattern_num; inverted_ranking++) {
    if (game->ruleset.pattern_ranking[inverted_ranking](select_num, occurrence_bins, wild_num)) {
      return game->ruleset.pattern_num - inverted_ranking + 2;
    }
  }
  return 0;
}

void score_selection(game_t * game, player_t * player) {
  unsigned int suit_bins[game->ruleset.suit_num * game->ruleset.sign_num + 1];
  int selection_points = 0;
  for (unsigned int i = 0; i < player->selection.num; i++) {
    card_t * card = player->selection.data[i];
    suit_bins[game->ruleset.suit_num * card->sign + card->suit]++;
    selection_points += card->value * -1 * (game->hand_sign != card->sign);
  }
  unsigned int max_select_num = game->ruleset.river_open_num + game->ruleset.river_add_num + game->ruleset.buy_num + game->ruleset.deal_num;
  max_select_num += 1; //because bin 0 is for selecting zero and so we need one more bin.
  unsigned int occurrence_bins[max_select_num];
  for (unsigned int i = 0; i < max_select_num; i++) {
    occurrence_bins[i] = 0;
  }
  for (unsigned int i = 0; i < game->ruleset.suit_num * game->ruleset.sign_num; i++) {
    occurrence_bins[suit_bins[i]]++;
  }

  unsigned int wild_num = suit_bins[game->ruleset.suit_num * game->ruleset.sign_num];
  player->selection_ranking = ranking_helper(game, player->selection.num, occurrence_bins, wild_num);
  if (player->selection_ranking == 0 && selection_points >= 0) {
    player->selection_ranking += 1 + (wild_num > 0);
  }
  player->selection_points = selection_points;
}

//shuffle
//deal
//start river
//bid
//winner choose direction
//buy
//finish river
//select hands
//score
//distribute trick pot
//pass deal to left

void deal(game_t * game) {
  for (unsigned int j = 0; j < game->ruleset.deal_num; j++) {
    for (unsigned int i = game->dealer + 1; i < game->num_players; i++) {
      if (!game->players[i]->game_valid) continue;
      game->players[i]->hand[j] = game->deck + game->deck_idx++;
    }
    for (unsigned int i = 0; i < game->dealer + 1; i++) {
      if (!game->players[i]->game_valid) continue;
      game->players[i]->hand[j] = game->deck + game->deck_idx++;
    }
  }
  for (unsigned int i = 0; i < game->ruleset.river_open_num + 1; i++) {
    game->river[i] = game->deck + game->deck_idx++;
  }
}

void ante(game_t * game) {
  for (unsigned int i = game->dealer + 1; i < game->num_players; i++) {
    player_t * player = game->players[i];

    MTX_LOCK(&(player->player_mtx));
    player->took_action = false;
    if (player->game_valid) {
      while (!player->took_action)
        COND_WAIT(&(player->player_cond), &(player->player_mtx));
      player->took_action = false;
      if (player->game_valid && player->purse >= game->little_ante) {
        game->game_pot += game->little_ante;
        player->purse -= game->little_ante;
      } else {
        player->game_valid = false;
      }
    }
    MTX_UNLOCK(&(player->player_mtx));
  }
  for (unsigned int i = 0; i < game->dealer + 1; i++) {
    player_t * player = game->players[i];

    MTX_LOCK(&(player->player_mtx));
    player->took_action = false;
    if (player->game_valid) {
      while (!player->took_action)
        COND_WAIT(&(player->player_cond), &(player->player_mtx));
      player->took_action = false;
      if (player->game_valid && player->purse >= game->little_ante) {
        game->game_pot += game->little_ante;
        player->purse -= game->little_ante;
      } else {
        player->game_valid = false;
      }
    }
    MTX_UNLOCK(&(player->player_mtx));
  }
}

void bid(game_t * game) {
  player_t * winner = NULL;
  bool breakbreak = false;
  game->current_bid = game->big_ante;

  while (1) {
    for (unsigned int i = game->dealer + 1; i < game->num_players; i++) {
      player_t * player = game->players[i];
      if (player == winner) {
        breakbreak = true;
        break;
      }
      MTX_LOCK(&(player->player_mtx));
      player->took_action = false;
      if (player->game_valid && player->trick_valid) {
        while (!player->took_action)
          COND_WAIT(&(player->player_cond), &(player->player_mtx));
        player->took_action = false;
        if (player->trick_valid && player->purse >= player->bet && player->bet > game->current_bid) {
          game->trick_pot += player->bet;
          player->purse -= player->bet;
          game->hand_sign = player->requested_sign;
          winner = player;
        } else {
          player->trick_valid = false;
        }
      }
      MTX_UNLOCK(&(player->player_mtx));
    }
    if (breakbreak) break;
    for (unsigned int i = 0; i < game->dealer + 1; i++) {
      player_t * player = game->players[i];
      if (player == winner) {
        breakbreak = true;
        break;
      }
      MTX_LOCK(&(player->player_mtx));
      player->took_action = false;
      if (player->game_valid && player->trick_valid) {
        while (!player->took_action)
          COND_WAIT(&(player->player_cond), &(player->player_mtx));
        player->took_action = false;
        if (player->trick_valid && player->purse >= player->bet && player->bet > game->current_bid) {
          game->trick_pot += player->bet;
          player->purse -= player->bet;
          game->hand_sign = player->requested_sign;
          winner = player;
        } else {
          player->trick_valid = false;
        }
      }
      MTX_UNLOCK(&(player->player_mtx));
    }
    if (breakbreak) break;
  }
}

void buy(game_t * game) {

}

void finish_river(game_t * game) {
  unsigned int river_total_num =
  game->ruleset.river_open_num
  + game->ruleset.river_add_num;
  for (unsigned int i = game->ruleset.river_open_num; i < river_total_num; i++) {
    game->river[i] = game->deck + game->deck_idx++;
  }
}

void score_selections(game_t * game) {
  for (unsigned int i = 0; i < game->num_players; i++) {
    player_t * player = game->players[i];
    MTX_LOCK(&(player->player_mtx));
    player->took_action = false;
    MTX_UNLOCK(&(player->player_mtx));
  }
  for (unsigned int i = 0; i < game->num_players; i++) {
    player_t * player = game->players[i];
    MTX_LOCK(&(player->player_mtx));
    if (player->game_valid && player->trick_valid) {
      while (!player->took_action)
        COND_WAIT(&(player->player_cond), &(player->player_mtx));
      score_selection(game, player);
    }
    MTX_UNLOCK(&(player->player_mtx));
  }
}

void score_trick(game_t * game) {
  player_t * winner = NULL;
  for (unsigned int i = 0; i < game->num_players; i++) {
    player_t * curr_player = game->players[i];
    if (curr_player->selection_ranking == 0) {
      continue;
    }
    if (!winner) {
      winner = curr_player;
    } else if (curr_player->selection_ranking > winner->selection_ranking) {
      winner = curr_player;
    } else if (curr_player->selection_ranking == winner->selection_ranking) {
      if (curr_player->selection_points > winner->selection_points) {
        winner = curr_player;
      } else if (curr_player->selection_points == winner->selection_points) {
        printf("!!![TRICK] WARNING MEGA TIE NO BREAKER.!!!\n");
      }
    }
  }
  if (!winner) {
    game->game_pot += game->trick_pot;
  } else {
    winner->purse += game->trick_pot;
  }
  game->trick_pot = 0;
}

bool score_game(game_t * game) {
  player_t * winner = NULL;
  for (unsigned int i = 0; i < game->num_players; i++) {
    player_t * curr_player = game->players[i];
    curr_player->game_progress += curr_player->selection_points;
    if (curr_player->game_progress >= game->ruleset.end_points) {
      if (!winner) {
        winner = curr_player;
      } else if (curr_player->game_progress > winner->game_progress) {
        winner = curr_player;
      } else if (curr_player->game_progress == winner->game_progress) {
        printf("!!![GAME] WARNING MEGA TIE NO BREAKER.!!!\n");
      }
    }
  }
  if (winner) {
    winner->purse += game->game_pot;
    game->game_pot = 0;
    return true;
  }
  return false;
}

void * gameflow(void * data) {
  game_t * game = (game_t *) data;

  MTX_LOCK(&(game->game_mtx));
  game->phase = JOIN;
  printf("JOIN PHASE\n");
  MTX_UNLOCK(&(game->game_mtx));
  MTX_LOCK(&(game->players[0]->player_mtx));
  while (!game->players[0]->took_action)
    COND_WAIT(&(game->players[0]->player_cond), &(game->players[0]->player_mtx));
  MTX_UNLOCK(&(game->players[0]->player_mtx));
  while (1) {
    shuffle(game, 256);
    MTX_LOCK(&(game->game_mtx));
    game->phase = ANTE;
    display_command_tray(game->phase);
    printf("ANTE PHASE\n");
    MTX_UNLOCK(&(game->game_mtx));
    ante(game);
    deal(game);
    MTX_LOCK(&(game->game_mtx));
    game->phase = BID;
    display_command_tray(game->phase);
    printf("BID PHASE\n");
    MTX_UNLOCK(&(game->game_mtx));
    bid(game);
    MTX_LOCK(&(game->game_mtx));
    game->phase = BUY;
    // display_command_tray(game->phase);
    printf("BUY PHASE\n");
    MTX_UNLOCK(&(game->game_mtx));
    buy(game);
    finish_river(game);
    MTX_LOCK(&(game->game_mtx));
    game->phase = SELECT;
    display_command_tray(game->phase);
    printf("SELECT PHASE\n");
    MTX_UNLOCK(&(game->game_mtx));
    score_selections(game);

    score_trick(game);
    if (score_game(game)) break;

    unsigned int biggest_ante = game->big_ante + game->little_ante;
    game->little_ante = game->big_ante;
    game->big_ante = biggest_ante;
    game->dealer++;
    game->dealer %= game->num_players;
  }

  return NULL;
}

// void destroy_game(game_t * game) {
//   POP_FAST_LIST(active_games, game);
//   munmap(game, sizeof(game_t));
// }
