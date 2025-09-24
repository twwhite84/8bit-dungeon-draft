#include "shared.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static void initTextures();
static void initQuads();
static void initTilemaps();
static void initStaticEnts();
static void initAnimdefs();
static void initPlayer();

/*------------------------------ MEMORY OFFSETS ------------------------------*/

// TEXTURES 0x2300 to 0x2C00
// these can be stored at any position provided masks immediately follow defs
static const int tTiled0 = TEXTURES + 8 * 0; // Q0
static const int tMesh0 = TEXTURES + 8 * 1;  // Q1
static const int tMesh1 = TEXTURES + 8 * 2;
static const int tMesh2 = TEXTURES + 8 * 3;
static const int tTread0 = TEXTURES + 8 * 4;   // Q2
static const int tDiamond0 = TEXTURES + 8 * 5; // Q3
static const int tDirt0 = TEXTURES + 8 * 6;    // Q4
static const int tDirt1 = TEXTURES + 8 * 7;
static const int tDirt2 = TEXTURES + 8 * 8;
static const int tDirt3 = TEXTURES + 8 * 9;
static const int tSquare0 = TEXTURES + 8 * 10; // Q5
static const int tSquare1 = TEXTURES + 8 * 11;
static const int tSquare2 = TEXTURES + 8 * 12;
static const int tSquare3 = TEXTURES + 8 * 13;
static const int tCrate0 = TEXTURES + 8 * 14; // Q6
static const int tCrate1 = TEXTURES + 8 * 15;
static const int tCrate2 = TEXTURES + 8 * 16;
static const int tCrate3 = TEXTURES + 8 * 17;
static const int tDoor0 = TEXTURES + 8 * 18; // Q32
static const int tDoor1 = TEXTURES + 8 * 19;
static const int tDoor2 = TEXTURES + 8 * 20;
static const int tDoor3 = TEXTURES + 8 * 21;
static const int tDoorLocked0 = TEXTURES + 8 * 22; // Q33
static const int tDoorLocked1 = TEXTURES + 8 * 23;
static const int tDoorLocked2 = TEXTURES + 8 * 24;
static const int tDoorLocked3 = TEXTURES + 8 * 25;
static const int tForceField00 = TEXTURES + 8 * 26; // Q34
static const int tForceField01 = TEXTURES + 8 * 27;
static const int tForceField10 = TEXTURES + 8 * 28; // Q35
static const int tForceField11 = TEXTURES + 8 * 29;

// def-mask pairs
static const int tBall0 = TEXTURES + 8 * 30; // Q48
static const int tBall2 = TEXTURES + 8 * 31;
static const int tBallMask0 = TEXTURES + 8 * 32; // Q49
static const int tBallMask2 = TEXTURES + 8 * 33;

static const int tDogIdleD_0 = TEXTURES + 8 * 34; // Q50
static const int tDogIdleD_2 = TEXTURES + 8 * 35;
static const int tDogIdleMaskD_0 = TEXTURES + 8 * 36; // Q51
static const int tDogIdleMaskD_2 = TEXTURES + 8 * 37;

static const int tDogWalkD_0 = TEXTURES + 8 * 38; // Q52
static const int tDogWalkD_2 = TEXTURES + 8 * 39;
static const int tDogWalkD_3 = TEXTURES + 8 * 40;

static const int tDogWalkMaskD_0 = TEXTURES + 8 * 41; // Q53
static const int tDogWalkMaskD_2 = TEXTURES + 8 * 42;
static const int tDogWalkMaskD_3 = TEXTURES + 8 * 43;

static const int tDogIdleU_0 = TEXTURES + 8 * 44;
static const int tDogIdleU_2 = TEXTURES + 8 * 45;
static const int tDogIdleMaskU_0 = TEXTURES + 8 * 46;
static const int tDogIdleMaskU_2 = TEXTURES + 8 * 47;

static const int tDogWalkU_0 = TEXTURES + 8 * 48;
static const int tDogWalkU_2 = TEXTURES + 8 * 49;
static const int tDogWalkU_3 = TEXTURES + 8 * 50;
static const int tDogWalkMaskU_0 = TEXTURES + 8 * 51;
static const int tDogWalkMaskU_2 = TEXTURES + 8 * 52;
static const int tDogWalkMaskU_3 = TEXTURES + 8 * 53;

static const int tDogIdleR_0 = TEXTURES + 8 * 54;
static const int tDogIdleR_1 = TEXTURES + 8 * 55;
static const int tDogIdleR_2 = TEXTURES + 8 * 56;
static const int tDogIdleR_3 = TEXTURES + 8 * 57;
static const int tDogIdleMaskR_0 = TEXTURES + 8 * 58;
static const int tDogIdleMaskR_1 = TEXTURES + 8 * 59;
static const int tDogIdleMaskR_2 = TEXTURES + 8 * 60;
static const int tDogIdleMaskR_3 = TEXTURES + 8 * 61;

static const int tDogWalkRF0_0 = TEXTURES + 8 * 62;
static const int tDogWalkRF0_1 = TEXTURES + 8 * 63;
static const int tDogWalkRF0_2 = TEXTURES + 8 * 64;
static const int tDogWalkRF0_3 = TEXTURES + 8 * 65;
static const int tDogWalkMaskRF0_0 = TEXTURES + 8 * 66;
static const int tDogWalkMaskRF0_1 = TEXTURES + 8 * 67;
static const int tDogWalkMaskRF0_2 = TEXTURES + 8 * 68;
static const int tDogWalkMaskRF0_3 = TEXTURES + 8 * 69;

