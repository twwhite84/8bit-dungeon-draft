#include "player.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>

bool isWallCollision(uint8_t i, uint8_t j, uint8_t dir);

void movePlayer(uint8_t dir) {

    // get the current x,y
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);

    // get the target x, y based on passed direction of travel
    uint16_t x1 = x0, y1 = y0;
    switch (dir) {
    case PLRDIR_U:
        y1--;
        beebram[PLAYER + ME_DX4_DY4] &= 0xF0;
        beebram[PLAYER + ME_DX4_DY4] |= dir;
        break;

    case PLRDIR_D:
        y1++;
        beebram[PLAYER + ME_DX4_DY4] &= 0xF0;
        beebram[PLAYER + ME_DX4_DY4] |= dir;
        break;

    case PLRDIR_L:
        x1--;
        beebram[PLAYER + ME_DX4_DY4] &= 0x0F;
        beebram[PLAYER + ME_DX4_DY4] |= (dir << 4);
        break;

    case PLRDIR_R:
        x1++;
        beebram[PLAYER + ME_DX4_DY4] &= 0x0F;
        beebram[PLAYER + ME_DX4_DY4] |= (dir << 4);
        break;
    }

    // will this involve moving the sprite container? check the i,j before and after
    bool hits_wall = false;
    uint8_t i0 = (xy2ij(x0, y0) >> 8);
    uint8_t j0 = (xy2ij(x0, y0) & 0xFF);

    uint8_t i1 = (xy2ij(x1, y1) >> 8);
    uint8_t j1 = (xy2ij(x1, y1) & 0xFF);

    // if the sprite container will move, where will it move to? will it overlap a wall?
    if (i1 - i0 != 0 || j1 - j0 != 0) {
        bool contains_wall = false;
        fprintf(stderr, "\nNEW I,J will be %d,%d", i1, j1);

        // the sprite container is 3x3, so check what will be copied to it
        for (uint8_t i = i1; i < i1 + 3; i++) {
            fprintf(stderr, "\n[ ");
            for (uint8_t j = j1; j < j1 + 3; j++) {
                uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                fprintf(stderr, "%d ", tid);
                if (TID_WALL_CRATE - tid < 4) {

                    contains_wall = true;
                    hits_wall = isWallCollision(i - i1, j - j1, dir);
                    // pass the relative i,j and check against direction of travel
                }
            }
            fprintf(stderr, " ]");
        }

        // if sprite container contains wall cells, check distance to see if player hits them
        // you will want to perform the check based on the direction of travel
        if (contains_wall) {
            fprintf(stderr, "\nCONTAINS WALL? %d", contains_wall);
            fprintf(stderr, "\nHITS WALL? %d\n", hits_wall);
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
}

bool isWallCollision(uint8_t i, uint8_t j, uint8_t dir) {
    // i need to check if the guilty i,j will lie in the player's direction of travel

    // [(0,0) (0,1) (0,2)]
    // [(1,0) (1,1) (1,2)]
    // [(2,0) (2,1) (2,2)]

    // wall cells are above
    if (i == 0) {
        fprintf(stderr, "\nWALL IS ABOVE");
    }

    // wall cells are below
    if (i == 2) {
        fprintf(stderr, "\nWALL IS BELOW");
    }

    // wall cells are left
    if (j == 0) {
        fprintf(stderr, "\nWALL IS LEFT");
    }

    // wall cells are right
    if (j == 2) {
        fprintf(stderr, "\nWALL IS RIGHT");
    }

    return false;
}
