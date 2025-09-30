#include "sprite.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

void updateSpriteContainer(uint16_t movable) {
    uint16_t x = beebram[movable + ME_X_LO] | (beebram[movable + ME_X_HI] << 8);
    uint16_t y = beebram[movable + ME_Y_LO] | (beebram[movable + ME_Y_HI] << 8);

    // sprite offsets within container
    uint8_t hshift = x & 0b111;
    uint8_t vshift = y & 0b111;
    beebram[movable + ME_HSHIFT4_VSHIFT4] = (beebram[movable + ME_HSHIFT4_VSHIFT4] & 0x0F) | (hshift << 4);
    beebram[movable + ME_HSHIFT4_VSHIFT4] = (beebram[movable + ME_HSHIFT4_VSHIFT4] & 0xF0) | vshift;

    // sprite container absolute i,j
    uint16_t i_j = xy2ij(x, y);
    beebram[movable + CE_CONTAINER_I] = i_j >> 8;
    beebram[movable + CE_CONTAINER_J] = i_j & 0xFF;
}

/*----------------------------------------------------------------------------*/

void bufferFGSprite(uint16_t movable) {

    // get the quad, either directly or via an animdef
    uint16_t pquad = beebram[movable + CE_PVIZDEF_LO] | (beebram[movable + CE_PVIZDEF_HI] << 8);
    if (pquad >= ANIMDEFS) {
        uint8_t frame_offset = (beebram[PLAYER + CE_FELAPSED5_FCURRENT3] & 0b111) << 1;
        pquad = beebram[pquad + AD_PFRAME_LO + frame_offset] | (beebram[pquad + AD_PFRAME_HI + frame_offset] << 8);
    }

    // get the shifts
    uint8_t rshift = beebram[movable + ME_HSHIFT4_VSHIFT4] >> 4;
    uint8_t lshift = 8 - rshift;
    uint8_t dshift = beebram[movable + ME_HSHIFT4_VSHIFT4] & 0x0F;
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
