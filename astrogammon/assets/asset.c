#include <string.h>
#include <stdio.h>

#include "asset.h"
#include "../../nblessings/nblessings.h"

unsigned int draw_asset(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, asset_t * asset) {
  offset_x += asset->offset_x;
  offset_y += asset->offset_y;
  for (unsigned int i = 0; i < asset->height; i++) {
    memcpy(buffer + msg_idx, (unsigned char[]){MOVE_TO(1 + offset_x, 1 + offset_y + i)}, MOVE_TO_SIZE);
    msg_idx += MOVE_TO_SIZE;
    memset(buffer + msg_idx, ' ', asset->width);
    msg_idx += asset->width;
  }
  return msg_idx;
}

unsigned int draw_aggregate_asset(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, asset_aggregate_t * asset) {
  for (unsigned int i = 0; i < 3; i++) {
    msg_idx += draw_asset(buffer, msg_idx, offset_x, offset_y, asset->assets + i);
  }
  return msg_idx;
}