static const int tDogWalkRF2_0 = TEXTURES + 8 * 70;
static const int tDogWalkRF2_1 = TEXTURES + 8 * 71;
static const int tDogWalkRF2_2 = TEXTURES + 8 * 72;
static const int tDogWalkRF2_3 = TEXTURES + 8 * 73;
static const int tDogWalkMaskRF2_0 = TEXTURES + 8 * 74;
static const int tDogWalkMaskRF2_1 = TEXTURES + 8 * 75;
static const int tDogWalkMaskRF2_2 = TEXTURES + 8 * 76;
static const int tDogWalkMaskRF2_3 = TEXTURES + 8 * 77;

// QUADS
// these quads correspond to 0-31 tilemap ids (32 of these)
static const int qTiled = QUADDEFS + 8 * 0;
static const int qMesh = QUADDEFS + 8 * 1;
static const int qTread = QUADDEFS + 8 * 2;
static const int qDiamond = QUADDEFS + 8 * 3;
static const int qDirt = QUADDEFS + 8 * 4;
static const int qSquare = QUADDEFS + 8 * 5;
static const int qCrate = QUADDEFS + 8 * 6;

// these quads correspond to 32-47 object textures (16 of these)
static const int qDoor = QUADDEFS + 8 * 32;
static const int qDoorLocked = QUADDEFS + 8 * 33;
static const int qForceField0 = QUADDEFS + 8 * 34;
static const int qForceField1 = QUADDEFS + 8 * 35;

// these quads correspond to 48-95 composite pairs (24 of these)
static const int qBall = QUADDEFS + 8 * 48;
static const int qBallMask = QUADDEFS + 8 * 49;

// dog d
static const int qDogIdleD = QUADDEFS + 8 * 50;
static const int qDogIdleMaskD = QUADDEFS + 8 * 51;

static const int qDogWalkDF0 = QUADDEFS + 8 * 52;
static const int qDogWalkMaskDF0 = QUADDEFS + 8 * 53;
static const int qDogWalkDF2 = QUADDEFS + 8 * 54;
static const int qDogWalkMaskDF2 = QUADDEFS + 8 * 55;

// dog u
static const int qDogIdleU = QUADDEFS + 8 * 56;
static const int qDogIdleMaskU = QUADDEFS + 8 * 57;

static const int qDogWalkUF0 = QUADDEFS + 8 * 58;
static const int qDogWalkMaskUF0 = QUADDEFS + 8 * 59;
static const int qDogWalkUF2 = QUADDEFS + 8 * 60;
static const int qDogWalkMaskUF2 = QUADDEFS + 8 * 61;

// dog r
static const int qDogIdleR = QUADDEFS + 8 * 62;
static const int qDogIdleMaskR = QUADDEFS + 8 * 63;

static const int qDogWalkRF0 = QUADDEFS + 8 * 64;
static const int qDogWalkMaskRF0 = QUADDEFS + 8 * 65;
static const int qDogWalkRF1 = QUADDEFS + 8 * 66;
static const int qDogWalkMaskRF1 = QUADDEFS + 8 * 67;
static const int qDogWalkRF2 = QUADDEFS + 8 * 68;
static const int qDogWalkMaskRF2 = QUADDEFS + 8 * 69;

// dog l
static const int qDogIdleL = QUADDEFS + 8 * 70;
static const int qDogIdleMaskL = QUADDEFS + 8 * 71;

static const int qDogWalkLF0 = QUADDEFS + 8 * 72;
static const int qDogWalkMaskLF0 = QUADDEFS + 8 * 73;
static const int qDogWalkLF1 = QUADDEFS + 8 * 74;
static const int qDogWalkMaskLF1 = QUADDEFS + 8 * 75;
static const int qDogWalkLF2 = QUADDEFS + 8 * 76;
static const int qDogWalkMaskLF2 = QUADDEFS + 8 * 77;

// ANIMDEFS 32 of these
static const int aForceField = ANIMDEFS + 2 * 0;
static const int aDogWalkD = ANIMDEFS + 2 * 1;
static const int aDogWalkU = ANIMDEFS + 2 * 2;
static const int aDogWalkR = ANIMDEFS + 2 * 3;
static const int aDogWalkL = ANIMDEFS + 2 * 4;

// TILEMAPS
static const int map1 = TILEMAPS + 0x20 + 0;
static const int map2 = TILEMAPS + 0x20 + 90;

/*----------------------------------------------------------------------------*/

void init_ram() {
    memset(beebram, 0, sizeof(beebram));
    initTextures();
    initQuads();
    initTilemaps();
    initStaticEnts();
    initAnimdefs();
    initPlayer();
}

/*----------------------------------------------------------------------------*/

