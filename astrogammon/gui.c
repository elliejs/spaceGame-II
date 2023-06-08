#include <stdlib.h>
#include <string.h>

#include "assets/asset.h"
#include "../nblessings/nblessings.h"
#include "gui.h"
#include "../color/palette_helper.h"

struct {
  char * message_buffer;
  unsigned int width;
  unsigned int height;
  ssh_channel channel;

  unsigned int num_colors;
  oklab_t * colors;
}
gui;

char *** command_trays[NUM_PHASES] = {
  [JOIN] = (char **[]) {
    (char *[]){"ENTER", "CLOSE GAME"},
  },
  [ANTE] = (char **[]) {
    (char *[]){"A", "ANTE"},
    (char *[]){"L", "LEAVE"}
  },
  [BID] = (char **[]) {
    (char *[]){"F", "FOLD"},
    (char *[]){"BACKSPACE", "DELETE BID"},
    (char *[]){"0-9", "BID AMOUNT"},
    (char *[]){"ENTER", "CONFIRM BID"},

  },
  [BUY] = (char **[]) {
    (char *[]){"UP", "BUY MORE"},
    (char *[]){"DOWN", "BUY FEWER"},
  },
  [SELECT] = (char **[]) {
    (char *[]){"1-3", "SELECT RIVER"},
    (char *[]){"Q-E", "SELECT HAND"}
  },
};

unsigned int num_commands_in_tray[NUM_PHASES] = {
  1,
  2,
  4,
  2,
  2,
};

#define MAX_MSG_LEN 2048

bool start_astrogammon_gui(unsigned int width, unsigned int height, unsigned int max_colors, ssh_channel channel, unsigned int id) {
  // unsigned int river_num = ruleset->river_open_num + ruleset->river_add_num;
  // unsigned int max_hand_num = ruleset->deal_num + ruleset->buy_num;
  // unsigned int max_card_width = river_num > max_hand_num ? river_num : max_hand_num;
  // max_card_width *=
  // if (width < max_card_width)
  gui.message_buffer = malloc(MAX_MSG_LEN);
  gui.width = width;
  gui.height = height;
  gui.channel = channel;

  gui.num_colors = 0;
  gui.colors = malloc(max_colors * sizeof(oklab_t));

  // INSTALL COLORS HERE
  install_palette(id, duel_init(), max_colors, gui.colors, &(gui.num_colors));

  unsigned int header_len;
  unsigned char * header_data = nblessings_header_data(gui.colors, gui.num_colors, &header_len);

  // printf("max_colors: %u, num_colors: %u\n", max_colors, render_client.num_colors);
  // printf("header_len: %u, header: %s\n", header_len, header_data);

  ssh_channel_write(gui.channel, header_data, header_len);

  return true;
}

unsigned int clear_rect(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, unsigned int width, unsigned int height) {
  for (unsigned int i = 0; i < height; i++) {
    memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1 + offset_x, 1 + offset_y + i)}, MOVE_TO_SIZE);
    msg_idx += MOVE_TO_SIZE;
    memset(buffer + msg_idx, ' ', width);
    msg_idx += width;
  }
  return msg_idx;
}

