#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

#define STORED_ROWS 13
#define STORED_COLUMNS 20
#define CAMERA_ROWS (2 * STORED_ROWS)
#define CAMERA_COLUMNS (2 * STORED_COLUMNS)
#define CAMERA_WIDTH (8 * CAMERA_COLUMNS)
#define CAMERA_HEIGHT (8 * CAMERA_ROWS)

#define TEXTURES 0X2500 // 48 quads
#define MASKS 0X3100    // 16 quads
#define QUADDEFS 0x3300 // 64 defs
#define ANIMDEFS 0X3500 // 32 index table + defs
#define STATENTS 0X3600 // 32 entities
#define PLAYER 0x3780
#define MOVEENTS 0x37A0 // 10 entities
#define TILEBUFFER 0x3980
#define CAMERA 0x3D90
#define OFFBUFFER 0X3DB0
#define TILEMAPS 0x3E10 // 32 maps
#define LITEMAPS 0X4F30 // 17 maps

typedef struct {
    uint8_t q;
    uint8_t r;
} FloorResults;

extern uint8_t beebram[0x8000];

FloorResults floordiv(int a, int b);

#endif