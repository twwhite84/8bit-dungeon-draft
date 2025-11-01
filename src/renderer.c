#include "renderer.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void renderCambufferTile(uint8_t i, uint8_t j);

/*----------------------------------------------------------------------------*/

// renders to offbuffer a square selection of the cambuffer from i,j
void bufferBG(uint8_t abs_i, uint8_t abs_j, uint8_t dim) {
    uint16_t penstart = OFFBUFFER;

    for (int rel_i = 0; rel_i < dim; rel_i++) {
        for (int rel_j = 0; rel_j < dim; rel_j++) {
            uint8_t tileID = beebram[BGBUFFER + 40 * (abs_i + rel_i) + (abs_j + rel_j)];
            uint16_t texture = getTileTextureAddr(tileID);
            bufferTile(penstart, texture, SENTINEL16);
            penstart += 8;
        }
    }
}

/*----------------------------------------------------------------------------*/

void bufferFGQuad(uint16_t pquad) {
    uint16_t penstart = OFFBUFFER;
    uint16_t mask = SENTINEL16, texture = 0;

    for (uint8_t tile = 0; tile < 4; tile++) {
        texture = beebram[pquad + 2 * tile] + (beebram[pquad + 2 * tile + 1] << 8);

        if (pquad >= QUADS_COMP)
            mask = beebram[pquad + 8 + 2 * tile] + (beebram[pquad + 8 + 2 * tile + 1] << 8);

        bufferTile(penstart, texture, mask);
        penstart += 8;
    }
}

/*----------------------------------------------------------------------------*/

// renders to offbuffer one tile with optional masking
void bufferTile(uint16_t penstart, uint16_t texture, uint16_t mask) {
    if (mask != SENTINEL16)
        goto compdef;

plaindef:
    for (int s = 7; s >= 0; s--) {
        beebram[penstart + s] = beebram[texture + s];
    }
    return;

compdef:
    uint8_t hflipped = texture >> 15;

    if (!hflipped) {
        for (int s = 7; s >= 0; s--) {
            beebram[penstart + s] &= (beebram[mask + s] ^ 0xFF);
            beebram[penstart + s] |= (beebram[texture + s] & beebram[mask + s]);
        }
    }

    else {
        texture &= 0x7FFF;
        mask &= 0x7FFF;
        for (int s = 7; s >= 0; s--) {
            uint8_t mask_data = beebram[mask + s];
            uint8_t texture_data = beebram[texture + s];
            beebram[penstart + s] &= (beebram[LUT_REVERSE + mask_data] ^ 0xFF);
            beebram[penstart + s] |= (beebram[LUT_REVERSE + texture_data] & beebram[LUT_REVERSE + mask_data]);
        }
    }
    // mySDLRender();
}

/*----------------------------------------------------------------------------*/

// renders to framebuffer all statics held in camera that are marked for redraw
void renderStatics() {

    uint16_t pstart = CAMERA + CAMF_PSE0_LO;
    for (int i = 0; i < 10; i++) {

        // get the entity or quit loop early if sentinel
        uint16_t pentity = beebram[pstart] + (beebram[pstart + 1] << 8);
        if (pentity == SENTINEL16)
            break;
        pstart += 2;

        // skip entity if not marked for redraw, or disable redraw until future update
        uint8_t redraw = (beebram[pentity + CEF_ROOMID6_REDRAW2] & CEC_REDRAW);
        if (redraw != CEC_REDRAW)
            continue;
        else
            beebram[pentity + CEF_ROOMID6_REDRAW2] &= ~CEC_REDRAW;

        uint8_t nquads = beebram[pentity + SEF_TYPE4_NQUADS4] & 0x0F;
        for (int q = 0; q < nquads; q++) {
            uint8_t qi = beebram[(pentity + CEF_I) + (4 * q)]; // 4q because repeating section 4 fields long
            uint8_t qj = beebram[(pentity + CEF_J) + (4 * q)];
            uint16_t pvizdef =
                beebram[(pentity + CEF_PVIZBASE_LO) + (4 * q)] | (beebram[(pentity + CEF_PVIZBASE_HI) + (4 * q)] << 8);

            // if not animated, jump ahead to directly rendering the quad
            if (pvizdef < AD_TABLE) {
                goto render;
            }

        animdef:
            // dereference the table entry, dont add an animset because SE only has one animdef
            uint16_t animdef = beebram[pvizdef] | (beebram[pvizdef + 1] << 8);

            uint8_t current = beebram[pentity + CEF_FELAPSED5_FCURRENT3] & 0b00000111;

            // get the current frame for rendering to offbuffer
            pvizdef = beebram[(animdef + ADF_PFRAME_LO) + (2 * current)];
            pvizdef |= (beebram[(animdef + ADF_PFRAME_HI) + (2 * current)] << 8);

        render:
            bufferBG(qi, qj, 2);
            bufferFGQuad(pvizdef);
            renderOffbuffer(qi, qj, 2);
        }
    }
}

/*----------------------------------------------------------------------------*/

