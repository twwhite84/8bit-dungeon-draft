#include "player.h"
#include "camera.h"
#include "renderer.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void checkAxis(uint16_t x1, uint16_t y1, uint8_t dir, uint16_t *collisions);
uint8_t checkBorderCollision(uint16_t x1, uint16_t y1);
void checkStaticCollisions(uint16_t x1, uint16_t y1, uint16_t *collisions, uint8_t *insertion_index);
void handleCollisions(uint16_t p0, uint16_t *p1, uint16_t *collisions);

/*----------------------------------------------------------------------------*/

// returns which border has been touched, or beebnull if border not touched
uint8_t checkBorderCollision(uint16_t x1, uint16_t y1) {
    uint8_t current_room = beebram[CAMERA + CAM_ROOMID];
    // crossing at screen top
    if (y1 == 0) {
        return DIR_UP;
    }
    // crossing at screen bottom
    if (y1 == (CAMERA_HEIGHT - 16 - 1)) {
        return DIR_DOWN;
    }
    // crossing at screen left
    if (x1 == 0) {
        return DIR_LEFT;
    }
    // crossing at screen right
    if (x1 == (CAMERA_WIDTH - 16)) {
        return DIR_RIGHT;
    }
    return SENTINEL8;
}

/*----------------------------------------------------------------------------*/

void movePlayer() {

    // get current bearing
    uint8_t xmag = (beebram[PLAYER + ME_XMD4_YMD4] >> 4) >> 2;
    uint8_t xdir = (beebram[PLAYER + ME_XMD4_YMD4] >> 4) & 0b11;
    uint8_t ymag = (beebram[PLAYER + ME_XMD4_YMD4] & 0x0F) >> 2;
    uint8_t ydir = (beebram[PLAYER + ME_XMD4_YMD4] & 0x0F) & 0b11;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);

    // get target coordinates
    uint16_t x1 = x0;
    uint16_t y1 = y0;
    if (xmag > 0) {
        x1 += (xdir == DIR_LEFT) ? -xmag : xmag;
        if (x1 >= 0x8000) // too far left
            x1 = 0;
        if (x1 > (CAMERA_WIDTH - 16))
            x1 = (CAMERA_WIDTH - 16);
    }

    if (ymag > 0) {
        y1 += (ydir == DIR_UP) ? -ymag : ymag;
        if (y1 >= 0x8000)
            y1 = 0;
        if (y1 > (CAMERA_HEIGHT - 16 - 1)) // -1 to prevent sprite container
            y1 = (CAMERA_HEIGHT - 16 - 1); //  updating beyond screen bottom
    }

    // check target for border crossings
    uint8_t current_room = beebram[CAMERA + CAM_ROOMID];
    uint8_t border_collision = checkBorderCollision(x1, y1);
    uint8_t exit_room =
        (border_collision != SENTINEL8) ? beebram[ROOMS + (current_room * 4) + border_collision] : SENTINEL8;

    if (exit_room != SENTINEL8) {
        uint8_t margin = 2;
        if (border_collision == DIR_UP) {
            y1 = (CAMERA_HEIGHT - 16 - margin);
        }
        if (border_collision == DIR_DOWN) {
            y1 = margin;
        }
        if (border_collision == DIR_LEFT) {
            x1 = margin;
        }
        if (border_collision == DIR_RIGHT) {
            x1 = (CAMERA_WIDTH - 16 - margin);
        }

        loadRoom(exit_room);
        renderBackground();
        renderStatics();
        goto save;
    }

    // check player path for walls or statics
    uint16_t collisions[4];
    if (xmag > 0) {
        for (uint8_t i = 0; i < 4; i++)
            collisions[i] = SENTINEL16;
        checkAxis(x1, y0, xdir, collisions);
        handleCollisions(x0, &x1, collisions);
    }

    if (ymag > 0) {
        // note: passing x1 here helps avoid some sticking when moving diagonally
        for (uint8_t i = 0; i < 4; i++)
            collisions[i] = SENTINEL16;
        checkAxis(x1, y1, ydir, collisions);
        handleCollisions(y0, &y1, collisions);
    }

save:
    // save the updated coordinates
    beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
    beebram[PLAYER + ME_X_HI] = x1 >> 8;
    beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
    beebram[PLAYER + ME_Y_HI] = y1 >> 8;

    // update the sprite container with its new coordinates
    updateSpriteContainer(PLAYER);

    // raise the redraw flag to let renderer know movement has taken place
    // beebram[PLAYER + CE_ROOMID6_CLEAN1_REDRAW1] |= true;
    beebram[CAMERA + CAM_REDRAW] |= REDRAW_PLAYER;
}

/*----------------------------------------------------------------------------*/

