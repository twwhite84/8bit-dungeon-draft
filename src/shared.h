#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

enum {
    // MEMORY SECTIONS
    LUT_REVERSE = 0x2200,
    TEXTURES = 0x2300,
    QUADS_PLAIN = 0x2C00,
    QUADS_COMP = 0x2D80,
    AD_TABLE = 0x2FA0,
    AD_DEFS = 0x2FC0,
    SE_TABLE = 0x3050,
    SE_DEFS = 0x30B0,
    PLAYER = 0x3340,
    MOVENTS = 0x3360,
    CAMERA = 0x3460,
    CAMBUFFER = 0x3480,
    OFFBUFFER = 0x3890,
    TMAP_TABLE = 0x38F0,
    TMAP_DEFS = 0x3950,
    LITEMAPS = 0x4E00,
    ROOMS = 0x5000,
    SCREEN = 0x5800,

    // ANIMDEF FIELDS & CONSTANTS
    ADF_FRAMES4_YOYO4 = 0x0,
    ADF_PERIODA4_PERIODB4 = 0x1,
    ADF_PERIODC4_PERIODD4 = 0x2,
    ADF_PFRAME_LO = 0x3,
    ADF_PFRAME_HI = 0x4,
    ADC_SET_WALKU = 0,
    ADC_SET_WALKD = 2,
    ADC_SET_WALKL = 4,
    ADC_SET_WALKR = 6,

    // COMMON ENTITY FIELDS
    CEF_ROOMID6_REDRAW2 = 0x0,
    CEF_FELAPSED5_FCURRENT3 = 0x1,
    CEF_I = 0x6,
    CEF_J = 0x7,
    CEF_PVIZBASE_LO = 0x8,
    CEF_PVIZBASE_HI = 0x9,
    CEC_REDRAW = 0b1,
    CEC_CLEAN = 0b10,

    // COMMON ME FIELDS
    MEF_X_LO = 0x2,
    MEF_X_HI = 0x3,
    MEF_Y_LO = 0x4,
    MEF_Y_HI = 0x5,
    MEF_ANIMSET = 0xA,
    MEF_XMD4_YMD4 = 0xB,
    MEF_HSHIFT4_VSHIFT4 = 0xC,
    MEF_OLDI = 0xD,
    MEF_OLDJ = 0xE,

    // PLAYER FIELDS
    PLRF_HP = 0xF,
    PLRF_MP = 0x10,
    PLRF_PINVA_LO = 0x11,
    PLRF_PINVA_HI = 0x12,
    PLRF_PINVB_LO = 0x13,
    PLRF_PINVB_HI = 0x14,
    PLRF_PINVC_LO = 0x15,
    PLRF_PINVC_HI = 0x16,

    // STATIC ENTITY FIELDS & CONSTANTS
    // I, J & VIZDEF REPEAT. APPLY DYNAMIC OFFSETTING WHEN USING
    SEF_TYPE4_NQUADS4 = 0x2,
    SEF_DATA24 = 0x3,
    SEC_SEMAX = 48,
    SEC_TYPE_DOORLOCKED = 0,
    SEC_TYPE_PICKUP = 1,

    // CAMERA FIELDS & CONSTANTS
    CAMF_ROOMID = 0x00,
    CAMF_REDRAW = 0x01, // XXXX | player | movables | statics | bg
    CAMF_PLITEMAP_LO = 0x02,
    CAMF_PLITEMAP_HI = 0x03,
    CAMF_PSE0_LO = 0x04,
    CAMF_PSE0_HI = 0x05,
    CAMF_PME0_LO = 0x18,
    CAMF_PME0_HI = 0x19,
    CAMF_PERASE_LO = 0x1E,
    CAMF_PERASE_HI = 0x1F,
    CAMC_SEMAX = 10,
    CAMC_MEMAX = 3,
    CAMC_REDRAW_BACKGROUND = 0b1,
    CAMC_REDRAW_STATICS = 0b10,
    CAMC_REDRAW_MOVABLES = 0b100,
    CAMC_REDRAW_PLAYER = 0b1000,
    _CAMC_STORED_ROWS = 13,
    _CAMC_STORED_COLUMNS = 20,
    CAMC_ROWS = (2 * _CAMC_STORED_ROWS),
    CAMC_COLUMNS = (2 * _CAMC_STORED_COLUMNS),
    CAMC_WIDTH = (8 * CAMC_COLUMNS),
    CAMC_HEIGHT = (8 * CAMC_ROWS),

    // TILE CODES (x4 in the cambuffer after upsizing)
    TID_FLOOR_TILED = 4 * 0,
    TID_FLOOR_MESH = 4 * 1,
    TID_FLOOR_TREAD = 4 * 2,
    TID_FLOOR_DIAMOND = 4 * 3,
    TID_FLOOR_DIRT = 4 * 4,
    TID_WALL_SQUARE = 4 * 5,
    TID_WALL_CRATE = 4 * 6,
    TID_WALLS = TID_WALL_SQUARE,

    // MISC CONSTANTS
    YOYO_OFF = 0,
    YOYO_FORWARD = 1,
    YOYO_BACKWARD = 2,

    DIR_UP = 0,
    DIR_DOWN = 1,
    DIR_LEFT = 2,
    DIR_RIGHT = 3,

    SENTINEL8 = 0xFF,
    SENTINEL16 = 0xFFFF,
};

typedef struct {
    uint8_t q;
    uint8_t r;
} FloorResults;

extern uint8_t beebram[0x8000];

FloorResults floordiv(int a, int b);

uint16_t getTileTextureAddr(uint8_t tid);

uint16_t ij2ramloc(uint8_t i, uint8_t j);

uint16_t ramloc2ij(uint16_t ramloc);

uint16_t xy2ramloc(uint16_t x, uint16_t y);

uint16_t xy2ij(uint16_t x, uint16_t y);

uint16_t ij2xy(uint8_t i, uint8_t j);

#endif