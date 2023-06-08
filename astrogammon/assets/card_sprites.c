#include "asset.h"
#include "card_sprites.h"
#include <stdlib.h>

asset_t card;
asset_aggregate_t sign_pos;
asset_aggregate_t sign_neg;

void init_card() {
  card.width = 13;
  card.height = 12;
  card.data = (char **) malloc(card.height * sizeof(char *));
  card.data[0] = "╭───────────╮";
  card.data[1] = "│           │";
  card.data[2] = "│           │";
  card.data[3] = "│           │";
  card.data[4] = "│           │";
  card.data[5] = "│           │";
  card.data[6] = "│           │";
  card.data[7] = "│           │";
  card.data[8] = "│           │";
  card.data[9] = "│           │";
  card.data[10]= "│           │";
  card.data[11]= "╰───────────╯";
}

void init_sign_pos() {
  sign_pos = (asset_aggregate_t) {
    .width = 9,
    .height = 5,
    .offset_x = 0,
    .offset_y = 0,
    .assets = {
      (asset_t) {
        .width = 6,
        .height = 1,
        .offset_x = 3,
        .offset_y = 0,
        .data = (char **) malloc(sign_pos.assets[0].height * sizeof(char *)),
      },
      (asset_t) {
        .width = 5,
        .height = 3,
        .offset_x = 0,
        .offset_y = 0,
        .data = (char **) malloc(sign_pos.assets[1].height * sizeof(char *)),
      },
      (asset_t) {
        .width = 5,
        .height = 3,
        .offset_x = 4,
        .offset_y = 4,
        .data = (char **) malloc(sign_pos.assets[2].height * sizeof(char *)),
      },
    },
  };

  sign_pos.assets[0].data[0] = "━━━━━┓";

  sign_pos.assets[1].data[0] = "┏━   ";
  sign_pos.assets[1].data[1] = "┃    ";
  sign_pos.assets[1].data[2] = "┗━━━┓";

  sign_pos.assets[2].data[0] = "    ┃";
  sign_pos.assets[2].data[1] = "    ┃";
  sign_pos.assets[2].data[2] = "┗━━━┛";
}

void init_sign_neg() {
  sign_neg = (asset_aggregate_t) {
    .width = 9,
    .height = 5,
    .offset_x = 0,
    .offset_y = 0,
    .assets = {
      (asset_t) {
        .width = 5,
        .height = 3,
        .offset_x = 0,
        .offset_y = 0,
        .data = (char **) malloc(sign_neg.assets[0].height * sizeof(char *)),
      },
      (asset_t) {
        .width = 5,
        .height = 3,
        .offset_x = 4,
        .offset_y = 2,
        .data = (char **) malloc(sign_neg.assets[1].height * sizeof(char *)),
      },
      (asset_t) {
        .width = 6,
        .height = 1,
        .offset_x = 0,
        .offset_y = 4,
        .data = (char **) malloc(sign_neg.assets[2].height * sizeof(char *)),
      },
    },
  };

  sign_neg.assets[0].data[0] = "┏━━━┓";
  sign_neg.assets[0].data[1] = "┃    ";
  sign_neg.assets[0].data[2] = "┃    ";

  sign_neg.assets[1].data[0] = "┗━━━┓";
  sign_neg.assets[1].data[1] = "    ┃";
  sign_neg.assets[1].data[2] = "   ━┛";

  sign_neg.assets[2].data[0] = "┗━━━━━";
}


void init_all_card_sprites() {
  init_card();
  init_sign_pos();
  init_sign_neg();
}
