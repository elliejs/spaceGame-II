#ifndef ASSET_H
#define ASSET_H

typedef
struct asset_s {
  unsigned int width;
  unsigned int height;
  unsigned int offset_x;
  unsigned int offset_y;

  char ** data;
}
asset_t;

typedef
struct asset_aggregate_s {
  unsigned int width;
  unsigned int height;
  unsigned int offset_x;
  unsigned int offset_y;

  asset_t assets[3];
}
asset_aggregate_t;

#define CRV_CNR_PREFIX  (unsigned char) 0xe2, (unsigned char) 0x95
#define CRV_CNR_UPR_LFT (unsigned char) 0xad
#define CRV_CNR_UPR_RGT (unsigned char) 0xae
#define CRV_CNR_LWR_LFT (unsigned char) 0xb0
#define CRV_CNR_LWR_RGT (unsigned char) 0xaf
#define CRV_CNR_LEN 3

#define EDGE_PREFIX     (unsigned char) 0xe2, (unsigned char) 0x94
#define EDGE_VERT       (unsigned char) 0x82
#define EDGE_HORZ       (unsigned char) 0x80
#define EDGE_LEN 3

unsigned int draw_asset(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, asset_t * asset);
unsigned int draw_aggregate_asset(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y, asset_aggregate_t * asset);
unsigned int draw_card(char * buffer, unsigned int msg_idx, unsigned int offset_x, unsigned int offset_y);

#endif /* end of include guard: ASSET_H */
