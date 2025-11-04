#include "renderer.h"
#include "mySDL.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

// copies background from I,J into a buffer
void bufferBG(uint8_t i, uint8_t j, uint8_t dim) {
    uint16_t penstart = OFFBUFFER;

    for (int rel_i = 0; rel_i < dim; rel_i++) {
        for (int rel_j = 0; rel_j < dim; rel_j++) {
            uint8_t tileID = beebram[BGBUFFER + 40 * (i + rel_i) + (j + rel_j)];
            uint16_t texture = getTileTextureAddr(tileID);
            bufferTile(penstart, texture, SENTINEL16);
            penstart += 8;
        }
    }
}

/*----------------------------------------------------------------------------*/

// copies all statiks overlapping a sprite into the sprite buffer
void statiks2container(uint16_t pentity) {
    // memset(&beebram[OFFBUFFER], 0, (size_t)(8 * 9));

    int pi = beebram[pentity + CEF_I];
    int pj = beebram[pentity + CEF_J];

    uint16_t psebase = CAMERA + CAMF_PSE0_LO;
    for (uint8_t idx = 0; idx < 10; idx++) {
        uint16_t pse = beebram[psebase] | (beebram[psebase + 1] << 8);
        psebase += 2;
        uint8_t nquads = beebram[pse + SEF_TYPE4_NQUADS4] & 0x0F;
        for (uint8_t q = 0; q < nquads; q++) {
            uint16_t pvizdef =
                beebram[pse + CEF_PVIZBASE_LO + (4 * q)] | (beebram[pse + CEF_PVIZBASE_HI + (4 * q)] << 8);

            if (pvizdef >= AD_TABLE) {
                uint16_t animdef = beebram[pvizdef] | (beebram[pvizdef + 1] << 8);
                uint8_t current = beebram[pse + CEF_FELAPSED5_FCURRENT3] & 0b00000111;
                pvizdef = beebram[(animdef + ADF_PFRAME_LO) + (2 * current)];
                pvizdef |= (beebram[(animdef + ADF_PFRAME_HI) + (2 * current)] << 8);
            }

            uint16_t pquad = pvizdef;
            int qi = beebram[pse + CEF_I + (4 * q)];
            int qj = beebram[pse + CEF_J + (4 * q)];

            if (qi == pi + 2) {
                if (qj == pj + 2) {
                    bufferTileIJ(0, 0, 2, 2, pquad, 3);
                }

                if (qj == pj + 1) {
                    bufferTileIJ(0, 0, 2, 1, pquad, 3);
                    bufferTileIJ(0, 1, 2, 2, pquad, 3);
                }

                if (qj == pj) {
                    bufferTileIJ(0, 0, 2, 0, pquad, 3);
                    bufferTileIJ(0, 1, 2, 1, pquad, 3);
                }

                if (qj == pj - 1) {
                    bufferTileIJ(0, 1, 2, 0, pquad, 3);
                }
            }

            if (qi == pi + 1) {

                if (qj == pj + 2) {
                    bufferTileIJ(0, 0, 1, 2, pquad, 3);
                    bufferTileIJ(1, 0, 2, 2, pquad, 3);
                }

                if (qj == pj + 1) {
                    bufferTileIJ(0, 0, 1, 1, pquad, 3);
                    bufferTileIJ(0, 1, 1, 2, pquad, 3);
                    bufferTileIJ(1, 0, 2, 1, pquad, 3);
                    bufferTileIJ(1, 1, 2, 2, pquad, 3);
                }

                if (qj == pj) {
                    bufferTileIJ(0, 0, 1, 0, pquad, 3);
                    bufferTileIJ(0, 1, 1, 1, pquad, 3);
                    bufferTileIJ(1, 0, 2, 0, pquad, 3);
                    bufferTileIJ(1, 1, 2, 1, pquad, 3);
                }

                if (qj == pj - 1) {
                    bufferTileIJ(0, 1, 1, 0, pquad, 3);
                    bufferTileIJ(1, 1, 2, 0, pquad, 3);
                }
            }

            if (qi == pi) {
                if (qj == pj + 2) {
                    bufferTileIJ(0, 0, 0, 2, pquad, 3);
                    bufferTileIJ(1, 0, 1, 2, pquad, 3);
                }
                if (qj == pj + 1) {
                    bufferTileIJ(0, 0, 0, 1, pquad, 3);
                    bufferTileIJ(0, 1, 0, 2, pquad, 3);
                    bufferTileIJ(1, 0, 1, 1, pquad, 3);
                    bufferTileIJ(1, 1, 1, 2, pquad, 3);
                }
                if (qj == pj) {
                    bufferTileIJ(0, 0, 0, 0, pquad, 3);
                    bufferTileIJ(0, 1, 0, 1, pquad, 3);
                    bufferTileIJ(1, 0, 1, 0, pquad, 3);
                    bufferTileIJ(1, 1, 1, 1, pquad, 3);
                }
                if (qj == pj - 1) {
                    bufferTileIJ(0, 1, 0, 0, pquad, 3);
                    bufferTileIJ(1, 1, 1, 0, pquad, 3);
                }
            }

            if (qi == pi - 1) {
                if (qj == pj + 2) {
                    bufferTileIJ(1, 0, 0, 2, pquad, 3);
                }
                if (qj == pj + 1) {
                    bufferTileIJ(1, 0, 0, 1, pquad, 3);
                    bufferTileIJ(1, 1, 0, 2, pquad, 3);
                }
                if (qj == pj) {
                    bufferTileIJ(1, 0, 0, 0, pquad, 3);
                    bufferTileIJ(1, 1, 0, 1, pquad, 3);
                }
                if (qj == pj - 1) {
                    bufferTileIJ(1, 1, 0, 0, pquad, 3);
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------*/

// renders to offbuffer one tile with optional masking
void bufferTileIJ(uint8_t src_i, uint8_t src_j, uint8_t dst_i, uint8_t dst_j, uint16_t pquad, uint8_t dim) {

    uint8_t imult = (dim == 2) ? 16 : 24;

    uint16_t ptexture, pmask, penread, penwrite;

    if (pquad >= QUADS_COMP) {
        goto compdef;
    }

plaindef:
    ptexture = pquad + (4 * src_i) + (2 * src_j);
    penread = beebram[ptexture] | (beebram[ptexture + 1] << 8);
    penwrite = OFFBUFFER + (imult * dst_i) + (8 * dst_j);

    for (int s = 0; s < 8; s++) {
        beebram[penwrite + s] = beebram[penread + s];
    }
    return;

compdef:
    // compdef quads always stored in 4xquad_texture, 4xquad_mask order
    // also the MSB is a flag indicating reverse storage
    ptexture = beebram[pquad + (4 * src_i) + (2 * src_j)];
    ptexture |= (beebram[pquad + (4 * src_i) + (2 * src_j) + 1] << 8);
    pmask = beebram[pquad + (4 * src_i) + (2 * src_j) + 8];
    pmask |= (beebram[pquad + (4 * src_i) + (2 * src_j) + 1 + 8] << 8);

    uint8_t hflipped = ptexture >> 15;

    if (!hflipped) {
        penwrite = OFFBUFFER + (imult * dst_i) + (8 * dst_j);
        for (int s = 0; s < 8; s++) {
            beebram[penwrite + s] &= (beebram[pmask + s] ^ 0xFF);
            beebram[penwrite + s] |= (beebram[ptexture + s] & beebram[pmask + s]);
        }
    }

    else {
        ptexture &= 0x7FFF;
        pmask &= 0x7FFF;
        penwrite = OFFBUFFER + (imult * dst_i) + (8 * dst_j);
        for (int s = 0; s < 8; s++) {
            uint8_t mask_data = beebram[pmask + s];
            uint8_t texture_data = beebram[ptexture + s];
            beebram[penwrite + s] &= (beebram[LUT_REVERSE + mask_data] ^ 0xFF);
            beebram[penwrite + s] |= (beebram[LUT_REVERSE + texture_data] & beebram[LUT_REVERSE + mask_data]);
        }
    }
}

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
            for (uint8_t i = 0; i < 2; i++) {
                for (uint8_t j = 0; j < 2; j++) {
                    bufferTileIJ(i, j, i, j, pvizdef, 2);
                }
            }
            renderOffbuffer(qi, qj, 2);
        }
    }
}

/*----------------------------------------------------------------------------*/

// renders to framebuffer a movable if marked for redraw
void renderPlayer() {
    uint8_t i = beebram[PLAYER + CEF_I];
    uint8_t j = beebram[PLAYER + CEF_J];
    bufferBG(i, j, 3);
    statiks2container(PLAYER);
    bufferFGSprite(PLAYER);
    renderOffbuffer(i, j, 3);

    beebram[PLAYER + CEF_ROOMID6_REDRAW2] &= ~CEC_REDRAW;
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
            renderPlayer(pmovable);
    }
}

/*----------------------------------------------------------------------------*/

// renders a background tile
void renderBGTile(uint8_t i, uint8_t j) {
    uint8_t tid = beebram[BGBUFFER + 40 * i + j];
    uint16_t tileptr = getTileTextureAddr(tid);
    uint16_t screenpos = SCREEN + (CAMC_WIDTH * i) + (8 * j);

    for (uint8_t s = 0; s < 8; s++) {
        beebram[screenpos + s] = beebram[tileptr + s];
    }
}

// renders entire background
void renderBG() {
    for (uint8_t i = 0; i < 26; i++) {
        for (uint8_t j = 0; j < 40; j++) {
            renderBGTile(i, j);
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
    // return;

    uint8_t xdir = (beebram[pentity + MEF_XMD4_YMD4] >> 4) & 0b11;
    uint8_t ydir = (beebram[pentity + MEF_XMD4_YMD4] & 0x0F) & 0b11;

    uint8_t old_i = beebram[pentity + MEF_OLDI];
    uint8_t old_j = beebram[pentity + MEF_OLDJ];

    // moving up, clear below
    if (ydir == DIR_UP) {
        renderBGTile(old_i + 2, old_j + 0);
        renderBGTile(old_i + 2, old_j + 1);
        renderBGTile(old_i + 2, old_j + 2);
    }

    // moving down, clean above
    if (ydir == DIR_DOWN) {
        renderBGTile(old_i, old_j + 0);
        renderBGTile(old_i, old_j + 1);
        renderBGTile(old_i, old_j + 2);
    }

    // moving left, clear right
    if (xdir = DIR_LEFT) {
        renderBGTile(old_i + 0, old_j + 2);
        renderBGTile(old_i + 1, old_j + 2);
        renderBGTile(old_i + 2, old_j + 2);
    }

    // moving right, clear left
    if (xdir = DIR_RIGHT) {
        renderBGTile(old_i + 0, old_j);
        renderBGTile(old_i + 1, old_j);
        renderBGTile(old_i + 2, old_j);
    }

    // lower cleanup flag
    beebram[pentity + CEF_ROOMID6_REDRAW2] &= ~CEC_CLEAN;
}

/*----------------------------------------------------------------------------*/

void renderEraseSlot() {
    // background redrawn over the item's tile/quad
    uint16_t pentity = beebram[CAMERA + CAMF_PERASE_LO] | (beebram[CAMERA + CAMF_PERASE_HI] << 8);
    uint8_t sei = beebram[pentity + CEF_I];
    uint8_t sej = beebram[pentity + CEF_J];
    renderBGTile(sei, sej);
    renderBGTile(sei, sej + 1);
    renderBGTile(sei + 1, sej);
    renderBGTile(sei + 1, sej + 1);

    // clear the erase slot
    beebram[CAMERA + CAMF_PERASE_LO] = SENTINEL8;
    beebram[CAMERA + CAMF_PERASE_HI] = SENTINEL8;
}