static void initTextures() {
    memcpy(&beebram[tTiled0], (uint8_t[]){0XD4, 0XAA, 0XD4, 0XAA, 0XD4, 0XAA, 0XFE, 0X00}, 8);
    memcpy(&beebram[tMesh0], (uint8_t[]){0x80, 0x41, 0x22, 0x14, 0x08, 0x10, 0x22, 0x41}, 8);
    memcpy(&beebram[tMesh1], (uint8_t[]){0x80, 0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41}, 8);
    memcpy(&beebram[tMesh2], (uint8_t[]){0x80, 0x40, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41}, 8);
    memcpy(&beebram[tTread0], (uint8_t[]){0x00, 0x20, 0x40, 0x80, 0x00, 0x08, 0x04, 0x02}, 8);
    memcpy(&beebram[tDiamond0], (uint8_t[]){0x10, 0x28, 0x54, 0xAA, 0x54, 0x28, 0x10, 0x00}, 8);
    memcpy(&beebram[tDirt0], (uint8_t[]){0x08, 0x00, 0x00, 0x10, 0x80, 0x00, 0x01, 0x20}, 8);
    memcpy(&beebram[tDirt1], (uint8_t[]){0x00, 0x22, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00}, 8);
    memcpy(&beebram[tDirt2], (uint8_t[]){0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01}, 8);
    memcpy(&beebram[tDirt3], (uint8_t[]){0x11, 0x00, 0x00, 0x02, 0x80, 0x08, 0x00, 0x00}, 8);
    memcpy(&beebram[tSquare0], (uint8_t[]){0x7F, 0xBF, 0xC0, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF}, 8);
    memcpy(&beebram[tSquare1], (uint8_t[]){0xFE, 0xFC, 0x00, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8}, 8);
    memcpy(&beebram[tSquare2], (uint8_t[]){0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xC0, 0x80, 0x00}, 8);
    memcpy(&beebram[tSquare3], (uint8_t[]){0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0x00, 0x00, 0x00}, 8);
    memcpy(&beebram[tCrate0], (uint8_t[]){0xFF, 0xBF, 0xC0, 0xD8, 0xDC, 0xCE, 0xC7, 0xC2}, 8);
    memcpy(&beebram[tCrate1], (uint8_t[]){0xFE, 0xFA, 0x06, 0x36, 0x76, 0xE6, 0xC6, 0x86}, 8);
    memcpy(&beebram[tCrate2], (uint8_t[]){0xC7, 0xCE, 0xDC, 0xF8, 0xF0, 0xBF, 0xFF, 0x00}, 8);
    memcpy(&beebram[tCrate3], (uint8_t[]){0xC6, 0xE6, 0x76, 0x3E, 0x1E, 0xFA, 0xFE, 0x00}, 8);
    memcpy(&beebram[tDoor0], (uint8_t[]){0X55, 0X28, 0X55, 0X28, 0X54, 0X2B, 0X55, 0X2A}, 8);
    memcpy(&beebram[tDoor1], (uint8_t[]){0X54, 0XAA, 0X54, 0XAA, 0X14, 0XAA, 0X14, 0XAA}, 8);
    memcpy(&beebram[tDoor2], (uint8_t[]){0X55, 0X2A, 0X55, 0X2A, 0X54, 0X29, 0X55, 0X28}, 8);
    memcpy(&beebram[tDoor3], (uint8_t[]){0X14, 0XAA, 0X14, 0XAA, 0X14, 0XEA, 0X54, 0XAA}, 8);
    memcpy(&beebram[tDoorLocked0], (uint8_t[]){0x55, 0x28, 0x55, 0x28, 0x54, 0x3B, 0x67, 0x42}, 8);
    memcpy(&beebram[tDoorLocked1], (uint8_t[]){0x54, 0xAA, 0x54, 0xAA, 0x14, 0xAA, 0x14, 0xAA}, 8);
    memcpy(&beebram[tDoorLocked2], (uint8_t[]){0x43, 0x66, 0x65, 0x26, 0x7C, 0x29, 0x55, 0x28}, 8);
    memcpy(&beebram[tDoorLocked3], (uint8_t[]){0x14, 0xAA, 0x14, 0xAA, 0x14, 0xEA, 0x54, 0xAA}, 8);
    memcpy(&beebram[tForceField00], (uint8_t[]){0x00, 0x0C, 0x12, 0x21, 0xCC, 0x12, 0x21, 0xCC}, 8);
    memcpy(&beebram[tForceField01], (uint8_t[]){0x12, 0x21, 0xCC, 0x12, 0x21, 0xC0, 0x00, 0x00}, 8);
    memcpy(&beebram[tForceField10], (uint8_t[]){0x00, 0x00, 0x03, 0x84, 0x48, 0x33, 0x84, 0x48}, 8);
    memcpy(&beebram[tForceField11], (uint8_t[]){0x33, 0x84, 0x48, 0x33, 0x84, 0x48, 0x30, 0x00}, 8);

    // masked textures
    memcpy(&beebram[tBall0], (uint8_t[]){0x00, 0x00, 0x00, 0x03, 0x0F, 0x0F, 0x1F, 0x1F}, 8);
    memcpy(&beebram[tBall2], (uint8_t[]){0x0F, 0x17, 0x0A, 0x05, 0x02, 0x00, 0x00, 0x00}, 8);
    memcpy(&beebram[tBallMask0], (uint8_t[]){0x00, 0x00, 0x03, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F}, 8);
    memcpy(&beebram[tBallMask2], (uint8_t[]){0x3F, 0x3F, 0x1F, 0x1F, 0x0F, 0x03, 0x00, 0x00}, 8);

    memcpy(&beebram[tDogIdleD_0], (uint8_t[]){0x00, 0x00, 0x03, 0x07, 0x09, 0x0F, 0x1D, 0x34}, 8);
    memcpy(&beebram[tDogIdleD_2], (uint8_t[]){0x36, 0x03, 0x08, 0x18, 0x17, 0x04, 0x06, 0x00}, 8);
    memcpy(&beebram[tDogIdleMaskD_0], (uint8_t[]){0x00, 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x7F}, 8);
    memcpy(&beebram[tDogIdleMaskD_2], (uint8_t[]){0x7F, 0x3F, 0x1F, 0x3F, 0x3F, 0x3F, 0x0F, 0x07}, 8);

    memcpy(&beebram[tDogWalkD_0], (uint8_t[]){0x00, 0x00, 0x00, 0x03, 0x07, 0x09, 0x3F, 0x7D}, 8);
    memcpy(&beebram[tDogWalkD_2], (uint8_t[]){0x64, 0x06, 0x0B, 0x0C, 0x00, 0x06, 0x00, 0x00}, 8);
    memcpy(&beebram[tDogWalkD_3], (uint8_t[]){0x26, 0x60, 0xC0, 0x08, 0x00, 0x60, 0x60, 0x00}, 8);
    memcpy(&beebram[tDogWalkMaskD_0], (uint8_t[]){0x00, 0x00, 0x03, 0x07, 0x0F, 0x3F, 0x7F, 0xFF}, 8);
    memcpy(&beebram[tDogWalkMaskD_2], (uint8_t[]){0xFF, 0x6F, 0x1F, 0x1F, 0x1F, 0x0F, 0x0F, 0x07}, 8);
    memcpy(&beebram[tDogWalkMaskD_3], (uint8_t[]){0xFF, 0xF6, 0xF8, 0xFC, 0xFC, 0xF0, 0xF0, 0xF0}, 8);

    memcpy(&beebram[tDogIdleU_0], (uint8_t[]){0x00, 0x00, 0x03, 0x07, 0x0F, 0x0F, 0x1F, 0x2F}, 8);
    memcpy(&beebram[tDogIdleU_2], (uint8_t[]){0x27, 0x03, 0x08, 0x18, 0x11, 0x07, 0x06, 0x00}, 8);
    memcpy(&beebram[tDogIdleMaskU_0], (uint8_t[]){0x00, 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x7F}, 8);
    memcpy(&beebram[tDogIdleMaskU_2], (uint8_t[]){0x7F, 0x2F, 0x1F, 0x3F, 0x3F, 0x1F, 0x0F, 0x07}, 8);

    memcpy(&beebram[tDogWalkU_0], (uint8_t[]){0x00, 0x00, 0x00, 0x03, 0x07, 0x0F, 0x3F, 0x7F}, 8);
    memcpy(&beebram[tDogWalkU_2], (uint8_t[]){0x6F, 0x07, 0x03, 0x08, 0x0C, 0x0B, 0x00, 0x00}, 8);
    memcpy(&beebram[tDogWalkU_3], (uint8_t[]){0xF6, 0xE0, 0xC0, 0x08, 0xC0, 0xC0, 0x60, 0x00}, 8);
    memcpy(&beebram[tDogWalkMaskU_0], (uint8_t[]){0x00, 0x00, 0x03, 0x07, 0x0F, 0x3F, 0x7F, 0xFF}, 8);
    memcpy(&beebram[tDogWalkMaskU_2], (uint8_t[]){0xFF, 0x7F, 0x0F, 0x1F, 0x1F, 0x1F, 0x0F, 0x07}, 8);
    memcpy(&beebram[tDogWalkMaskU_3], (uint8_t[]){0xFF, 0xFE, 0xFC, 0xFC, 0xFC, 0xF0, 0xF0, 0xF0}, 8);

    memcpy(&beebram[tDogIdleR_0], (uint8_t[]){0x00, 0x00, 0x03, 0x07, 0x0F, 0x0D, 0x1D, 0x1D}, 8);
    memcpy(&beebram[tDogIdleR_1], (uint8_t[]){0x00, 0x00, 0xC0, 0xE0, 0x90, 0xF0, 0xD0, 0xD0}, 8);
    memcpy(&beebram[tDogIdleR_2], (uint8_t[]){0x1D, 0x0B, 0x00, 0x00, 0x01, 0x0A, 0x03, 0x00}, 8);
    memcpy(&beebram[tDogIdleR_3], (uint8_t[]){0xE8, 0xD8, 0x00, 0x80, 0x80, 0x00, 0xC0, 0x00}, 8);
    memcpy(&beebram[tDogIdleMaskR_0], (uint8_t[]){0x00, 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F}, 8);
    memcpy(&beebram[tDogIdleMaskR_1], (uint8_t[]){0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC}, 8);
    memcpy(&beebram[tDogIdleMaskR_2], (uint8_t[]){0x3F, 0x1F, 0x0F, 0x07, 0x1F, 0x1F, 0x0F, 0x07}, 8);
    memcpy(&beebram[tDogIdleMaskR_3], (uint8_t[]){0xFC, 0xFC, 0xF8, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0}, 8);

    memcpy(&beebram[tDogWalkRF0_0], (uint8_t[]){0x00, 0x00, 0x03, 0x0F, 0x1F, 0x1D, 0x1D, 0x1D}, 8);
    memcpy(&beebram[tDogWalkRF0_1], (uint8_t[]){0x00, 0x00, 0xC0, 0xE0, 0x90, 0xF0, 0xD0, 0xD0}, 8);
    memcpy(&beebram[tDogWalkRF0_2], (uint8_t[]){0x01, 0x03, 0x00, 0x00, 0x08, 0x04, 0x06, 0x00}, 8);
    memcpy(&beebram[tDogWalkRF0_3], (uint8_t[]){0xE8, 0xD8, 0x00, 0xE0, 0x60, 0x10, 0x60, 0x00}, 8);
    memcpy(&beebram[tDogWalkMaskRF0_0], (uint8_t[]){0x00, 0x03, 0x0F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F}, 8);
    memcpy(&beebram[tDogWalkMaskRF0_1], (uint8_t[]){0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC}, 8);
    memcpy(&beebram[tDogWalkMaskRF0_2], (uint8_t[]){0x1F, 0x0F, 0x07, 0x1F, 0x1F, 0x0F, 0x0F, 0x07}, 8);
    memcpy(&beebram[tDogWalkMaskRF0_3], (uint8_t[]){0xFC, 0xFC, 0xF8, 0xF0, 0xF8, 0xF8, 0xF0, 0xE0}, 8);

    memcpy(&beebram[tDogWalkRF2_0], (uint8_t[]){0x00, 0x00, 0x03, 0x07, 0x0F, 0x0F, 0x1D, 0x1D}, 8);
    memcpy(&beebram[tDogWalkRF2_1], (uint8_t[]){0x00, 0x00, 0xC0, 0xE0, 0x90, 0xF0, 0xD0, 0xD0}, 8);
    memcpy(&beebram[tDogWalkRF2_2], (uint8_t[]){0x1D, 0x0D, 0x00, 0x07, 0x06, 0x00, 0x06, 0x00}, 8);
    memcpy(&beebram[tDogWalkRF2_3], (uint8_t[]){0xE8, 0xD8, 0x00, 0x00, 0x00, 0xD0, 0x60, 0x00}, 8);
    memcpy(&beebram[tDogWalkMaskRF2_0], (uint8_t[]){0x00, 0x03, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F}, 8);
    memcpy(&beebram[tDogWalkMaskRF2_1], (uint8_t[]){0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC}, 8);
    memcpy(&beebram[tDogWalkMaskRF2_2], (uint8_t[]){0x3F, 0x1F, 0x0F, 0x1F, 0x1F, 0x0F, 0x0F, 0x0F}, 8);
    memcpy(&beebram[tDogWalkMaskRF2_3], (uint8_t[]){0xFC, 0xFC, 0xF8, 0xE0, 0xF8, 0xF8, 0xF0, 0xE0}, 8);
}

