#include "shared.h"
#include <stddef.h>
#include <string.h>

void init_ram() {

    memset(beebram, 0, sizeof(beebram));

    /*-------------------- QUADS & TEXTURES --------------------*/

    // TILEMAP IDS GOTO QUADS, SO KEEP NON-TILEMAP TEXTURES HIGH
    const int qTiled = QUADDEFS + 8 * 0;
    const int qMesh = QUADDEFS + 8 * 1;
    const int qTread = QUADDEFS + 8 * 2;
    const int qDiamond = QUADDEFS + 8 * 3;
    const int qDirt = QUADDEFS + 8 * 4;
    const int qSquare = QUADDEFS + 8 * 5;
    const int qCrate = QUADDEFS + 8 * 6;
    const int qDoor = QUADDEFS + 8 * 31;
    const int qDoorLocked = QUADDEFS + 8 * 32;

    const int tTiled0 = TEXTURES + 8 * 0; // Q0
    const int tMesh0 = TEXTURES + 8 * 1;  // Q1
    const int tMesh1 = TEXTURES + 8 * 2;
    const int tMesh2 = TEXTURES + 8 * 3;
    const int tTread0 = TEXTURES + 8 * 4;   // Q2
    const int tDiamond0 = TEXTURES + 8 * 5; // Q3
    const int tDirt0 = TEXTURES + 8 * 6;    // Q4
    const int tDirt1 = TEXTURES + 8 * 7;
    const int tDirt2 = TEXTURES + 8 * 8;
    const int tDirt3 = TEXTURES + 8 * 9;
    const int tSquare0 = TEXTURES + 8 * 10; // Q5
    const int tSquare1 = TEXTURES + 8 * 11;
    const int tSquare2 = TEXTURES + 8 * 12;
    const int tSquare3 = TEXTURES + 8 * 13;
    const int tCrate0 = TEXTURES + 8 * 14; // Q6
    const int tCrate1 = TEXTURES + 8 * 15;
    const int tCrate2 = TEXTURES + 8 * 16;
    const int tCrate3 = TEXTURES + 8 * 17;
    const int tDoor0 = TEXTURES + 8 * 18; // Q7
    const int tDoor1 = TEXTURES + 8 * 19;
    const int tDoor2 = TEXTURES + 8 * 20;
    const int tDoor3 = TEXTURES + 8 * 21;
    const int tDoorLocked0 = TEXTURES + 8 * 22; // Q8
    const int tDoorLocked1 = TEXTURES + 8 * 23;
    const int tDoorLocked2 = TEXTURES + 8 * 24;
    const int tDoorLocked3 = TEXTURES + 8 * 25;

    // floor: tiled	[0,0,0,0]
    memcpy(&beebram[qTiled], (uint16_t[]){tTiled0, tTiled0, tTiled0, tTiled0}, 8);
    memcpy(&beebram[tTiled0], (uint8_t[]){0XD4, 0XAA, 0XD4, 0XAA, 0XD4, 0XAA, 0XFE, 0X00}, 8);

    // floor: mesh [0,1,2,0]
    memcpy(&beebram[qMesh], (uint16_t[]){tMesh0, tMesh1, tMesh2, tMesh0}, 8);
    memcpy(&beebram[tMesh0], (uint8_t[]){0x80, 0x41, 0x22, 0x14, 0x08, 0x10, 0x22, 0x41}, 8);
    memcpy(&beebram[tMesh1], (uint8_t[]){0x80, 0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41}, 8);
    memcpy(&beebram[tMesh2], (uint8_t[]){0x80, 0x40, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41}, 8);

    // floor: tread [0,0,0,0]
    memcpy(&beebram[qTread], (uint16_t[]){tTread0, tTread0, tTread0, tTread0}, 8);
    memcpy(&beebram[tTread0], (uint8_t[]){0x00, 0x20, 0x40, 0x80, 0x00, 0x08, 0x04, 0x02}, 8);

    // floor: diamond [0,0,0,0]
    memcpy(&beebram[qDiamond], (uint16_t[]){tDiamond0, tDiamond0, tDiamond0, tDiamond0}, 8);
    memcpy(&beebram[tDiamond0], (uint8_t[]){0x10, 0x28, 0x54, 0xAA, 0x54, 0x28, 0x10, 0x00}, 8);

    // floor: dirt [0,1,2,3]
    memcpy(&beebram[qDirt], (uint16_t[]){tDirt0, tDirt1, tDirt2, tDirt3}, 8);
    memcpy(&beebram[tDirt0], (uint8_t[]){0x08, 0x00, 0x00, 0x10, 0x80, 0x00, 0x01, 0x20}, 8);
    memcpy(&beebram[tDirt1], (uint8_t[]){0x00, 0x22, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00}, 8);
    memcpy(&beebram[tDirt2], (uint8_t[]){0x00, 0x04, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01}, 8);
    memcpy(&beebram[tDirt3], (uint8_t[]){0x11, 0x00, 0x00, 0x02, 0x80, 0x08, 0x00, 0x00}, 8);

    //  wall: square [0,1,2,3]
    memcpy(&beebram[qSquare], (uint16_t[]){tSquare0, tSquare1, tSquare2, tSquare3}, 8);
    memcpy(&beebram[tSquare0], (uint8_t[]){0x7F, 0xBF, 0xC0, 0xDF, 0xDF, 0xDF, 0xDF, 0xDF}, 8);
    memcpy(&beebram[tSquare1], (uint8_t[]){0xFE, 0xFC, 0x00, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8}, 8);
    memcpy(&beebram[tSquare2], (uint8_t[]){0xDF, 0xDF, 0xDF, 0xDF, 0xDF, 0xC0, 0x80, 0x00}, 8);
    memcpy(&beebram[tSquare3], (uint8_t[]){0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0x00, 0x00, 0x00}, 8);

    //  wall: crate [0,1,2,3]
    memcpy(&beebram[qCrate], (uint16_t[]){tCrate0, tCrate1, tCrate2, tCrate3}, 8);
    memcpy(&beebram[tCrate0], (uint8_t[]){0xFF, 0xBF, 0xC0, 0xD8, 0xDC, 0xCE, 0xC7, 0xC2}, 8);
    memcpy(&beebram[tCrate1], (uint8_t[]){0xFE, 0xFA, 0x06, 0x36, 0x76, 0xE6, 0xC6, 0x86}, 8);
    memcpy(&beebram[tCrate2], (uint8_t[]){0xC7, 0xCE, 0xDC, 0xF8, 0xF0, 0xBF, 0xFF, 0x00}, 8);
    memcpy(&beebram[tCrate3], (uint8_t[]){0xC6, 0xE6, 0x76, 0x3E, 0x1E, 0xFA, 0xFE, 0x00}, 8);

    //  door: closed [0,1,2,3]
    memcpy(&beebram[qDoor], (uint16_t[]){tDoor0, tDoor1, tDoor2, tDoor3}, 8);
    memcpy(&beebram[tDoor0], (uint8_t[]){0X55, 0X28, 0X55, 0X28, 0X54, 0X2B, 0X55, 0X2A}, 8);
    memcpy(&beebram[tDoor1], (uint8_t[]){0X54, 0XAA, 0X54, 0XAA, 0X14, 0XAA, 0X14, 0XAA}, 8);
    memcpy(&beebram[tDoor2], (uint8_t[]){0X55, 0X2A, 0X55, 0X2A, 0X54, 0X29, 0X55, 0X28}, 8);
    memcpy(&beebram[tDoor3], (uint8_t[]){0X14, 0XAA, 0X14, 0XAA, 0X14, 0XEA, 0X54, 0XAA}, 8);

    //  door: locked [0,1,2,3]
    memcpy(&beebram[qDoorLocked], (uint16_t[]){tDoorLocked0, tDoorLocked1, tDoorLocked2, tDoorLocked3}, 8);
    memcpy(&beebram[tDoorLocked0], (uint8_t[]){0x55, 0x28, 0x55, 0x28, 0x54, 0x3B, 0x67, 0x42}, 8);
    memcpy(&beebram[tDoorLocked1], (uint8_t[]){0x54, 0xAA, 0x54, 0xAA, 0x14, 0xAA, 0x14, 0xAA}, 8);
    memcpy(&beebram[tDoorLocked2], (uint8_t[]){0x43, 0x66, 0x65, 0x26, 0x7C, 0x29, 0x55, 0x28}, 8);
    memcpy(&beebram[tDoorLocked3], (uint8_t[]){0x14, 0xAA, 0x14, 0xAA, 0x14, 0xEA, 0x54, 0xAA}, 8);

    /*-------------------- TILEMAPS --------------------*/

    const int map0 = TILEMAPS + 0x20 + 0;
    const int map1 = TILEMAPS + 0x20 + 109;

    // index supports up to 32 maps
    memcpy(&beebram[TILEMAPS + 0 * 2], (uint8_t[]){map0 & 0xFF, map0 >> 8}, 2);
    memcpy(&beebram[TILEMAPS + 1 * 2], (uint8_t[]){map1 & 0xFF, map1 >> 8}, 2);

    memcpy(&beebram[map0],
           (uint8_t[]){0x6d, 0x01, 0x0c, 0xd1, 0x41, 0x10, 0x60, 0xcd, 0x14, 0x11,
                       0x06, 0x0c, 0xe1, 0x06, 0x0c, 0xe1, 0x06, 0x14, 0x20, 0xc3,
                       0x14, 0x40, 0xc4, 0x14, 0x11, 0x06, 0x18, 0x10, 0x85, 0x18,
                       0x21, 0x41, 0x0c, 0x41, 0x41, 0x10, 0x60, 0x81, 0x18, 0x10,
                       0x84, 0x18, 0x21, 0x41, 0x0c, 0x41, 0x41, 0x10, 0x60, 0x81,
                       0x18, 0x20, 0x85, 0x14, 0x10, 0xc4, 0x14, 0x11, 0x06, 0x08,
                       0x81, 0x41, 0x0c, 0x41, 0x41, 0x10, 0x60, 0x88, 0x14, 0x10,
                       0xc4, 0x14, 0x11, 0x06, 0x08, 0x81, 0x41, 0x0c, 0x41, 0x41,
                       0x10, 0x60, 0x81, 0x18, 0x20, 0x82, 0x18, 0x20, 0x81, 0x14,
                       0x10, 0xc4, 0x14, 0x11, 0x06, 0x08, 0x11, 0x83, 0x08, 0x11,
                       0x82, 0x08, 0x11, 0x41, 0x0c, 0x41, 0x41, 0x10, 0xf0},
           (size_t)109);

    memcpy(&beebram[map1],
           (uint8_t[]){0x54, 0x01, 0x00, 0x41, 0x45, 0x04, 0xb0, 0x04, 0x14, 0x11,
                       0x81, 0x08, 0x30, 0x4b, 0x00, 0x41, 0x41, 0x18, 0x20, 0x82,
                       0x04, 0xb0, 0x04, 0x14, 0x10, 0x83, 0x14, 0x10, 0x4b, 0x00,
                       0x41, 0x41, 0x08, 0x11, 0x82, 0x14, 0x10, 0x4b, 0x00, 0x40,
                       0x84, 0x14, 0x10, 0x4b, 0x00, 0x40, 0x84, 0x14, 0x10, 0x4b,
                       0x00, 0x41, 0x41, 0x08, 0x31, 0x41, 0x04, 0xb0, 0x04, 0x14,
                       0x10, 0x83, 0x14, 0x10, 0x4b, 0x14, 0x50, 0xc3, 0x14, 0xc0,
                       0xcd, 0x14, 0x11, 0x06, 0x0c, 0xd1, 0x41, 0x10, 0x60, 0xcd,
                       0x14, 0x11, 0x06, 0xfc},
           (size_t)84);

    return;
}