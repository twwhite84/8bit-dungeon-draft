#include "sprite.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>
#include <stdio.h>

void updateSpriteContainer(uint16_t movable) {

    // shifts will always update on any movement made
    uint16_t x = beebram[movable + MEF_X_LO] | (beebram[movable + MEF_X_HI] << 8);
    uint16_t y = beebram[movable + MEF_Y_LO] | (beebram[movable + MEF_Y_HI] << 8);
    uint8_t hshift = x & 0b111;
    uint8_t vshift = y & 0b111;
    beebram[movable + MEF_HSHIFT4_VSHIFT4] = (beebram[movable + MEF_HSHIFT4_VSHIFT4] & 0x0F) | (hshift << 4);
    beebram[movable + MEF_HSHIFT4_VSHIFT4] = (beebram[movable + MEF_HSHIFT4_VSHIFT4] & 0xF0) | vshift;

    // sprite container may or may not move
    uint8_t current_i = beebram[movable + CEF_I];
    uint8_t current_j = beebram[movable + CEF_J];
    uint8_t new_i = y >> 3;
    uint8_t new_j = x >> 3;

    // if sprite container moves
    if (current_i != new_i || current_j != new_j) {

        // save the old container position
        beebram[movable + MEF_OLDI] = current_i;
        beebram[movable + MEF_OLDJ] = current_j;

        // write the new container position
        beebram[movable + CEF_I] = new_i;
        beebram[movable + CEF_J] = new_j;

        // raise cleanup flag for streak removal
        beebram[movable + CEF_DRAWOPTS] |= CEC_DRAWOPTS_CLEAN;
    }
}