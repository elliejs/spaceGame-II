#ifndef TEXT_SPRITES_H
#define TEXT_SPRITES_H

#include "asset.h"

#include "text_sprites.h"

#define zero (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "│ │" \
  "╰─╯" \
};

#define one (asset_t) { \
  .width = 1, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╮" \
  "│" \
  "╰" \
};

#define two (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "╭─╯" \
  "╰─╯" \
};

#define three (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  " ─┤" \
  "╰─╯" \
};

#define four (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭ ╭" \
  "╰─┤" \
  "  ╯" \
};

#define five (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "╰─╮" \
  "╰─╯" \
};

#define six (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "├─╮" \
  "╰─╯" \
};

#define seven (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "  │" \
  "  ╰" \
};

#define eight (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "├─┤" \
  "╰─╯" \
};

#define nine (asset_t) { \
  .width = 3, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╭─╮" \
  "╰─┤" \
  "╰─╯" \
};

#define ten (asset_t) { \
  .width = 4, \
  .height = 3, \
  .offset_x = 0, \
  .offset_y = 0, \
  .data = \
  "╮╭─╮" \
  "││ │" \
  "╰╰─╯" \
};

#endif /* end of include guard: TEXT_SPRITES_H */