// renders to framebuffer a movable if marked for redraw
void renderMovable(uint16_t pmovable) {
    uint8_t clean = (beebram[pmovable + CEF_ROOMID6_REDRAW2] & CEC_CLEAN);
    if (clean == CEC_CLEAN)
        renderCleanup(pmovable);

    uint8_t i = beebram[pmovable + CEF_I];
    uint8_t j = beebram[pmovable + CEF_J];
    bufferBG(i, j, 3);
    bufferFGSprite(pmovable);
    renderOffbuffer(i, j, 3);
    beebram[pmovable + CEF_ROOMID6_REDRAW2] &= ~CEC_REDRAW;
}

/*----------------------------------------------------------------------------*/

// renders to framebuffer all movables held in camera that are marked for redraw
void renderMovables() {
    uint16_t pstart = CAMERA + CAMF_PME0_LO;
    for (uint8_t i = 0; i < 4; i++) {
        uint16_t pmovable = beebram[pstart] | (beebram[pstart + 1] << 8);
        if (pmovable == SENTINEL16)
            break;
        pstart += 2;
        if ((beebram[pmovable + CEF_ROOMID6_REDRAW2] & CEC_REDRAW) == CEC_REDRAW)
            renderMovable(pmovable);
    }
}

/*----------------------------------------------------------------------------*/

// renders the entire cambuffer (ie background tilemap) to the framebuffer
void renderBackground() {
    for (uint8_t i = 0; i < 26; i++) {
        for (uint8_t j = 0; j < 40; j++) {
            renderCambufferTile(i, j);
        }
    }
}

/*----------------------------------------------------------------------------*/

// renders square portion of the offbuffer to screen at absolute i,j
void renderOffbuffer(uint8_t i, uint8_t j, uint8_t dim) {
    uint16_t penstart = SCREEN + i * 0x140 + j * 8;
    uint16_t offbase = OFFBUFFER;

    for (uint8_t rel_i = 0; rel_i < dim; rel_i++) {
        for (uint8_t rel_j = 0; rel_j < dim; rel_j++) {
            for (uint8_t s = 0; s < 8; s++) {
                beebram[penstart + s] = beebram[offbase + s];
            }
            offbase += 8;
            penstart += 8;
        }
        penstart += CAMC_WIDTH - (dim << 3);
    }
}

/*----------------------------------------------------------------------------*/

// streak removal when moving the sprite container
// this mainly seems to affect the vertical rather than horizontal axis
void renderCleanup(uint16_t pentity) {

    uint8_t xdir = (beebram[pentity + MEF_XMD4_YMD4] >> 4) & 0b11;
    uint8_t ydir = (beebram[pentity + MEF_XMD4_YMD4] & 0x0F) & 0b11;

    uint8_t old_i = beebram[pentity + MEF_OLDI];
    uint8_t old_j = beebram[pentity + MEF_OLDJ];

    // moving up, clear below
    if (ydir == DIR_UP) {
        renderCambufferTile(old_i + 2, old_j + 0);
        renderCambufferTile(old_i + 2, old_j + 1);
        renderCambufferTile(old_i + 2, old_j + 2);
    }

    // moving down, clean above
    if (ydir == DIR_DOWN) {
        renderCambufferTile(old_i, old_j + 0);
        renderCambufferTile(old_i, old_j + 1);
        renderCambufferTile(old_i, old_j + 2);
    }

    // moving left, clear right
    if (xdir = DIR_LEFT) {
        renderCambufferTile(old_i + 0, old_j + 2);
        renderCambufferTile(old_i + 1, old_j + 2);
        renderCambufferTile(old_i + 2, old_j + 2);
    }

    // moving right, clear left
    if (xdir = DIR_RIGHT) {
        renderCambufferTile(old_i + 0, old_j);
        renderCambufferTile(old_i + 1, old_j);
        renderCambufferTile(old_i + 2, old_j);
    }

    // lower cleanup flag
    beebram[pentity + CEF_ROOMID6_REDRAW2] &= ~CEC_CLEAN;
}

/*----------------------------------------------------------------------------*/

void renderCambufferTile(uint8_t i, uint8_t j) {
    uint8_t tid = beebram[BGBUFFER + 40 * i + j];
    uint16_t tileptr = getTileTextureAddr(tid);
    uint16_t screenpos = SCREEN + (CAMC_WIDTH * i) + (8 * j);

    for (uint8_t s = 0; s < 8; s++) {
        beebram[screenpos + s] = beebram[tileptr + s];
    }
}

void renderEraseSlot() {
    // background redrawn over the item's tile/quad
    uint16_t pentity = beebram[CAMERA + CAMF_PERASE_LO] | (beebram[CAMERA + CAMF_PERASE_HI] << 8);
    uint8_t sei = beebram[pentity + CEF_I];
    uint8_t sej = beebram[pentity + CEF_J];
    renderCambufferTile(sei, sej);
    renderCambufferTile(sei, sej + 1);
    renderCambufferTile(sei + 1, sej);
    renderCambufferTile(sei + 1, sej + 1);

    // clear the erase slot
    beebram[CAMERA + CAMF_PERASE_LO] = SENTINEL8;
    beebram[CAMERA + CAMF_PERASE_HI] = SENTINEL8;
}