#include "player.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>

void movePlayer() {

    // get current bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8), x1 = x0;
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8), y1 = y0;

checkx:
    if (dir_x != DIR_ZERO) {
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++; // potential update
        uint8_t i1 = y1 >> 3;
        uint8_t j1 = x1 >> 3;
        uint8_t ilimit = ((y1 & 0b111) == 0) ? 2 : 3;

        for (uint8_t i = i1; i < (i1 + ilimit); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    uint8_t h1 = x1 & 0b111;
                    if (h1 != 0) // if hshift not 0, an edge case
                        x1 = x0; // cancel update
                    goto checky;
                }
                if (tid >= TID_WALL_CRATE && tid < TID_WALL_CRATE + 4) {
                    uint8_t h1 = x1 & 0b111;
                    if (h1 != 0)
                        x1 = x0; // cancel update
                    goto checky;
                }
            }
        }
    }

checky:
    if (dir_y != DIR_ZERO) {
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++; // potential update
        uint8_t i1 = y1 >> 3;
        uint8_t j1 = x1 >> 3;
        uint8_t jlimit = ((x1 & 0b111) == 0) ? 2 : 3;

        // GOING DOWN
        if (dir_y == DIR_POSITIVE || dir_y == DIR_NEGATIVE) {
            for (uint8_t i = i1; i < (i1 + 3); i++) {
                for (uint8_t j = j1; j < (j1 + jlimit); j++) {
                    uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                    if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                        uint8_t v1 = y1 & 0b111;
                        if (v1 != 0) // if hshift not 0, an edge case
                            y1 = y0;
                        goto save;
                    }
                    if (tid >= TID_WALL_CRATE && tid < TID_WALL_CRATE + 4) {
                        uint8_t v1 = y1 & 0b111;
                        if (v1 != 0) // if hshift not 0, an edge case
                            y1 = y0;
                        goto save;
                    }
                }
            }
        }
    }

save:
    // save the updated coordinates
    beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
    beebram[PLAYER + ME_X_HI] = x1 >> 8;
    beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
    beebram[PLAYER + ME_Y_HI] = y1 >> 8;

    // update the sprite container with its new coordinates
    updateSpriteContainer(PLAYER);

    // raise the redraw flag to let renderer know movement has taken place
    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
}

// void movePlayer() {

//     // get current bearing
//     uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
//     uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

//     // get the current coordinates
//     uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8), x1 = x0;
//     uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8), y1 = y0;

// checkx:
//     if (dir_x != DIR_ZERO) {
//         (dir_x == DIR_NEGATIVE) ? x1-- : x1++; // potential update
//         uint8_t i1 = y1 >> 3;
//         uint8_t j1 = x1 >> 3;

//         for (uint8_t i = i1; i < (i1 + 3); i++) {
//             for (uint8_t j = j1; j < (j1 + 3); j++) {
//                 uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
//                 if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
//                     uint8_t h1 = x1 & 0b111;
//                     if (h1 != 0) // if hshift not 0, an edge case
//                         x1 = x0; // cancel update
//                     goto checky;
//                 }
//                 if (tid >= TID_WALL_CRATE && tid < TID_WALL_CRATE + 4) {
//                     uint8_t h1 = x1 & 0b111;
//                     if (h1 != 0)
//                         x1 = x0; // cancel update
//                     goto checky;
//                 }
//             }
//         }
//     }

// checky:
//     if (dir_y != DIR_ZERO) {
//         (dir_y == DIR_NEGATIVE) ? y1-- : y1++; // potential update
//         uint8_t i1 = y1 >> 3;
//         uint8_t j1 = x1 >> 3;
//         uint8_t jlimit = ((x1 & 0b111) == 0) ? 2 : 3;

//         for (uint8_t i = i1; i < (i1 + 3); i++) {
//             for (uint8_t j = j1; j < (j1 + jlimit); j++) {
//                 uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
//                 if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
//                     y1 = y0;
//                     goto save;
//                 }
//                 if (tid >= TID_WALL_CRATE && tid < TID_WALL_CRATE + 4) {
//                     y1 = y0;
//                     goto save;
//                 }
//             }
//         }
//     }

// save:
//     // save the updated coordinates
//     beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
//     beebram[PLAYER + ME_X_HI] = x1 >> 8;
//     beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
//     beebram[PLAYER + ME_Y_HI] = y1 >> 8;

//     // update the sprite container with its new coordinates
//     updateSpriteContainer(PLAYER);

//     // raise the redraw flag to let renderer know movement has taken place
//     beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
//     beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
// }

/*----------------------------------------------------------------------------*/

void _movePlayer() {

    // get current bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8), x1 = x0;
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8), y1 = y0;

checkx:
    if (dir_x != DIR_ZERO) {
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++; // potential update
        uint8_t i1 = y1 >> 3;
        uint8_t j1 = x1 >> 3;

        for (uint8_t i = i1; i < (i1 + 3); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    x1 = x0; // cancel update
                    goto checky;
                }
                if (tid >= TID_WALL_CRATE && tid < TID_WALL_CRATE + 4) {
                    x1 = x0; // cancel update
                    goto checky;
                }
            }
        }
    }

checky:
    if (dir_y != DIR_ZERO) {
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++; // potential update
        uint8_t i1 = y1 >> 3;
        uint8_t j1 = x1 >> 3;

        for (uint8_t i = i1; i < (i1 + 3); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    y1 = y0;
                    goto save;
                }
                if (tid >= TID_WALL_CRATE && tid < TID_WALL_CRATE + 4) {
                    y1 = y0;
                    goto save;
                }
            }
        }
    }

save:
    // save the updated coordinates
    beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
    beebram[PLAYER + ME_X_HI] = x1 >> 8;
    beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
    beebram[PLAYER + ME_Y_HI] = y1 >> 8;

    // update the sprite container with its new coordinates
    updateSpriteContainer(PLAYER);

    // raise the redraw flag to let renderer know movement has taken place
    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
}