unsigned int draw_command_tray(char * buffer, unsigned int msg_idx, unsigned int height, unsigned int width, char *** commands, unsigned int num_commands, int selected_command) {
  unsigned int total_command_len = 2*num_commands;
  for (unsigned int i = 0; i < num_commands; i++) {
    total_command_len += strlen(commands[i][0]) + strlen(commands[i][1]);
  }
  unsigned int num_spaces = width - total_command_len;
  unsigned int space_around = num_spaces / (num_commands + 1);
  memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1 + space_around, 1 + height - 3)}, MOVE_TO_SIZE);
  msg_idx += MOVE_TO_SIZE;
  for (unsigned int i = 0; i < num_commands; i++) {
    if (i == selected_command) {
      printf("lmao bold\n");
      memcpy(buffer + msg_idx, (unsigned char[]){SET_BOLD}, SET_BOLD_SIZE);
      msg_idx += SET_BOLD_SIZE;
    }
    memcpy(buffer + msg_idx, (unsigned char[]){CRV_CNR_PREFIX, CRV_CNR_UPR_LFT}, CRV_CNR_LEN);
    msg_idx += CRV_CNR_LEN;
    for (unsigned int j = 0; j < strlen(commands[i][0]); j++) {
      memcpy(buffer + msg_idx, (unsigned char[]){EDGE_PREFIX, EDGE_HORZ}, EDGE_LEN);
      msg_idx += EDGE_LEN;
    }
    memcpy(buffer + msg_idx, (unsigned char[]){CRV_CNR_PREFIX, CRV_CNR_UPR_RGT, MOVE_X(space_around + strlen(commands[i][1]))}, CRV_CNR_LEN + MOVE_DIR_SIZE);
    msg_idx += CRV_CNR_LEN + MOVE_DIR_SIZE;
    if (i == selected_command) {
      memcpy(buffer + msg_idx, (unsigned char[]){UNSET_BOLD}, UNSET_BOLD_SIZE);
      msg_idx += UNSET_BOLD_SIZE;
    }
  }

  memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1 + space_around, 1 + height - 2)}, MOVE_TO_SIZE);
  msg_idx += MOVE_TO_SIZE;
  for (unsigned int i = 0; i < num_commands; i++) {
    if (i == selected_command) {
      memcpy(buffer + msg_idx, (unsigned char[]){SET_BOLD}, SET_BOLD_SIZE);
      msg_idx += SET_BOLD_SIZE;
    }
    memcpy(buffer + msg_idx, (unsigned char[]){EDGE_PREFIX, EDGE_VERT}, EDGE_LEN);
    msg_idx += EDGE_LEN;

    memcpy(buffer + msg_idx, commands[i][0], strlen(commands[i][0]));
    msg_idx += strlen(commands[i][0]);

    memcpy(buffer + msg_idx, (unsigned char[]){EDGE_PREFIX, EDGE_VERT}, EDGE_LEN);
    msg_idx += EDGE_LEN;

    memcpy(buffer + msg_idx, commands[i][1], strlen(commands[i][1]));
    msg_idx += strlen(commands[i][1]);

    memcpy(buffer + msg_idx, (unsigned char[]){MOVE_X(space_around)}, MOVE_DIR_SIZE);
    msg_idx += MOVE_DIR_SIZE;
    if (i == selected_command) {
      memcpy(buffer + msg_idx, (unsigned char[]){UNSET_BOLD}, UNSET_BOLD_SIZE);
      msg_idx += UNSET_BOLD_SIZE;
    }
  }

  memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1 + space_around, 1 + height - 1)}, MOVE_TO_SIZE);
  msg_idx += MOVE_TO_SIZE;
  for (unsigned int i = 0; i < num_commands; i++) {
    if (i == selected_command) {
      memcpy(buffer + msg_idx, (unsigned char[]){SET_BOLD}, SET_BOLD_SIZE);
      msg_idx += SET_BOLD_SIZE;
    }
    memcpy(buffer + msg_idx, (unsigned char[]){CRV_CNR_PREFIX, CRV_CNR_LWR_LFT}, CRV_CNR_LEN);
    msg_idx += CRV_CNR_LEN;
    for (unsigned int j = 0; j < strlen(commands[i][0]); j++) {
      memcpy(buffer + msg_idx, (unsigned char[]){EDGE_PREFIX, EDGE_HORZ}, EDGE_LEN);
      msg_idx += EDGE_LEN;
    }
    memcpy(buffer + msg_idx, (unsigned char[]){CRV_CNR_PREFIX, CRV_CNR_LWR_RGT, MOVE_X(space_around + strlen(commands[i][1]))}, CRV_CNR_LEN + MOVE_DIR_SIZE);
    msg_idx += CRV_CNR_LEN + MOVE_DIR_SIZE;
    if (i == selected_command) {
      memcpy(buffer + msg_idx, (unsigned char[]){UNSET_BOLD}, UNSET_BOLD_SIZE);
      msg_idx += UNSET_BOLD_SIZE;
    }
  }

  return msg_idx;
}

unsigned int clear_command_tray(char * buffer, unsigned int msg_idx, unsigned int height) {
  memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1, 1 + height - 3)}, MOVE_TO_SIZE);
  msg_idx += MOVE_TO_SIZE;
  memcpy(buffer + msg_idx, (unsigned char[]){CSI, 'J'}, 3);
  msg_idx += 3;
  return msg_idx;
}


void write_buff(char * buffer, unsigned int len) {
  unsigned int written = 0;
  buffer[len] = '\0';
  printf("would like to write %u\n", len);
  while (written < len) {
    printf("written (before: %u)\n", written);
    written += ssh_channel_write(gui.channel, (void *) buffer + written, len - written);
    printf("written (after: %u)\n", written);
  }
}

void display_command_tray(game_phase_t phase, int selected_command) {
  printf("display_command_tray %d\n", selected_command);
  unsigned int msg_idx = 0;
  msg_idx += clear_command_tray(gui.message_buffer, msg_idx, gui.height);
  msg_idx += draw_command_tray(gui.message_buffer, msg_idx, gui.height, gui.width, command_trays[phase], num_commands_in_tray[phase], selected_command);
  write_buff(gui.message_buffer, msg_idx);
}


