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
    SCREEN = 0x5800,

    // ANIMDEF FIELDS
    AD_FRAMES4_YOYO4 = 0x0,
    AD_PERIODA4_PERIODB4 = 0x1,
    AD_PERIODC4_PERIODD4 = 0x2,
    AD_PFRAME_LO = 0x3,
    AD_PFRAME_HI = 0x4,

    // COMMON ENTITY FIELDS
    CE_ROOMID6_REDRAW2 = 0x0,
    CE_FELAPSED5_FCURRENT3 = 0x1,
    CE_CONTAINER_I = 0x6,
    CE_CONTAINER_J = 0x7,
    CE_PVIZBASE_LO = 0x8,
    CE_PVIZBASE_HI = 0x9,

    // COMMON ME FIELDS
    ME_X_LO = 0x2,
    ME_X_HI = 0x3,
    ME_Y_LO = 0x4,
    ME_Y_HI = 0x5,
    ME_ANIMSET = 0xA,
    ME_DX4_DY4 = 0xB,
    ME_HSHIFT4_VSHIFT4 = 0xC,

    // ANIMATION SETS
    ANIMSET_WALKU = 0,
    ANIMSET_WALKD = 2,
    ANIMSET_WALKL = 4,
    ANIMSET_WALKR = 6,

    // PLAYER FIELDS
    PLR_HP = 0xD,
    PLR_MP = 0xE,
    PLR_PINVA_LO = 0xF,
    PLR_PINVA_HI = 0x10,
    PLR_PINVB_LO = 0x11,
    PLR_PINVB_HI = 0x12,
    PLR_PINVC_LO = 0x13,
    PLR_PINVC_HI = 0x14,

    // STATIC ENTITY FIELDS
    // I, J & VIZDEF REPEAT. APPLY DYNAMIC OFFSETTING WHEN USING
    SE_TYPE4_NQUADS4 = 0x2,
    SE_DATA24 = 0x3,

    // CAMERA FIELDS
    CAM_ROOMID = 0x00,
    CAM_PLITEMAP_LO = 0x01,
    CAM_PLITEMAP_HI = 0x02,
    CAM_PME0_LO = 0x04,
    CAM_PME0_HI = 0x05,
    CAM_PSE0_LO = 0x0C,
    CAM_PSE0_HI = 0x0D,

    // TILE CODES (x4 in the cambuffer after upsizing)
    TID_FLOOR_TILED = 4 * 0,
    TID_FLOOR_MESH = 4 * 1,
    TID_FLOOR_TREAD = 4 * 2,
    TID_FLOOR_DIAMOND = 4 * 3,
    TID_FLOOR_DIRT = 4 * 4,
    TID_WALL_SQUARE = 4 * 5,
    TID_WALL_CRATE = 4 * 6,

    // MISC CONSTANTS
    STORED_ROWS = 13,
    STORED_COLUMNS = 20,
    CAMERA_ROWS = (2 * STORED_ROWS),
    CAMERA_COLUMNS = (2 * STORED_COLUMNS),
    CAMERA_WIDTH = (8 * CAMERA_COLUMNS),
    CAMERA_HEIGHT = (8 * CAMERA_ROWS),

    PLRDIR_U = 1,
    PLRDIR_D = 2,
    PLRDIR_L = 3,
    PLRDIR_R = 4,

    SETYPE_DOORLOCKED = 0,
    SETYPE_PICKUP = 1,

    YOYO_OFF = 0,
    YOYO_FORWARD = 1,
    YOYO_BACKWARD = 2
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

#endif