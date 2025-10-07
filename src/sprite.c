#include "sprite.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>
#include <stdio.h>

void updateSpriteContainer(uint16_t movable) {
    uint16_t x = beebram[movable + ME_X_LO] | (beebram[movable + ME_X_HI] << 8);
    uint16_t y = beebram[movable + ME_Y_LO] | (beebram[movable + ME_Y_HI] << 8);

    // shifts will always be updated on any move
    uint8_t hshift = x & 0b111;
    uint8_t vshift = y & 0b111;
    beebram[movable + ME_HSHIFT4_VSHIFT4] = (beebram[movable + ME_HSHIFT4_VSHIFT4] & 0x0F) | (hshift << 4);
    beebram[movable + ME_HSHIFT4_VSHIFT4] = (beebram[movable + ME_HSHIFT4_VSHIFT4] & 0xF0) | vshift;

    uint8_t current_i = beebram[movable + CE_I];
    uint8_t current_j = beebram[movable + CE_J];
    uint8_t new_i = y >> 3;
    uint8_t new_j = x >> 3;

    // if sprite container moves
    if (current_i != new_i || current_j != new_j) {

        // save the old container position
        beebram[movable + ME_OLDI] = current_i;
        beebram[movable + ME_OLDJ] = current_j;

        // write the new container position
        beebram[movable + CE_I] = new_i;
        beebram[movable + CE_J] = new_j;

        // raise cleanup flag
        beebram[movable + CE_ROOMID6_CLEAN1_REDRAW1] |= 0b10;
    }
}

/*----------------------------------------------------------------------------*/

void bufferFGSprite(uint16_t pentity) {

    // get the quad, either directly or via an animdef
    uint16_t pvizbase = beebram[pentity + CE_PVIZBASE_LO] | (beebram[pentity + CE_PVIZBASE_HI] << 8);
    uint16_t pquad = pvizbase;

    // if the vizdef is an animdef, dereference it to get to the quad
    if (pvizbase >= AD_TABLE && pvizbase < AD_DEFS) {
        uint8_t animset = beebram[pentity + ME_ANIMSET];
        uint16_t ppanimdef = pvizbase + animset;

        uint16_t panimdef = beebram[ppanimdef] | (beebram[ppanimdef + 1] << 8);

        uint8_t frame_offset = (beebram[PLAYER + CE_FELAPSED5_FCURRENT3] & 0b111) << 1; // *2 because 2 byte pointer
        pquad =
            beebram[panimdef + AD_PFRAME_LO + frame_offset] | (beebram[panimdef + AD_PFRAME_HI + frame_offset] << 8);
    }

    // get the shifts
    uint8_t rshift = beebram[pentity + ME_HSHIFT4_VSHIFT4] >> 4;
    uint8_t lshift = 8 - rshift;
    uint8_t dshift = beebram[pentity + ME_HSHIFT4_VSHIFT4] & 0x0F;
    uint8_t ushift = 8 - dshift;

    // position each sprite tile in the container by its shifts
    uint8_t thops[4] = {0, 8, 24, 32}; // +8, +16, +8 ... 1, 2, 1

    for (uint8_t tile = 0; tile < 4; tile++) {
        uint16_t penstart = OFFBUFFER + dshift + thops[tile];
        uint16_t ptexture = beebram[pquad + (tile << 1)] | (beebram[pquad + (tile << 1) + 1] << 8);
        uint16_t pmask = beebram[pquad + (tile << 1) + 8] | (beebram[pquad + (tile << 1) + 9] << 8);

        uint8_t hflipped = ptexture >> 15;
        ptexture &= 0x7FFF;
        pmask &= 0x7FFF;

        for (uint8_t s = 0; s < 8; s++) {
            uint8_t overL, overR, maskL, maskR;
            if (!hflipped) {
                overL = beebram[ptexture + s] >> rshift;
                overR = beebram[ptexture + s] << lshift;
                maskL = beebram[pmask + s] >> rshift;
                maskR = beebram[pmask + s] << lshift;
            } else if (hflipped) {
                uint8_t texture_data = beebram[ptexture + s];
                uint8_t mask_data = beebram[pmask + s];
                overL = beebram[LUT_REVERSE + texture_data] >> rshift;
                overR = beebram[LUT_REVERSE + texture_data] << lshift;
                maskL = beebram[LUT_REVERSE + mask_data] >> rshift;
                maskR = beebram[LUT_REVERSE + mask_data] << lshift;
            }

            // stripes have crossed to lower tile
            if (s == ushift) {
                penstart += 16;
            }

            // lhs
            beebram[penstart + s] = beebram[penstart + s] & (maskL ^ 0xFF);
            beebram[penstart + s] = beebram[penstart + s] | overL;

            // rhs
            beebram[penstart + 8 + s] = beebram[penstart + 8 + s] & (maskR ^ 0xFF);
            beebram[penstart + 8 + s] = beebram[penstart + 8 + s] | overR;
        }
    }
}
