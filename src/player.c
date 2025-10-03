#include "player.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>

bool isWallCollision(uint8_t i, uint8_t j, uint8_t dir_x, uint8_t dir_y);

bool playerLook() {}

void movePlayer() {

    // get bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current x,y
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);

    // get the target x, y based on passed direction of travel
    uint16_t x1 = x0, y1 = y0;
    if (dir_x != DIR_ZERO)
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++;
    if (dir_y != DIR_ZERO)
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++;

    // get the sprite container for the target x,y
    uint8_t i1 = (xy2ij(x1, y1) >> 8);
    uint8_t j1 = (xy2ij(x1, y1) & 0xFF);

    // check the contents of the sprite container for wall tiles
    bool hits_wall = false;
    for (uint8_t i = i1; i < i1 + 3; i++) {
        for (uint8_t j = j1; j < j1 + 3; j++) {
            uint8_t tid = beebram[CAMBUFFER + 40 * i + j];

            // if tile is a wall, determine if bearing will collide
            if (TID_WALL_CRATE - tid < 4)
                hits_wall = isWallCollision(i - i1, j - j1, dir_x, dir_y);
        }
    }

    // if no wall is hit, proceed to move the player
    if (!hits_wall) {
        // save the new x,y
        beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
        beebram[PLAYER + ME_X_HI] = x1 >> 8;
        beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y1 >> 8;

        // update the sprite container
        updateSpriteContainer(PLAYER);

        // raise the redraw flag to let renderer know movement has taken place
        beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
        beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
    }
}

bool isWallCollision(uint8_t i, uint8_t j, uint8_t dir_x, uint8_t dir_y) {
    // i need to check if the wall tile i,j is in the path of the player's bearing

    // [(0,0) (0,1) (0,2)]
    // [(1,0) (1,1) (1,2)]
    // [(2,0) (2,1) (2,2)]

    // wall cells are above
    if (i == 0) {
        fprintf(stderr, "\nWALL IS ABOVE");
        return true;
    }

    // wall cells are below
    if (i == 2) {
        fprintf(stderr, "\nWALL IS BELOW");
        return true;
    }

    // wall cells are left
    if (j == 0 && i == 1) {
        fprintf(stderr, "\nWALL IS LEFT");
        return true;
    }

    // wall cells are right
    if (j == 2 && i == 1) {
        fprintf(stderr, "\nWALL IS RIGHT");
        return true;
    }

    return false;
}

/* void _movePlayer() {

    // get bearing
    uint8_t dir_x = beebram[PLAYER + ME_DIRX4_DIRY4] >> 4;
    uint8_t dir_y = beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F;

    // get the current x,y
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);

    // get the target x, y based on passed direction of travel
    uint16_t x1 = x0, y1 = y0;
    if (dir_x != DIR_ZERO)
        (dir_x == DIR_NEGATIVE) ? x1-- : x1++;
    if (dir_y != DIR_ZERO)
        (dir_y == DIR_NEGATIVE) ? y1-- : y1++;

    // will this involve moving the sprite container? check the i,j before and after
    bool hits_wall = false;
    uint8_t i0 = (xy2ij(x0, y0) >> 8);
    uint8_t j0 = (xy2ij(x0, y0) & 0xFF);

    uint8_t i1 = (xy2ij(x1, y1) >> 8);
    uint8_t j1 = (xy2ij(x1, y1) & 0xFF);

    // if the sprite container will move, where will it move to? will it overlap a wall?
    if (i1 - i0 != 0 || j1 - j0 != 0) {
        bool contains_wall = false;
        // fprintf(stderr, "\nNEW I,J will be %d,%d", i1, j1);

        // the sprite container is 3x3, so check what will be copied to it
        for (uint8_t i = i1; i < i1 + 3; i++) {
            // fprintf(stderr, "\n[ ");
            for (uint8_t j = j1; j < j1 + 3; j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                // fprintf(stderr, "%d ", tid);
                if (TID_WALL_CRATE - tid < 4) {

                    contains_wall = true;
                    hits_wall = isWallCollision(i - i1, j - j1, dir_x, dir_y);
                    // pass the relative i,j and check against direction of travel
                }
            }
            // fprintf(stderr, " ]");
        }

        // if sprite container contains wall cells, check distance to see if player hits them
        // you will want to perform the check based on the direction of travel
        if (contains_wall) {
            // fprintf(stderr, "\nCONTAINS WALL? %d", contains_wall);
            // fprintf(stderr, "\nHITS WALL? %d\n", hits_wall);
        }
    }

    if (!hits_wall) {
        // save the new x,y
        beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
        beebram[PLAYER + ME_X_HI] = x1 >> 8;
        beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y1 >> 8;

        // update the sprite container
        updateSpriteContainer(PLAYER);

        // print what the next i,j would be given the current direction
        // raise the redraw flag to let renderer know movement has taken place
        beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
        beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;
    }
} */
