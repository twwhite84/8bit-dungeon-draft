#include "renderer.h"
#include "mySDL.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

// copies portion of background starting from I,J into the offbuffer
void bufferBG(uint8_t i, uint8_t j, uint8_t dim) {
    uint16_t penstart = OFFBUFFER;
    for (int rel_i = 0; rel_i < dim; rel_i++) {
        for (int rel_j = 0; rel_j < dim; rel_j++) {
            uint8_t tileID = beebram[BGBUFFER + 40 * (i + rel_i) + (j + rel_j)];
            uint16_t texture = getTileTextureAddr(tileID);
            bufferTextureAndMask(rel_i, rel_j, texture, SENTINEL16, dim);
        }
    }
}

/*----------------------------------------------------------------------------*/

// gets texture and mask for one cell of a quad
void getTextureAndMask(uint8_t i, uint8_t j, uint16_t quad, uint16_t *texture, uint16_t *mask) {
    uint16_t ptexture_lo = quad + (4 * i) + (2 * j);
    *texture = beebram[ptexture_lo] | (beebram[ptexture_lo + 1] << 8);

    if (quad >= QUADS_COMP) {
        uint16_t pmask_lo = quad + (4 * i) + (2 * j) + 8;
        *mask = beebram[pmask_lo] | (beebram[pmask_lo + 1] << 8);
    } else
        *mask = SENTINEL16;
}

/*----------------------------------------------------------------------------*/

// writes a texture cell with optional mask into offbuffer at I,J
void bufferTextureAndMask(uint8_t i, uint8_t j, uint16_t texture, uint16_t mask, uint8_t dim) {
    uint8_t imult = (dim == 2) ? 16 : 24;
    uint16_t penread, penwrite;
    if (mask != SENTINEL16)
        goto compdef;

plaindef:
    penwrite = OFFBUFFER + (imult * i) + (8 * j);
    for (int s = 0; s < 8; s++) {
        beebram[penwrite + s] = beebram[texture + s];
    }
    return;

compdef:
    uint8_t hflipped = texture >> 15;
    if (!hflipped) {
        penwrite = OFFBUFFER + (imult * i) + (8 * j);
        for (int s = 0; s < 8; s++) {
            beebram[penwrite + s] &= (beebram[mask + s] ^ 0xFF);
            beebram[penwrite + s] |= (beebram[texture + s] & beebram[mask + s]);
        }
    }

    else {
        texture &= 0x7FFF;
        mask &= 0x7FFF;
        penwrite = OFFBUFFER + (imult * i) + (8 * j);
        for (int s = 0; s < 8; s++) {
            uint8_t mask_data = beebram[mask + s];
            uint8_t texture_data = beebram[texture + s];
            beebram[penwrite + s] &= (beebram[LUT_REVERSE + mask_data] ^ 0xFF);
            beebram[penwrite + s] |= (beebram[LUT_REVERSE + texture_data] & beebram[LUT_REVERSE + mask_data]);
        }
    }
}

/*----------------------------------------------------------------------------*/

