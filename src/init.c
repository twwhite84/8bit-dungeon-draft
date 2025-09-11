#include "shared.h"
#include <stddef.h>
#include <string.h>

static void init_textures();
static void init_quads();
static void init_tilemaps();
static void init_entities();
static void init_animdefs();

/*------------------------------ MEMORY OFFSETS ------------------------------*/

// TEXTURES 0x2500 to 0x3300 (0xE00, or 3584 bytes)
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

static const int tBall0 = TEXTURES + 8 * 30; // Q48
static const int tBall1 = TEXTURES + 8 * 31;
static const int tBall2 = TEXTURES + 8 * 32;
static const int tBall3 = TEXTURES + 8 * 33;
static const int tBallMask0 = TEXTURES + 8 * 34; // Q49
static const int tBallMask1 = TEXTURES + 8 * 35;
static const int tBallMask2 = TEXTURES + 8 * 36;
static const int tBallMask3 = TEXTURES + 8 * 37;

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

// these quads correspond to 48-63 composite pairs (8 of these)
static const int qBall = QUADDEFS + 8 * 48;
static const int qBallMask = QUADDEFS + 8 * 49;

// ANIMDEFS
static const int aFField_idx = ANIMDEFS + 2 * 0;

// TILEMAPS
static const int map1 = TILEMAPS + 0x20 + 0;
static const int map2 = TILEMAPS + 0x20 + 90;

/*----------------------------------------------------------------------------*/

void init_ram() {
    memset(beebram, 0, sizeof(beebram));
    init_textures();
    init_quads();
    init_tilemaps();
    init_entities();
    init_animdefs();
}

/*----------------------------------------------------------------------------*/

static void init_textures() {
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

    // def-mask pairs must always go 4xdef, 4xmask
    memcpy(&beebram[tBall0], (uint8_t[]){0x00, 0x00, 0x00, 0x03, 0x0F, 0x0F, 0x1F, 0x1F}, 8);
    memcpy(&beebram[tBall1], (uint8_t[]){0x00, 0x00, 0x00, 0xC0, 0xF0, 0xF0, 0xF8, 0xF8}, 8);
    memcpy(&beebram[tBall2], (uint8_t[]){0x0F, 0x17, 0x0A, 0x05, 0x02, 0x00, 0x00, 0x00}, 8);
    memcpy(&beebram[tBall3], (uint8_t[]){0xF8, 0xD0, 0xA0, 0x50, 0x80, 0x00, 0x00, 0x00}, 8);
    memcpy(&beebram[tBallMask0], (uint8_t[]){0x00, 0x00, 0x03, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F}, 8);
    memcpy(&beebram[tBallMask1], (uint8_t[]){0x00, 0x00, 0xC0, 0xF0, 0xF8, 0xF8, 0xFC, 0xFC}, 8);
    memcpy(&beebram[tBallMask2], (uint8_t[]){0x3F, 0x3F, 0x1F, 0x1F, 0x0F, 0x03, 0x00, 0x00}, 8);
    memcpy(&beebram[tBallMask3], (uint8_t[]){0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xC0, 0x00, 0x00}, 8);
}

/*----------------------------------------------------------------------------*/

static void init_quads() {
    memcpy(&beebram[qTiled], (uint16_t[]){tTiled0, tTiled0, tTiled0, tTiled0}, 8);
    memcpy(&beebram[qMesh], (uint16_t[]){tMesh0, tMesh1, tMesh2, tMesh0}, 8);
    memcpy(&beebram[qTread], (uint16_t[]){tTread0, tTread0, tTread0, tTread0}, 8);
    memcpy(&beebram[qDiamond], (uint16_t[]){tDiamond0, tDiamond0, tDiamond0, tDiamond0}, 8);
    memcpy(&beebram[qDirt], (uint16_t[]){tDirt0, tDirt1, tDirt2, tDirt3}, 8);
    memcpy(&beebram[qSquare], (uint16_t[]){tSquare0, tSquare1, tSquare2, tSquare3}, 8);
    memcpy(&beebram[qCrate], (uint16_t[]){tCrate0, tCrate1, tCrate2, tCrate3}, 8);
    memcpy(&beebram[qDoor], (uint16_t[]){tDoor0, tDoor1, tDoor2, tDoor3}, 8);
    memcpy(&beebram[qDoorLocked], (uint16_t[]){tDoorLocked0, tDoorLocked1, tDoorLocked2, tDoorLocked3}, 8);
    memcpy(&beebram[qBall], (uint16_t[]){tBall0, tBall1, tBall2, tBall3}, 8);
    memcpy(&beebram[qBallMask], (uint16_t[]){tBallMask0, tBallMask1, tBallMask2, tBallMask3}, 8);
    memcpy(&beebram[qForceField0], (uint16_t[]){tForceField00, tForceField00, tForceField01, tForceField01}, 8);
    memcpy(&beebram[qForceField1], (uint16_t[]){tForceField10, tForceField10, tForceField11, tForceField11}, 8);
}

