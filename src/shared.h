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

    TEXTURES = 0x2300,   // 48 textures + 24 texture-mask pairs (worst case)
    QUADDEFS = 0x2C00,   // 96 quaddefs
    ANIMDEFS = 0x2F00,   // 32 indices + ~32 animdefs varsize
    PLAYER = 0x3050,     //
    STATENTS = 0x3070,   // 48 indices + ~48 statents varsize
    MOVEENTS = 0x3360,   // 12 entities
    TILEBUFFER = 0x3460, // 26x40 8x8px tiles
    CAMERA = 0x3870,     //
    OFFBUFFER = 0x3890,  // 12 8x8px tiles
    TILEMAPS = 0x3900,   // 32 maps
    LITEMAPS = 0x4E00,   // 17 maps

    SE_ELAPSED5_TYPE3 = 0,
    SE_NQUADS2_ROOMID6 = 1,
    SE_REDRAW1_DATA7 = 2,
    SE_DATA24 = 3,
    SE_I = 6,
    SE_J = 7,
    SE_PVIZDEF_LO = 8,
    SE_PVIZDEF_HI = 9,

    AD_FRAMES3_CURRENT3_YOYO2 = 0,
    AD_PERIOD0_PERIOD1 = 1,
    AD_PERIOD2_PERIOD3 = 2,
    AD_PQUADDEF_LO = 3,
    AD_PQUADDEF_HI = 4,

    PLR_ROOMID = 0,
    PLR_X_LO = 1,
    PLR_X_HI = 2,
    PLR_Y_LO = 3,
    PLR_Y_HI = 4,
    PLR_DX = 5,
    PLR_DY = 6,
    PLR_HSHIFT = 7,
    PLR_VSHIFT = 8,
    PLR_PCORNER_LO = 9,
    PLR_PCORNER_HI = 10,
    PLR_CLEANUP = 11,
    PLR_PVIZDEF_LO = 12,
    PLR_PVIZDEF_HI = 13,
    PLR_ELAPSED = 14,
    PLR_HP = 15,
    PLR_MP = 16,
    PLR_PINV0_LO = 17,
    PLR_PINV0_HI = 18,
    PLR_PINV1_LO = 19,
    PLR_PINV1_HI = 20,
    PLR_PINV2_LO = 21,
    PLR_PINV2_HI = 22,

    CAM_ROOMID = 0,
    CAM_PLITEMAP_LO = 1,
    CAM_PLITEMAP_HI = 2,
    CAM_NMOVS = 3,

    PLRDIR_N = 0,
    PLRDIR_E = 1,
    PLRDIR_S = 2,
    PLRDIR_W = 3,

    SETYPE_DOORLOCKED = 0,
    SETYPE_PICKUP = 1
};

typedef struct {
    uint8_t q;
    uint8_t r;
} FloorResults;

extern uint8_t beebram[0x8000];

extern uint8_t reversed_bytes[256];

FloorResults floordiv(int a, int b);

#endif