// copies any statik cells overlapping a movable entity into the offbuffer
void statiks2container(uint16_t pentity) {
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
            uint16_t texture, mask;
            if (qi == pi + 2) {
                if (qj == pj + 2) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 2, texture, mask, 3);
                }

                if (qj == pj + 1) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 1, texture, mask, 3);
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 2, texture, mask, 3);
                }

                if (qj == pj) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 0, texture, mask, 3);
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 1, texture, mask, 3);
                }

                if (qj == pj - 1) {
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 0, texture, mask, 3);
                }
            }

            if (qi == pi + 1) {

                if (qj == pj + 2) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 2, texture, mask, 3);
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 2, texture, mask, 3);
                }

                if (qj == pj + 1) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 1, texture, mask, 3);
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 2, texture, mask, 3);
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 1, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 2, texture, mask, 3);
                }

                if (qj == pj) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 0, texture, mask, 3);
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 1, texture, mask, 3);
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 0, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 1, texture, mask, 3);
                }

                if (qj == pj - 1) {
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 0, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(2, 0, texture, mask, 3);
                }
            }

            if (qi == pi) {
                if (qj == pj + 2) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 2, texture, mask, 3);
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 2, texture, mask, 3);
                }
                if (qj == pj + 1) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 1, texture, mask, 3);
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 2, texture, mask, 3);
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 1, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 2, texture, mask, 3);
                }
                if (qj == pj) {
                    getTextureAndMask(0, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 0, texture, mask, 3);
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 1, texture, mask, 3);
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 0, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 1, texture, mask, 3);
                }
                if (qj == pj - 1) {
                    getTextureAndMask(0, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 0, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(1, 0, texture, mask, 3);
                }
            }

            if (qi == pi - 1) {
                if (qj == pj + 2) {
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 2, texture, mask, 3);
                }
                if (qj == pj + 1) {
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 1, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 2, texture, mask, 3);
                }
                if (qj == pj) {
                    getTextureAndMask(1, 0, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 0, texture, mask, 3);
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 1, texture, mask, 3);
                }
                if (qj == pj - 1) {
                    getTextureAndMask(1, 1, pquad, &texture, &mask);
                    bufferTextureAndMask(0, 0, texture, mask, 3);
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------*/

// renders to framebuffer all statiks held in camera that are marked for redraw
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
                    uint16_t texture, mask;
                    getTextureAndMask(i, j, pvizdef, &texture, &mask);
                    bufferTextureAndMask(i, j, texture, mask, 2);
                }
            }
            renderOffbuffer(qi, qj, 2);
        }
    }
}

/*----------------------------------------------------------------------------*/

// renders players to framebuffer
void renderPlayer() {
    if (beebram[PLAYER + CEF_ROOMID6_REDRAW2] & CEC_CLEAN != 0) {
        renderCleanup(PLAYER);
    }

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

// renders portion of the offbuffer to screen at screen i,j
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

    uint8_t xmag = (beebram[PLAYER + MEF_XMD4_YMD4] >> 4) >> 2;
    uint8_t xdir = (beebram[PLAYER + MEF_XMD4_YMD4] >> 4) & 0b11;
    uint8_t ymag = (beebram[PLAYER + MEF_XMD4_YMD4] & 0x0F) >> 2;
    uint8_t ydir = (beebram[PLAYER + MEF_XMD4_YMD4] & 0x0F) & 0b11;

    uint8_t old_i = beebram[pentity + MEF_OLDI];
    uint8_t old_j = beebram[pentity + MEF_OLDJ];

    // moving up, clear below
    if (ymag != 0) {
        if (ydir == DIR_UP) {
            fprintf(stderr, "\nCLEAN: MOVING UP, CLEAR BELOW");
            renderBGTile(old_i + 2, old_j + 0);
            renderBGTile(old_i + 2, old_j + 1);
            renderBGTile(old_i + 2, old_j + 2);
        }

        // moving down, clean above
        if (ydir == DIR_DOWN) {
            fprintf(stderr, "\nCLEAN: MOVING DOWN, CLEAR ABOVE");
            renderBGTile(old_i, old_j + 0);
            renderBGTile(old_i, old_j + 1);
            renderBGTile(old_i, old_j + 2);
        }
    }

    if (xmag != 0) {
        // moving left, clear right
        if (xdir == DIR_LEFT) {
            fprintf(stderr, "\nCLEAN: MOVING LEFT, CLEAR RIGHT");
            renderBGTile(old_i + 0, old_j + 2);
            renderBGTile(old_i + 1, old_j + 2);
            renderBGTile(old_i + 2, old_j + 2);
        }

        // moving right, clear left
        if (xdir == DIR_RIGHT) {
            fprintf(stderr, "\nCLEAN: MOVING RIGHT, CLEAR LEFT");
            renderBGTile(old_i + 0, old_j);
            renderBGTile(old_i + 1, old_j);
            renderBGTile(old_i + 2, old_j);
        }
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