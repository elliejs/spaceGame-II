#include "../ssh/ssh_client.h"
#include "astrogammon_server.h"
#include <stdlib.h>
#include "gui.h"

static
struct {
  game_t * game;
  player_t * player;
  pthread_t pid;
}
astrogammon_client;

void * astrogammon_client_task(void * nothing) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  for (;;) {
    MTX_LOCK(&(astrogammon_client.player->player_mtx));
    printf("all ready for a wakeup\n");
    while (!astrogammon_client.player->took_action) {
      printf("pre wait, took_action is: %d\n", astrogammon_client.player->took_action);
      COND_WAIT(&(astrogammon_client.player->player_cond), &(astrogammon_client.player->player_mtx));
      printf("post wait, took_action is: %d\n", astrogammon_client.player->took_action);
    }
    printf("all woken up");
    game_phase_t game_phase_private;
    MTX_LOCK(&(astrogammon_client.game->game_mtx));
    game_phase_private = astrogammon_client.game->phase;
    MTX_UNLOCK(&(astrogammon_client.game->game_mtx));
    display_command_tray(game_phase_private, astrogammon_client.player->selected_command);
    switch (game_phase_private) {
      case JOIN:
      case ANTE:
      case BID:
      case BUY:
      case SELECT:

      default:
        break;
    }
    printf("41 astrogammon_client took_action :== false\n");
    astrogammon_client.player->took_action = false;
    MTX_UNLOCK(&(astrogammon_client.player->player_mtx));
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  }
}

void start_astrogammon_client(int width, int height, int max_colors, ssh_channel channel, unsigned int id) {
  start_astrogammon_gui(width, height, max_colors, channel, id);
  pthread_create(&(astrogammon_client.pid), NULL, astrogammon_client_task, NULL);
}

bool join_game(char * game_tag) {
  game_t * game = find_game(game_tag);
  if (!game) return false;
  MTX_LOCK(&(game->game_mtx));
  player_t * player = game->players[game->num_players];
  game->num_players++;
  player->bought_num = 0;
  player->purse = game->ruleset.buy_in_purse;
  player->game_progress = 0;
  printf("63 astrogammon_client took_action :== false\n");
  player->took_action = false;
  player->game_valid = true;
  player->trick_valid = true;
  player->selected_command = -1;
  MTX_INIT(&(player->player_mtx));
  COND_INIT(&(player->player_cond));
  MTX_UNLOCK(&(game->game_mtx));
  unsigned int max_hand_num =
  game->ruleset.deal_num
  + game->ruleset.buy_num;
  for (unsigned int i = 0; i < max_hand_num; i++) {
    player->hand[i] = NULL;
  }

  astrogammon_client.game = game;
  astrogammon_client.player = player;
  return true;
}


int handle_input(void *data, uint32_t len, int is_stderr, void *userdata) {
  ssh_client_t * ssh_client = (ssh_client_t *) userdata;
  unsigned char * char_data = (unsigned char *) data;

  printf("[ssh_client %u]: %s\n", ssh_client->id, "channel_data_callback");
  printf("\t[ssh_client %u]: len: %d\n", ssh_client->id, len);
  printf("\t[ssh_client %u]: data: ", ssh_client->id);
  for(int i = 0; i < len; i++) {
    printf("%x ", char_data[i]);
  }
  printf("\n");


  game_t * game = astrogammon_client.game;
  player_t * player = astrogammon_client.player;

  char control_char = char_data[0];
  if(len == 3 && char_data[0] == 0x1b && char_data[1] == 0x5b) control_char = char_data[2]; //arrows

  bool took_action_private;
  MTX_LOCK(&(player->player_mtx));
  took_action_private = player->took_action;
  MTX_UNLOCK(&(player->player_mtx));
  printf("took_action_private %d\n", took_action_private);
  if (took_action_private) return len;

  game_phase_t game_phase;
  MTX_LOCK(&(game->game_mtx));
  game_phase = game->phase;
  MTX_UNLOCK(&(game->game_mtx));
  printf("switch game_phase\n");
  switch (game_phase) {
    case JOIN:
      // closes the game and proceeds to play
      if (control_char == 0x0d) { //ENTER
        MTX_LOCK(&(player->player_mtx));
        printf("JOIN took_action :== true\n");
        player->took_action = true;
        COND_SIGNAL(&(astrogammon_server->server_cond));
        MTX_UNLOCK(&(player->player_mtx));
      }
      break;

    case ANTE:
      switch (control_char) {
        case 0x61: //[A]NTE
          printf("anted clientside\n");
          MTX_LOCK(&(player->player_mtx));
          printf("ANTE ANTE took_action :== true\n");
          player->took_action = true;
          player->selected_command = 0;
          COND_SIGNAL(&(astrogammon_server->server_cond));
          MTX_UNLOCK(&(player->player_mtx));
          break;
        case 0x6c: //[L]EAVE
          MTX_LOCK(&(player->player_mtx));
          player->game_valid = false;
          printf("ANTE LEAVE took_action :== true\n");
          player->took_action = true;
          player->selected_command = 1;
          COND_SIGNAL(&(astrogammon_server->server_cond));
          MTX_UNLOCK(&(player->player_mtx));
          break;
      }
      break;

    case BID:
      if (control_char == 0x66) { // [F]OLD
        MTX_LOCK(&(player->player_mtx));
        player->trick_valid = false;
        printf("BID FOLD took_action :== true\n");
        player->took_action = true;
        COND_SIGNAL(&(astrogammon_server->server_cond));
        MTX_UNLOCK(&(player->player_mtx));
      }
      else if (control_char == 0x08) { //BACKSPACE
        if (player->bet_strlen == 0) break;
        player->bet_str[--(player->bet_strlen)] = '\0';
      }
      else if (control_char >= 0x30 && control_char <= 0x39) { //NUMBER
        player->bet_str[player->bet_strlen++] = control_char;
      }
      else if (control_char == 0x0d) { //ENTER
        player->bet_str[player->bet_strlen] = '\0';

        MTX_LOCK(&(player->player_mtx));
        player->bet = strtol(player->bet_str, NULL, 10);
        printf("BID BID took_action :== true\n");
        player->took_action = true;
        COND_SIGNAL(&(astrogammon_server->server_cond));
        MTX_UNLOCK(&(player->player_mtx));
      }
      else if (control_char == 0x41) { //up-arrow
        if (player->requested_sign + 1 < game->ruleset.sign_num)
          player->requested_sign++;
      }
      else if (control_char == 0x42) { //down-arrow
        if (player->requested_sign)
          player->requested_sign--;
      }
      break;

    case BUY:
      break;

    case SELECT:
      switch (control_char) {
        case 0x71:
          select_card(player, game->river[0]);
          break;
        case 0x77:
          select_card(player, game->river[1]);
          break;
        case 0x65:
          select_card(player, game->river[2]);
          break;

        case 0x31:
          select_card(player, player->hand[0]);
          break;
        case 0x32:
          select_card(player, player->hand[1]);
          break;
        case 0x33:
          select_card(player, player->hand[2]);
          break;

        case 0x0d: //ENTER
          MTX_LOCK(&(player->player_mtx));
          printf("SELECT SELECT took_action :== true\n");
          player->took_action = true;
          COND_SIGNAL(&(astrogammon_server->server_cond));
          MTX_UNLOCK(&(player->player_mtx));
          break;
      }
      break;
    default:
      break;
  }

  return len;
}
