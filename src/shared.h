#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

enum {
    STORED_ROWS = 13,
    STORED_COLUMNS = 20,
    CAMERA_ROWS = (2 * STORED_ROWS),
    CAMERA_COLUMNS = (2 * STORED_COLUMNS),
    CAMERA_WIDTH = (8 * CAMERA_COLUMNS),
    CAMERA_HEIGHT = (8 * CAMERA_ROWS),

    TEXTURES = 0x2500,   // 48 textures + 16 texture-mask pairs
    QUADDEFS = 0x2D00,   // 64 quaddefs
    ANIMDEFS = 0x2F00,   // 32 indices + ~32 animdefs varsize
    PLAYER = 0x3050,     //
    STATENTS = 0x3070,   // 48 indices + ~48 statents varsize
    MOVEENTS = 0x3360,   // 12 entities
    TILEBUFFER = 0x3460, // 26x40 8x8px tiles
    CAMERA = 0x3870,     //
    OFFBUFFER = 0x3890,  // 12 8x8px tiles
    TILEMAPS = 0x3900,   // 32 maps
    LITEMAPS = 0x4E00,   // 17 maps

    SE_FRAMES5_TYPE3 = 0,
    SE_LENGTH2_ROOM6 = 1,
    SE_I8 = 3,
    SE_J8 = 4,
    SE_VIZDEF16 = 5,

    SE_DOORLOCKED = 0,
    SE_PICKUP = 1
};

typedef struct {
    uint8_t q;
    uint8_t r;
} FloorResults;

extern uint8_t beebram[0x8000];

FloorResults floordiv(int a, int b);

#endif