void handleCollisions(uint16_t p0, uint16_t *p1, uint16_t *collisions) {

    for (uint8_t i = 0; i < 4; i++) {
        if (collisions[i] == SENTINEL16) {
            return;
        }
        if (collisions[i] == 0) { // wall
            fprintf(stderr, "\nWALL");
            *p1 = p0;
            return; // not sure if i want this here
        }
        if (collisions[i] >= SE_DEFS && collisions[i] < PLAYER) {
            uint8_t se_type = beebram[collisions[i] + SE_TYPE4_NQUADS4] >> 4;

            if (se_type == SETYPE_DOORLOCKED) {
                fprintf(stderr, "\nLOCKED DOOR");
                *p1 = p0;
            }
            if (se_type == SETYPE_PICKUP) {
                fprintf(stderr, "\nPICKUP");

                // copy the pickup into player inventory
                beebram[PLAYER + PLR_PINVA_LO] = collisions[i] & 0xFF;
                beebram[PLAYER + PLR_PINVA_HI] = collisions[i] >> 8;

                // set static roomID to not be in any room
                beebram[collisions[i] + CE_ROOMID6_CLEAN1_REDRAW1] = SENTINEL8;

                // refresh the statics for the room
                loadStatics(beebram[CAMERA + CAM_ROOMID]);

                // re-render the background
                beebram[CAMERA + CAM_REDRAW] |= REDRAW_BG;
                // renderBackground();

                // re-render the statics
                beebram[CAMERA + CAM_REDRAW] |= REDRAW_STATICS;
                // renderStatics();
            }
        }
    }
}

/*----------------------------------------------------------------------------*/

void checkAxis(uint16_t x1, uint16_t y1, uint8_t dir, uint16_t *collisions) {
    uint8_t insertion_index = 0;

    /* -------------------- WALLS CHECK --------------------*/

    uint8_t i1 = y1 >> 3;
    uint8_t j1 = x1 >> 3;
    uint8_t h1 = x1 & 0b111;
    uint8_t v1 = y1 & 0b111;
    bool wall_x = false, wall_y = false; // helps prevent redundant checks

    // we only need to check 2 rows/columns when shifts are at 0
    uint8_t ilimit, jlimit;
    if (dir == DIR_UP || dir == DIR_DOWN) {
        ilimit = 3;
        jlimit = (h1 == 0) ? 2 : 3;
    }
    if (dir == DIR_LEFT || dir == DIR_RIGHT) {
        ilimit = (v1 == 0) ? 2 : 3;
        jlimit = 3;
    }

    for (uint8_t i = i1; i < (i1 + ilimit); i++) {
        for (uint8_t j = j1; j < (j1 + jlimit); j++) {
            uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
            if (tid >= TID_WALLS) {

                // exception for h1/v1 == 0 to allow a movement to a grid-snapped location
                // this permits subsequent movement on the other axis through tight gaps
                if (dir == DIR_UP || dir == DIR_DOWN) {
                    if (!wall_y && v1 > 0) {
                        wall_y = true;
                        collisions[insertion_index++] = 0;
                    }
                }

                if (dir == DIR_LEFT || dir == DIR_RIGHT) {
                    if (!wall_x && h1 > 0) {
                        wall_x = true;
                        collisions[insertion_index++] = 0;
                    }
                }
            }
        }
    }

    /* -------------------- STATICS CHECK --------------------*/
    checkStaticCollisions(x1, y1, collisions, &insertion_index);
}

/*----------------------------------------------------------------------------*/

// returns code for the type involved in the closest collision, or else -1
void checkStaticCollisions(uint16_t x1, uint16_t y1, uint16_t *collisions, uint8_t *insertion_index) {

    uint8_t i1 = y1 >> 3;
    uint8_t j1 = x1 >> 3;
    uint16_t pse_base = CAMERA + CAM_PSE0_LO;

    for (uint8_t static_index = 0; static_index < 20; static_index += 2) {

        uint16_t pse = beebram[pse_base + static_index] | (beebram[pse_base + static_index + 1] << 8);
        if (pse == 0xFFFF)
            break;

        uint8_t static_type = beebram[pse + SE_TYPE4_NQUADS4] >> 4;
        uint8_t nquads = beebram[pse + SE_TYPE4_NQUADS4] & 0x0F;

        for (uint8_t q = 0; q < nquads; q++) {

            // does the movable's container overlap the static's quad(s)?
            uint8_t se_i = beebram[pse + CE_I + (4 * q)];
            uint8_t se_j = beebram[pse + CE_J + (4 * q)];
            uint8_t collision_intercepts = 0, redraw_intercepts = 0;

            uint8_t idelta = (se_i - i1);
            uint8_t jdelta = (se_j - j1);
            if (idelta >= 0x80)
                idelta = (idelta ^ 0xFF) + 1;
            if (jdelta >= 0x80)
                jdelta = (jdelta ^ 0xFF) + 1;

            // redraw is triggered by a wider boundary than collision
            // to ensure statics are redrawn when pulling away
            if (idelta < 4) {
                redraw_intercepts++;
                if (idelta < 3)
                    collision_intercepts++;
            }
            if (jdelta < 4) {
                redraw_intercepts++;
                if (jdelta < 3)
                    collision_intercepts++;
            }

            if (redraw_intercepts == 2)
                beebram[pse + CE_ROOMID6_CLEAN1_REDRAW1] |= 1;
            if (collision_intercepts != 2)
                continue; // no: skip to the next static quad, if any exists

            // yes the sprite container overlaps with static, so check for collision
            uint16_t qx = beebram[pse + CE_J + (4 * q)] << 3;
            uint16_t qy = beebram[pse + CE_I + (4 * q)] << 3;

            uint8_t xdelta_current = x1 - qx;
            xdelta_current = (xdelta_current >= 0x80) ? (xdelta_current ^ 0xFF) + 1 : xdelta_current;
            uint8_t ydelta_current = y1 - qy;
            ydelta_current = (ydelta_current >= 0x80) ? (ydelta_current ^ 0xFF) + 1 : ydelta_current;

            if (xdelta_current < 16 && ydelta_current < 16) {
                collisions[(*insertion_index)++] = pse;
            }
        }
    }
}