/*----------------------------------------------------------------------------*/

static void initQuads() {
    memcpy(&beebram[qTiled], (uint16_t[]){tTiled0, tTiled0, tTiled0, tTiled0}, 8);
    memcpy(&beebram[qMesh], (uint16_t[]){tMesh0, tMesh1, tMesh2, tMesh0}, 8);
    memcpy(&beebram[qTread], (uint16_t[]){tTread0, tTread0, tTread0, tTread0}, 8);
    memcpy(&beebram[qDiamond], (uint16_t[]){tDiamond0, tDiamond0, tDiamond0, tDiamond0}, 8);
    memcpy(&beebram[qDirt], (uint16_t[]){tDirt0, tDirt1, tDirt2, tDirt3}, 8);
    memcpy(&beebram[qSquare], (uint16_t[]){tSquare0, tSquare1, tSquare2, tSquare3}, 8);
    memcpy(&beebram[qCrate], (uint16_t[]){tCrate0, tCrate1, tCrate2, tCrate3}, 8);
    memcpy(&beebram[qDoor], (uint16_t[]){tDoor0, tDoor1, tDoor2, tDoor3}, 8);
    memcpy(&beebram[qDoorLocked], (uint16_t[]){tDoorLocked0, tDoorLocked1, tDoorLocked2, tDoorLocked3}, 8);

    memcpy(&beebram[qBall], (uint16_t[]){tBall0, (tBall0 | 0x8000), tBall2, (tBall2 | 0x8000)}, 8);
    memcpy(&beebram[qBallMask], (uint16_t[]){tBallMask0, (tBallMask0 | 0x8000), tBallMask2, tBallMask2 | 0x8000}, 8);

    memcpy(&beebram[qForceField0], (uint16_t[]){tForceField00, tForceField00, tForceField01, tForceField01}, 8);
    memcpy(&beebram[qForceField1], (uint16_t[]){tForceField10, tForceField10, tForceField11, tForceField11}, 8);

    // dog d
    memcpy(&beebram[qDogIdleD], (uint16_t[]){tDogIdleD_0, (tDogIdleD_0 | 0x8000), tDogIdleD_2, (tDogIdleD_2 | 0x8000)}, 8);
    memcpy(&beebram[qDogIdleMaskD], (uint16_t[]){tDogIdleMaskD_0, tDogIdleMaskD_0, tDogIdleMaskD_2, tDogIdleMaskD_2}, 8);

    memcpy(&beebram[qDogWalkDF0], (uint16_t[]){tDogWalkD_0, tDogWalkD_0 | 0x8000, tDogWalkD_2, tDogWalkD_3}, 8);
    memcpy(&beebram[qDogWalkMaskDF0], (uint16_t[]){tDogWalkMaskD_0, tDogWalkMaskD_0 | 0x8000, tDogWalkMaskD_2, tDogWalkMaskD_3}, 8);
    memcpy(&beebram[qDogWalkDF2], (uint16_t[]){tDogWalkD_0, tDogWalkD_0 | 0x8000, tDogWalkD_3 | 0x8000, tDogWalkD_2 | 0x8000}, 8);
    memcpy(&beebram[qDogWalkMaskDF2], (uint16_t[]){tDogWalkMaskD_0, tDogWalkMaskD_0 | 0x8000, tDogWalkMaskD_3 | 0x8000, tDogWalkMaskD_2 | 0x8000}, 8);

    // dog u
    memcpy(&beebram[qDogIdleU], (uint16_t[]){tDogIdleU_0, (tDogIdleU_0 | 0x8000), tDogIdleU_2, (tDogIdleU_2 | 0x8000)}, 8);
    memcpy(&beebram[qDogIdleMaskU], (uint16_t[]){tDogIdleMaskU_0, tDogIdleMaskU_0, tDogIdleMaskU_2, tDogIdleMaskU_2}, 8);

    memcpy(&beebram[qDogWalkUF0], (uint16_t[]){tDogWalkU_0, tDogWalkU_0 | 0x8000, tDogWalkU_2, tDogWalkU_3}, 8);
    memcpy(&beebram[qDogWalkMaskUF0], (uint16_t[]){tDogWalkMaskU_0, tDogWalkMaskU_0 | 0x8000, tDogWalkMaskU_2, tDogWalkMaskU_3}, 8);
    memcpy(&beebram[qDogWalkUF2], (uint16_t[]){tDogWalkU_0, tDogWalkU_0 | 0x8000, tDogWalkU_3 | 0x8000, tDogWalkU_2 | 0x8000}, 8);
    memcpy(&beebram[qDogWalkMaskUF2], (uint16_t[]){tDogWalkMaskU_0, tDogWalkMaskU_0 | 0x8000, tDogWalkMaskU_3 | 0x8000, tDogWalkMaskU_2 | 0x8000}, 8);

    // dog r
    memcpy(&beebram[qDogIdleR], (uint16_t[]){tDogIdleR_0, tDogIdleR_1, tDogIdleR_2, tDogIdleR_3}, 8);
    memcpy(&beebram[qDogIdleMaskR], (uint16_t[]){tDogIdleMaskR_0, tDogIdleMaskR_1, tDogIdleMaskR_2, tDogIdleMaskR_3}, 8);

    memcpy(&beebram[qDogWalkRF0], (uint16_t[]){tDogWalkRF0_0, tDogWalkRF0_1, tDogWalkRF0_2, tDogWalkRF0_3}, 8);
    memcpy(&beebram[qDogWalkMaskRF0], (uint16_t[]){tDogWalkMaskRF0_0, tDogWalkMaskRF0_1, tDogWalkMaskRF0_2, tDogWalkMaskRF0_3}, 8);
    memcpy(&beebram[qDogWalkRF2], (uint16_t[]){tDogWalkRF2_0, tDogWalkRF2_1, tDogWalkRF2_2, tDogWalkRF2_3}, 8);
    memcpy(&beebram[qDogWalkMaskRF2], (uint16_t[]){tDogWalkMaskRF2_0, tDogWalkMaskRF2_1, tDogWalkMaskRF2_2, tDogWalkMaskRF2_3}, 8);

    // dog l
    memcpy(&beebram[qDogIdleL], (uint16_t[]){tDogIdleR_1 | 0x8000, tDogIdleR_0 | 0x8000, tDogIdleR_3 | 0x8000, tDogIdleR_2 | 0x8000}, 8);
    memcpy(&beebram[qDogIdleMaskL], (uint16_t[]){tDogIdleMaskR_1 | 0x8000, tDogIdleMaskR_0 | 0x8000, tDogIdleMaskR_3 | 0x8000, tDogIdleMaskR_2 | 0x8000}, 8);

    memcpy(&beebram[qDogWalkLF0], (uint16_t[]){tDogWalkRF0_1 | 0x8000, tDogWalkRF0_0 | 0x8000, tDogWalkRF0_3 | 0x8000, tDogWalkRF0_2 | 0x8000}, 8);
    memcpy(&beebram[qDogWalkMaskLF0], (uint16_t[]){tDogWalkMaskRF0_1 | 0x8000, tDogWalkMaskRF0_0 | 0x8000, tDogWalkMaskRF0_3 | 0x8000, tDogWalkMaskRF0_2 | 0x8000}, 8);
    memcpy(&beebram[qDogWalkLF2], (uint16_t[]){tDogWalkRF2_1 | 0x8000, tDogWalkRF2_0 | 0x8000, tDogWalkRF2_3 | 0x8000, tDogWalkRF2_2 | 0x8000}, 8);
    memcpy(&beebram[qDogWalkMaskLF2], (uint16_t[]){tDogWalkMaskRF2_1 | 0x8000, tDogWalkMaskRF2_0 | 0x8000, tDogWalkMaskRF2_3 | 0x8000, tDogWalkMaskRF2_2 | 0x8000}, 8);
}

