#include "player.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>

bool _isWallCollision(uint8_t i, uint8_t j, uint8_t dir_x, uint8_t dir_y);

bool playerLook() {}

void OLDmovePlayer() {

    // get current bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8), x1 = x0;
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8), y1 = y0;
    uint8_t i0 = (y0 >> 3), i1 = i0;
    uint8_t j0 = (x0 >> 3), j1 = j0;

    bool x_blocked = false, y_blocked = false;

    if (dir_x != DIR_ZERO) {
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++;
        i1 = y1 >> 3;
        j1 = x1 >> 3;

        for (uint8_t i = i1; i < (i1 + 3); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    x_blocked = true;
                }
            }
        }
        if (x_blocked)
            x1 = x0;
    }

    // this block uses x1 that may have been altered in previous step
    if (dir_y != DIR_ZERO) {
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++;
        i1 = y1 >> 3;
        j1 = x1 >> 3;

        for (uint8_t i = i1; i < (i1 + 3); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    y_blocked = true;
                }
            }
        }
    }

    if (!x_blocked) {
        beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
        beebram[PLAYER + ME_X_HI] = x1 >> 8;
    }

    if (!y_blocked) {
        beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y1 >> 8;
    }

    // update the sprite container
    updateSpriteContainer(PLAYER);

    // raise the redraw flag to let renderer know movement has taken place
    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
}

/*----------------------------------------------------------------------------*/

void movePlayer() {

    // get current bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8), x1 = x0;
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8), y1 = y0;

    // CHECK X MOVEMENT
    if (dir_x != DIR_ZERO) {
        bool x_blocked = false;
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++; // potential update
        uint8_t i1 = y1 >> 3;
        uint8_t j1 = x1 >> 3;

        for (uint8_t i = i1; i < (i1 + 3); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    x_blocked = true;
                }
            }
        }
        if (x_blocked) // cancels potential update
            x1 = x0;
    }

    // CHECK Y MOVEMENT
    if (dir_y != DIR_ZERO) {
        bool y_blocked = false;
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++; // potential update
        uint8_t i1 = y1 >> 3;
        uint8_t j1 = x1 >> 3;

        for (uint8_t i = i1; i < (i1 + 3); i++) {
            for (uint8_t j = j1; j < (j1 + 3); j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {
                    y_blocked = true;
                }
            }
        }
        if (y_blocked) // cancels potential update
            y1 = y0;
    }

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

/*----------------------------------------------------------------------------*/

void _movePlayer() {

    // get current bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);
    uint8_t i0 = (y0 >> 3);
    uint8_t j0 = (x0 >> 3);

    // get the target coordinates based on current bearing
    uint16_t x1 = x0, y1 = y0;
    if (dir_x != DIR_ZERO)
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++;
    if (dir_y != DIR_ZERO)
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++;

    // get the sprite container for the target x,y
    uint8_t i1 = (y1 >> 3);
    uint8_t j1 = (x1 >> 3);

    // check the target sprite container for wall tiles and mark which side struck
    bool hit_h = false, hit_v = false;
    for (uint8_t i = i1; i < (i1 + 3); i++) {
        for (uint8_t j = j1; j < (j1 + 3); j++) {
            uint8_t tid = beebram[CAMBUFFER + 40 * i + j];

            // if tile at i,j is a wall
            if (tid >= TID_WALL_SQUARE && tid < TID_WALL_SQUARE + 4) {

                // origin is above or below a wall tile
                if (i0 < i1 || i0 > i1) {

                    uint8_t x_wall = 8 * j;
                    int dx = x_wall - x1;
                    fprintf(stderr, "dx: %d\n", dx);

                    if (dx >= -8 && dx < 16) {
                        hit_v = true;
                    }

                }

                // origin is left or right of a wall tile
                else if (j0 < j1 || j0 > j1) {

                    uint8_t y_wall = 8 * i;
                    int dy = y_wall - y1;
                    fprintf(stderr, "dy: %d\n", dy);

                    if (dy >= -8 && dy < 16) {
                        hit_h = true;
                    }
                }
            }
        }
    }
    fprintf(stderr, "\n");

    if (!hit_h) {
        beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
        beebram[PLAYER + ME_X_HI] = x1 >> 8;
    }

    if (!hit_v) {
        beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y1 >> 8;
    }

    // update the sprite container
    updateSpriteContainer(PLAYER);

    // raise the redraw flag to let renderer know movement has taken place
    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
}