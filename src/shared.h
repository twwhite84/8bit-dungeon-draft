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

    TEXTURES = 0x2500, // 48 quads
    MASKS = 0x3100,    // 16 quads
    QUADDEFS = 0x3300, // 64 defs
    ANIMDEFS = 0x3500, // 32 index table + defs
    STATENTS = 0x3600, // 32 entities
    PLAYER = 0x3780,
    MOVEENTS = 0x37A0, // 10 entities
    TILEBUFFER = 0x3980,
    CAMERA = 0x3D90,
    OFFBUFFER = 0x3DB0,
    TILEMAPS = 0x3E10, // 32 maps
    LITEMAPS = 0x4F30, // 17 maps

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