/*----------------------------------------------------------------------------*/

static void initTilemaps() {
    // index supports up to 16 maps
    memcpy(&beebram[TILEMAPS + 0 * 2], (uint8_t[]){map1 & 0xFF, map1 >> 8}, 2);
    memcpy(&beebram[TILEMAPS + 1 * 2], (uint8_t[]){map2 & 0xFF, map2 >> 8}, 2);

    memcpy(&beebram[map1],
           (uint8_t[]){
               0x5a, 0x01, 0x1b, 0x4a, 0x12, 0x18, 0x6d, 0x28, 0x48, 0x61,
               0xb8, 0x86, 0x1b, 0x88, 0x62, 0x88, 0x63, 0x29, 0x06, 0x42,
               0x84, 0x86, 0x30, 0x44, 0x53, 0x08, 0xa1, 0x19, 0x0a, 0x12,
               0x18, 0x41, 0x30, 0x44, 0x43, 0x08, 0xa1, 0x19, 0x0a, 0x12,
               0x18, 0x41, 0x30, 0x84, 0x52, 0x84, 0x64, 0x28, 0x48, 0x61,
               0x20, 0xa1, 0x19, 0x0a, 0x12, 0x18, 0x48, 0x28, 0x46, 0x42,
               0x84, 0x86, 0x12, 0x0a, 0x11, 0x90, 0xa1, 0x21, 0x84, 0x13,
               0x08, 0x42, 0x30, 0x84, 0x12, 0x84, 0x64, 0x28, 0x48, 0x61,
               0x04, 0xc3, 0x10, 0x4c, 0x21, 0x04, 0xa1, 0x19, 0x0a, 0x12},
           (size_t)90);

    memcpy(&beebram[map2],
           (uint8_t[]){
               0x45, 0x01, 0x01, 0x0a, 0x50, 0xac, 0x04, 0x28, 0x4c, 0x11,
               0x0c, 0x2b, 0x01, 0x0a, 0x13, 0x08, 0x42, 0x0a, 0xc0, 0x42,
               0x84, 0x43, 0x28, 0x42, 0xb0, 0x10, 0xa1, 0x10, 0x4c, 0x22,
               0x84, 0x2b, 0x01, 0x04, 0x42, 0x84, 0x2b, 0x01, 0x04, 0x42,
               0x84, 0x2b, 0x01, 0x0a, 0x11, 0x0c, 0xa1, 0x0a, 0xc0, 0x42,
               0x84, 0x43, 0x28, 0x42, 0xb2, 0x94, 0x63, 0x2b, 0x06, 0xd2,
               0x84, 0x86, 0x1b, 0x4a, 0x12, 0x18, 0x6d, 0x28, 0x48},
           (size_t)69);
}