/*----------------------------------------------------------------------------*/

static void init_tilemaps() {
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

void init_entities() {
    // STATIC ENTITIES (0x40 is index length, then + previous entity size)
    // static const int seDoorLockedM1 = STATENTS + 0x40 + 0;
    // static const int seBallM1 = STATENTS + 0x40 + 6;

    // STATIC ENTITY TYPES
    // static const int DOOR_LOCKED = 0;
    // static const int PICKUP = 1;

    // INDEX
    uint16_t se_doorLockedM1 = STATENTS + 0x40;
    memcpy(&beebram[STATENTS + 0 * 2], (uint8_t[]){se_doorLockedM1 & 0xFF, se_doorLockedM1 >> 8}, 2);

    uint16_t se_ballM1 = se_doorLockedM1 + 10;
    memcpy(&beebram[STATENTS + 1 * 2], (uint8_t[]){se_ballM1 & 0xFF, se_ballM1 >> 8}, 2);

    uint16_t se_forceField01M1 = se_ballM1 + 6;
    memcpy(&beebram[STATENTS + 2 * 2], (uint8_t[]){se_forceField01M1 & 0xFF, se_forceField01M1 >> 8}, 2);

    // DEFS. I,J is in 26x40
    memcpy(&beebram[se_doorLockedM1], (uint8_t[]){
                                          (SE_DOORLOCKED << 4) | 2,            // TYPE (4) | SIZE (4)
                                          (0 << 6) | 1,                        // CURRENT_FRAME (2) | ROOMID (6)
                                          4, 26,                               // I (8), J (8)
                                          qDoor & 0xFF, qDoor >> 8,            // PTR_VIZDEF (16)
                                          6, 26,                               // I (8), J (8)
                                          qDoorLocked & 0xFF, qDoorLocked >> 8 // PTR_VIZDEF (16)
                                      },
           (size_t)10);

    memcpy(&beebram[se_ballM1], (uint8_t[]){
                                    (SE_PICKUP << 4) | 1,    // TYPE (4) | SIZE (4)
                                    (0 << 6) | 1,            // CURRENT_FRAME (2) | ROOMID(6)
                                    9, 8,                    // I (8), J (8)
                                    qBall & 0xFF, qBall >> 8 // PTR_VIZDEF (16)
                                },
           (size_t)6);

    memcpy(&beebram[se_forceField01M1], (uint8_t[]){
                                            (SE_DOORLOCKED << 4) | 3,             // TYPE (4) | SIZE (4)
                                            (0 << 6) | 1,                         // CURRENT_FRAME (2) | ROOMID(6)
                                            8, 4,                                 // I (8), J (8)
                                            aFField_idx & 0xFF, aFField_idx >> 8, // PTR_VIZDEF (16)
                                            8, 6,                                 // I (8), J (8)
                                            aFField_idx & 0xFF, aFField_idx >> 8, // PTR_VIZDEF (16)
                                            8, 8,                                 // I (8), J (8)
                                            aFField_idx & 0xFF, aFField_idx >> 8, // PTR_VIZDEF (16)
                                        },
           (size_t)14);
    return;
}

/*----------------------------------------------------------------------------*/

void init_animdefs() {
    // index: 2 byte pointers
    uint16_t aFField_def = ANIMDEFS + 0x40;
    memcpy(&beebram[aFField_idx], (uint8_t[]){aFField_def & 0xFF, aFField_def >> 8}, 2);

    // defs:
    memcpy(&beebram[aFField_def], (uint8_t[]){
                                      (2 << 6) | (0 << 2) | 0,                // FRAMES (2) | ELAPSED (4) | YOYO (2)
                                      (15 << 4) | 15,                         // PERIOD_0 (4) | PERIOD_1 (4)
                                      (0 << 4) | 0,                           // PERIOD_2 (4) | PERIOD_3 (4)
                                      qForceField0 & 0xFF, qForceField0 >> 8, // PTR_QUAD (16)
                                      qForceField1 & 0xFF, qForceField1 >> 8, // PTR_QUAD (16)
                                  },
           (size_t)7);
}
