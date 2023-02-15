#ifndef CARD_SPRITES_H
#define CARD_SPRITES_H

#include "asset.h"


#define card (asset_t) { \
  .width = 13, \
  .height = 12, \
  .data = \
  "╭───────────╮" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "│           │" \
  "╰───────────╯" \
};

#define sign_pos (asset_aggregate_t) { \
  .width = 9, \
  .height = 5, \
  .offset_x = 0, \
  .offset_y = 0, \
  .assets = { \
    (asset_t) { \
      .width = 6, \
      .height = 1, \
      .offset_x = 3, \
      .offset_y = 0, \
      .data = \
      "━━━━━┓" \
    }, \
    (asset_t) { \
      .width = 5, \
      .height = 3, \
      .offset_x = 0, \
      .offset_y = 0, \
      .data = \
      "┏━   " \
      "┃    " \
      "┗━━━┓" \
    }, \
    (asset_t) { \
      .width = 5, \
      .height = 3, \
      .offset_x = 4, \
      .offset_y = 4, \
      .data = \
      "    ┃" \
      "    ┃" \
      "┗━━━┛" \
    } \
  } \
};

#define sign_neg (asset_aggregate_t) { \
  .width = 9, \
  .height = 5, \
  .offset_x = 0, \
  .offset_y = 0, \
  .assets = { \
    (asset_t) { \
      .width = 5, \
      .height = 3, \
      .offset_x = 0, \
      .offset_y = 0, \
      .data = \
      "┏━━━┓" \
      "┃    " \
      "┃    " \
    }, \
    (asset_t) { \
      .width = 5, \
      .height = 3, \
      .offset_x = 4, \
      .offset_y = 2, \
      .data = \
      "┗━━━┓" \
      "    ┃" \
      "   ━┛" \
    }, \
    (asset_t) { \
      .width = 6, \
      .height = 1, \
      .offset_x = 0, \
      .offset_y = 4, \
      .data = \
      "┗━━━━━" \
    }, \
  } \
};
#endif /* end of include guard: CARD_SPRITES_H */