/*----------------------------------------------------------------------------*/

void initStaticEnts() {
    // (I,J) is 26x40 here, not 13x20
    uint16_t se_ptr = STATENTS, se_def = STATENTS + 0x60;

    // map01: locked door
    memcpy(&beebram[se_ptr], (uint8_t[]){se_def & 0xFF, se_def >> 8}, 2);
    memcpy(&beebram[se_def], (uint8_t[]){
                                 (0 << 3) | SETYPE_DOORLOCKED,        // ELAPSED_FRAMES (5) | TYPE (3)
                                 (1 << 6) | 1,                        // N_QUADS-1 (2) | ROOMID (6)
                                 (0 << 7) | 0,                        // REDRAW (1) | DATA (7)
                                 0, 0, 0,                             // DATA (24)
                                 4, 26,                               // I (8), J (8)
                                 qDoor & 0xFF, qDoor >> 8,            // PTR_VIZDEF (16)
                                 6, 26,                               // I (8), J (8)
                                 qDoorLocked & 0xFF, qDoorLocked >> 8 // PTR_VIZDEF (16)
                             },
           (size_t)14);
    se_ptr += 2;
    se_def += 14;

    // map01: ball pickup
    memcpy(&beebram[se_ptr], (uint8_t[]){se_def & 0xFF, se_def >> 8}, 2);
    memcpy(&beebram[se_def], (uint8_t[]){
                                 (0 << 3) | SETYPE_PICKUP, // ELAPSED_FRAMES (5) | TYPE (3)
                                 (0 << 6) | 1,             // N_QUADS-1 (2) | ROOMID (6)
                                 (0 << 7) | 0,             // REDRAW (1) | DATA (7)
                                 0, 0, 0,                  // DATA (24)
                                 10, 8,                    // I (8), J (8)
                                 qBall & 0xFF, qBall >> 8  // PTR_VIZDEF (16)
                             },
           (size_t)10);
    se_ptr += 2;
    se_def += 10;

    // map01: force field (animated)
    memcpy(&beebram[se_ptr], (uint8_t[]){se_def & 0xFF, se_def >> 8}, 2);
    memcpy(&beebram[se_def], (uint8_t[]){
                                 (0 << 3) | SETYPE_DOORLOCKED,           // ELAPSED_FRAMES (5) | TYPE (3)
                                 (2 << 6) | 1,                           // N_QUADS-1 (2) | ROOMID (6)
                                 (0 << 7) | 0,                           // REDRAW (1) | DATA (7)
                                 0, 0, 0,                                // DATA (24)
                                 8, 4,                                   // I (8), J (8)
                                 ADPTR_FFIELD & 0xFF, ADPTR_FFIELD >> 8, // PTR_VIZDEF (16)
                                 8, 6,                                   // I (8), J (8)
                                 ADPTR_FFIELD & 0xFF, ADPTR_FFIELD >> 8, // PTR_VIZDEF (16)
                                 8, 8,                                   // I (8), J (8)
                                 ADPTR_FFIELD & 0xFF, ADPTR_FFIELD >> 8, // PTR_VIZDEF (16)
                             },
           (size_t)18);
    se_ptr += 2;
    se_def += 18;

    // map01: dog (animated)
    memcpy(&beebram[se_ptr], (uint8_t[]){se_def & 0xFF, se_def >> 8}, 2);
    memcpy(&beebram[se_def], (uint8_t[]){
                                 (0 << 3) | SETYPE_PICKUP,                   // ELAPSED_FRAMES (5) | TYPE (3)
                                 (0 << 6) | 1,                               // N_QUADS-1 (2) | ROOMID (6)
                                 (0 << 7) | 0,                               // REDRAW (1) | DATA (7)
                                 0, 0, 0,                                    // DATA (24)
                                 18, 6,                                      // I (8), J (8)
                                 ADPTR_DOGWALKL & 0xFF, ADPTR_DOGWALKL >> 8, // PTR_VIZDEF (16)
                             },
           (size_t)10);
    se_ptr += 2;
    se_def += 10;

    return;
}

