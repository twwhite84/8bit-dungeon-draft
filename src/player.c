#include "player.h"
#include "shared.h"
#include <stdbool.h>
#include <stdio.h>

void movePlayer(uint8_t dir) {
    fprintf(stderr, "Player direction: %d\n", dir);

    // raise the redraw flag
    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + CE_ROOMID6_REDRAW2] |= true;

    // for now simply increment/decrement the player x or y
    uint16_t x, y;
    switch (dir) {
    case PLRDIR_R:
        beebram[PLAYER + CE_PVIZDEF_LO] = ADPTR_DOGWALKR & 0xFF;
        beebram[PLAYER + CE_PVIZDEF_HI] = ADPTR_DOGWALKR >> 8;
        x = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
        x++;
        beebram[PLAYER + ME_X_LO] = x & 0xFF;
        beebram[PLAYER + ME_X_HI] = x >> 8;
        break;

    case PLRDIR_L:
        beebram[PLAYER + CE_PVIZDEF_LO] = ADPTR_DOGWALKL & 0xFF;
        beebram[PLAYER + CE_PVIZDEF_HI] = ADPTR_DOGWALKL >> 8;
        x = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
        x--;
        beebram[PLAYER + ME_X_LO] = x & 0xFF;
        beebram[PLAYER + ME_X_HI] = x >> 8;

        break;
    case PLRDIR_U:
        beebram[PLAYER + CE_PVIZDEF_LO] = ADPTR_DOGWALKU & 0xFF;
        beebram[PLAYER + CE_PVIZDEF_HI] = ADPTR_DOGWALKU >> 8;
        y = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);
        y--;
        beebram[PLAYER + ME_Y_LO] = y & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y >> 8;
        break;

    case PLRDIR_D:
        beebram[PLAYER + CE_PVIZDEF_LO] = ADPTR_DOGWALKD & 0xFF;
        beebram[PLAYER + CE_PVIZDEF_HI] = ADPTR_DOGWALKD >> 8;
        y = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);
        y++;
        beebram[PLAYER + ME_Y_LO] = y & 0xFF;
        beebram[PLAYER + ME_Y_HI] = y >> 8;
        break;
    }

    // set the player direction vector

    // check the next tile a pixel of movement in that direction will land

    // take appropriate action
}