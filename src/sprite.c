#include "sprite.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

void updateSpriteContainer(uint16_t actor) {
    // ONLY IMPLEMENTED FOR PLAYER FOR NOW

    // get current player position
    uint16_t x = beebram[actor + ME_X_LO] | (beebram[actor + ME_X_HI] << 8);
    uint16_t y = beebram[actor + ME_Y_LO] | (beebram[actor + ME_Y_HI] << 8);

    // compute and set the shifts for the sprite container
    uint8_t hshift = x & 0b111;
    uint8_t vshift = y & 0b111;
    beebram[actor + ME_HSHIFT4_VSHIFT4] = (beebram[actor + ME_HSHIFT4_VSHIFT4] & 0x0F) | (hshift << 4);
    beebram[actor + ME_HSHIFT4_VSHIFT4] = (beebram[actor + ME_HSHIFT4_VSHIFT4] & 0xF0) | vshift;

    // compute relative screen address for origin of sprite container (top-left corner)
    uint16_t corner = xy2ij(x, y);
    beebram[actor + CE_I] = corner >> 8;
    beebram[actor + CE_J] = corner & 0xFF;
}

/*----------------------------------------------------------------------------*/

void bufferSpriteForeground(uint16_t actor) {
    // only implemented for player for now
    int rshift = beebram[actor + ME_HSHIFT4_VSHIFT4] >> 4;
    int lshift = 8 - rshift;

    int dshift = beebram[actor + ME_HSHIFT4_VSHIFT4] & 0x0F;
    int ushift = 8 - dshift;

    uint16_t pvizdef = beebram[actor + CE_PVIZDEF_LO] | (beebram[actor + CE_PVIZDEF_HI] << 8);
    uint16_t pcompdef;

    // if the vizdef is an animdef, get the current frame
    if (pvizdef >= ANIMDEFS) {
        uint8_t current = beebram[PLAYER + CE_FELAPSED5_FCURRENT3] & 0b111;
        current *= 2;
        pcompdef = beebram[pvizdef + AD_PFRAME_LO + current] | (beebram[pvizdef + AD_PFRAME_HI + current] << 8);
    } else {
        pcompdef = pvizdef;
    }

    uint16_t penbase = OFFBUFFER + dshift;

    // position each portion of the quad into place
    int tidx_lo = 6; // [(0,1),(2,3),(4,5),(6,7)]
    for (int t = 3; t >= 0; t--) {
        uint16_t penstart = penbase + bhops[t]; // +15 and penstart -=16 to decrement inner loop

        uint16_t ptexture = beebram[pcompdef + tidx_lo] | (beebram[pcompdef + tidx_lo + 1] << 8);
        uint16_t pmask = beebram[pcompdef + 8 + tidx_lo] | (beebram[pcompdef + 8 + tidx_lo + 1] << 8);
        tidx_lo -= 2;
        uint8_t hflipped = ptexture >> 15;
        ptexture &= 0x7FFF;
        pmask &= 0x7FFF;

        // to get this to decrement, penstart -= 16 with initial penbase + 15 (or something)
        for (int s = 0; s < 8; s++) {
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