/*----------------------------------------------------------------------------*/

// aForceField
// aDogWalkD
// aDogWalkU
// aDogWalkR

void initAnimdefs() {

    // force field
    memcpy(&beebram[ADPTR_FFIELD], (uint8_t[]){
                                       (1 << 5) | (0 << 2) | 0,                // FRAMES-1 (3) | CURRENT (3) | YOYO (2)
                                       (15 << 4) | 15,                         // PERIOD_0 (4) | PERIOD_1 (4)
                                       (0 << 4) | 0,                           // PERIOD_2 (4) | PERIOD_3 (4)
                                       qForceField0 & 0xFF, qForceField0 >> 8, // PTR_QUAD (16)
                                       qForceField1 & 0xFF, qForceField1 >> 8, // PTR_QUAD (16)
                                   },
           (size_t)7);

    // dog_walk_up
    memcpy(&beebram[ADPTR_DOGWALKU], (uint8_t[]){
                                         (2 << 5) | (0 << 2) | 1,              // FRAMES-1 (3) | CURRENT (3) | YOYO (2)
                                         (8 << 4) | 3,                         // PERIOD_0 (4) | PERIOD_1 (4)
                                         (8 << 4) | 0,                         // PERIOD_2 (4) | PERIOD_3 (4)
                                         qDogWalkUF0 & 0xFF, qDogWalkUF0 >> 8, // PTR_QUAD (16)
                                         qDogIdleU & 0xFF, qDogIdleU >> 8,     // PTR_QUAD (16)
                                         qDogWalkUF2 & 0xFF, qDogWalkUF2 >> 8, // PTR_QUAD (16)
                                     },
           (size_t)9);

    // dog_walk_down
    memcpy(&beebram[ADPTR_DOGWALKD], (uint8_t[]){
                                         (2 << 5) | (0 << 2) | 1,              // FRAMES-1 (3) | CURRENT (3) | YOYO (2)
                                         (8 << 4) | 3,                         // PERIOD_0 (4) | PERIOD_1 (4)
                                         (8 << 4) | 0,                         // PERIOD_2 (4) | PERIOD_3 (4)
                                         qDogWalkDF0 & 0xFF, qDogWalkDF0 >> 8, // PTR_QUAD (16)
                                         qDogIdleD & 0xFF, qDogIdleD >> 8,     // PTR_QUAD (16)
                                         qDogWalkDF2 & 0xFF, qDogWalkDF2 >> 8, // PTR_QUAD (16)
                                     },
           (size_t)9);

    // dog_walk_left
    memcpy(&beebram[ADPTR_DOGWALKL], (uint8_t[]){
                                         (2 << 5) | (0 << 2) | 1,              // FRAMES-1 (3) | CURRENT (3) | YOYO (2)
                                         (8 << 4) | 3,                         // PERIOD_0 (4) | PERIOD_1 (4)
                                         (8 << 4) | 0,                         // PERIOD_2 (4) | PERIOD_3 (4)
                                         qDogWalkLF0 & 0xFF, qDogWalkLF0 >> 8, // PTR_QUAD (16)
                                         qDogIdleL & 0xFF, qDogIdleL >> 8,     // PTR_QUAD (16)
                                         qDogWalkLF2 & 0xFF, qDogWalkLF2 >> 8, // PTR_QUAD (16)
                                     },
           (size_t)9);

    // dog_walk_right
    memcpy(&beebram[ADPTR_DOGWALKR], (uint8_t[]){
                                         (2 << 5) | (0 << 2) | 1,              // FRAMES-1 (3) | CURRENT (3) | YOYO (2)
                                         (8 << 4) | 3,                         // PERIOD_0 (4) | PERIOD_1 (4)
                                         (8 << 4) | 0,                         // PERIOD_2 (4) | PERIOD_3 (4)
                                         qDogWalkRF0 & 0xFF, qDogWalkRF0 >> 8, // PTR_QUAD (16)
                                         qDogIdleR & 0xFF, qDogIdleR >> 8,     // PTR_QUAD (16)
                                         qDogWalkRF2 & 0xFF, qDogWalkRF2 >> 8, // PTR_QUAD (16)
                                     },
           (size_t)9);
}

/*----------------------------------------------------------------------------*/

// player
void initPlayer() {
    memset(&beebram[PLAYER], 0, (size_t)32);

    beebram[PLAYER + PLR_ROOM6_REDRAW2] |= (1 << 2); // room 1
    beebram[PLAYER + PLR_ROOM6_REDRAW2] |= true;
    beebram[PLAYER + PLR_X_LO] = 8;
    beebram[PLAYER + PLR_X_HI] = 0;
    beebram[PLAYER + PLR_Y_LO] = 8;
    beebram[PLAYER + PLR_Y_HI] = 0;
    beebram[PLAYER + PLR_PVIZDEF_LO] = ADPTR_DOGWALKR & 0xFF;
    beebram[PLAYER + PLR_PVIZDEF_HI] = ADPTR_DOGWALKR >> 8;
    beebram[PLAYER + PLR_ELAPSED6_CLEANUP2] |= 2; // cleanup 2 means unset
}
