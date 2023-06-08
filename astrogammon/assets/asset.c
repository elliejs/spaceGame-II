#include <string.h>
#include <stdio.h>

#include "asset.h"
#include "card_sprites.h"
#include "text_sprites.h"

#include "../../nblessings/nblessings.h"

unsigned int draw_asset(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, asset_t * asset) {
  for (unsigned int i = 0; i < asset->height; i++) {
    memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1 + offset_x, 1 + offset_y + i)}, MOVE_TO_SIZE);
    msg_idx += MOVE_TO_SIZE;
    unsigned int asset_str_len = strlen(asset->data[i]);
    memcpy(buffer + msg_idx, asset->data[i], asset_str_len);
    msg_idx += asset_str_len;
  }
  return msg_idx;
}

unsigned int draw_aggregate_asset(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, asset_aggregate_t * asset) {
  for (unsigned int i = 0; i < 3; i++) {
    asset_t * sub_asset = asset->assets + i;
    msg_idx += draw_asset(buffer, msg_idx, offset_x + sub_asset->offset_x, offset_y + sub_asset->offset_y, sub_asset);
  }
  return msg_idx;
}

unsigned int draw_card(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y) {
  msg_idx += draw_asset(buffer, msg_idx, offset_x, offset_y, &card);
  return msg_idx;
}
