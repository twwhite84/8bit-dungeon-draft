#include "player.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>

void movePlayer(uint8_t dir) {

    // raise the redraw flag
    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;

    // print the current i,j
    uint8_t i_current = beebram[PLAYER + CE_CONTAINER_I] + 1;
    uint8_t j_current = beebram[PLAYER + CE_CONTAINER_J] + 1;
    // fprintf(stderr, "CURRENT (I,J): %d,%d\n", i_current, j_current);

    // if (i == 0 || j == 0) {
    //     fprintf(stderr, "HIT EDGE, EXITING MOVE FUNCTION");
    //     return;
    // }

    uint16_t x, y;
    switch (dir) {
    case PLRDIR_U:
        y = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);
        y--;
        beebram[PLAYER + ME_Y_LO] = y & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y >> 8;
        beebram[PLAYER + ME_DX4_DY4] &= 0xF0;
        beebram[PLAYER + ME_DX4_DY4] |= dir;
        break;

    case PLRDIR_D:
        y = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);
        y++;
        beebram[PLAYER + ME_Y_LO] = y & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y >> 8;
        beebram[PLAYER + ME_DX4_DY4] &= 0xF0;
        beebram[PLAYER + ME_DX4_DY4] |= dir;
        break;

    case PLRDIR_R:
        x = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
        x++;
        beebram[PLAYER + ME_X_LO] = x & 0xFF;
        beebram[PLAYER + ME_X_HI] = x >> 8;
        beebram[PLAYER + ME_DX4_DY4] &= 0x0F;
        beebram[PLAYER + ME_DX4_DY4] |= (dir << 4);

        break;

    case PLRDIR_L:
        x = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
        x--;
        beebram[PLAYER + ME_X_LO] = x & 0xFF;
        beebram[PLAYER + ME_X_HI] = x >> 8;
        beebram[PLAYER + ME_DX4_DY4] &= 0x0F;
        beebram[PLAYER + ME_DX4_DY4] |= (dir << 4);

        break;
    }

    // update the sprite container
    updateSpriteContainer(PLAYER);

    // print what the next i,j would be given the current direction
}