#include "player.h"
#include "camera.h"
#include "renderer.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void checkStaticHits(uint16_t x1, uint16_t y1, uint16_t *static_hits_out);
uint8_t checkAxis(uint16_t x1, uint16_t y1, uint8_t dir, uint8_t speed, uint8_t axis);
uint8_t checkBorderCollision(uint16_t x1, uint16_t y1);

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
    return BEEBNULL;
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
        (border_collision != BEEBNULL) ? beebram[ROOMS + (current_room * 4) + border_collision] : BEEBNULL;

    if (exit_room != BEEBNULL) {
        uint8_t margin = 2;
        if (border_collision == DIR_UP) {
            beebram[PLAYER + ME_Y_LO] = (CAMERA_HEIGHT - 16 - margin) & 0xFF;
            beebram[PLAYER + ME_Y_HI] = (CAMERA_HEIGHT - 16 - margin) >> 8;
        }
        if (border_collision == DIR_DOWN) {
            beebram[PLAYER + ME_Y_LO] = margin;
            beebram[PLAYER + ME_Y_HI] = 0;
        }
        if (border_collision == DIR_LEFT) {
            beebram[PLAYER + ME_X_LO] = margin;
            beebram[PLAYER + ME_X_HI] = 0;
        }
        if (border_collision == DIR_RIGHT) {
            beebram[PLAYER + ME_X_LO] = (CAMERA_WIDTH - 16 - margin) & 0xFF;
            beebram[PLAYER + ME_X_HI] = (CAMERA_WIDTH - 16 - margin) >> 8;
        }

        updateSpriteContainer(PLAYER);
        beebram[PLAYER + CE_ROOMID6_CLEAN1_REDRAW1] |= 0b1;
        loadRoom(exit_room);

        renderCambuffer();
        renderStatics();
        return;
    }

    // check player path for walls or statics
    uint8_t collision_type = OBSTACLE_NONE;
    if (xmag > 0) {
        collision_type = checkAxis(x1, y0, xdir, xmag, X_AXIS);
        if (collision_type != OBSTACLE_NONE) {
            x1 = x0;
        }
    }

    if (ymag > 0) {
        // note: passing x1 here helps avoid some sticking when moving diagonally
        collision_type = checkAxis(x1, y1, ydir, ymag, Y_AXIS);
        if (collision_type != OBSTACLE_NONE) {
            y1 = y0;
        }
    }

    // save the updated coordinates
    beebram[PLAYER + ME_X_LO] = x1 & 0xFF;
    beebram[PLAYER + ME_X_HI] = x1 >> 8;
    beebram[PLAYER + ME_Y_LO] = y1 & 0xFF;
    beebram[PLAYER + ME_Y_HI] = y1 >> 8;

    // update the sprite container with its new coordinates
    updateSpriteContainer(PLAYER);

    // raise the redraw flag to let renderer know movement has taken place
    beebram[PLAYER + CE_ROOMID6_CLEAN1_REDRAW1] |= true;
}

/*----------------------------------------------------------------------------*/

// the reason for a separate dir and speed is i can't pack a negative value into 4 bits
uint8_t checkAxis(uint16_t x1, uint16_t y1, uint8_t dir, uint8_t speed, uint8_t axis) {
    uint8_t i1 = y1 >> 3;
    uint8_t j1 = x1 >> 3;
    uint8_t h1 = x1 & 0b111;
    uint8_t v1 = y1 & 0b111;

    // we only need to check 2 rows/columns when shifts are at 0
    uint8_t ilimit, jlimit;
    if (axis == X_AXIS) {
        ilimit = (v1 == 0) ? 2 : 3;
        jlimit = 3;
    } else {
        ilimit = 3;
        jlimit = (h1 == 0) ? 2 : 3;
    }

    /* -------------------- WALLS CHECK --------------------*/
    for (uint8_t i = i1; i < (i1 + ilimit); i++) {
        for (uint8_t j = j1; j < (j1 + jlimit); j++) {
            uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
            if (tid >= TID_WALLS) {

                // exception for h1/v1 == 0 to allow a movement to a grid-snapped location
                // this permits subsequent movement on the other axis through tight gaps
                if (axis == X_AXIS && h1 > 0) {
                    return OBSTACLE_WALL;
                }

                else if (axis == Y_AXIS && v1 > 0) {
                    return OBSTACLE_WALL;
                }
            }
        }
    }

    /* -------------------- STATICS CHECK --------------------*/
    uint16_t static_hits[2] = {0xFFFF, 0xFFFF}; // handles up to 2
    checkStaticHits(x1, y1, static_hits);
    for (uint8_t i = 0; i < 2; i++) {
        uint16_t pse = static_hits[i];
        if (pse == 0xFFFF)
            break;
        uint8_t pse_type = beebram[pse + SE_TYPE4_NQUADS4] >> 4;
        switch (pse_type) {
        case OBSTACLE_NONE:
            break;
        case SETYPE_DOORLOCKED:
            if (axis == X_AXIS) {
                fprintf(stderr, "\nLOCKED DOOR on X-axis ");
                (dir == DIR_LEFT) ? fprintf(stderr, " -") : fprintf(stderr, " +");
                return OBSTACLE_WALL + SETYPE_DOORLOCKED;
            } else {
                fprintf(stderr, "\nLOCKED DOOR on Y-axis ");
                (dir == DIR_UP) ? fprintf(stderr, " -") : fprintf(stderr, " +");
                return OBSTACLE_WALL + SETYPE_DOORLOCKED;
            }
            break;
        case SETYPE_PICKUP:
            if (axis == X_AXIS) {
                fprintf(stderr, "\nPICKUP on X-axis ");
                (dir == DIR_LEFT) ? fprintf(stderr, " -") : fprintf(stderr, " +");
                return OBSTACLE_WALL + SETYPE_PICKUP;
            } else {
                fprintf(stderr, "\nPICKUP on Y-axis ");
                (dir == DIR_UP) ? fprintf(stderr, " -") : fprintf(stderr, " +");
                return OBSTACLE_WALL + SETYPE_PICKUP;
            }
            break;
        }

        return OBSTACLE_NONE;
    }
}

/*----------------------------------------------------------------------------*/

// returns code for the type involved in the closest collision, or else -1
void checkStaticHits(uint16_t x1, uint16_t y1, uint16_t *static_hits_out) {

    uint8_t i1 = y1 >> 3;
    uint8_t j1 = x1 >> 3;
    uint16_t pse_base = CAMERA + CAM_PSE0_LO;
    uint8_t insert_idx = 0;

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
                static_hits_out[insert_idx++] = pse;
            }
        }
    }
}
