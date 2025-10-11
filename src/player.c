#include "player.h"
#include "camera.h"
#include "renderer.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t checkStaticCollisions(uint16_t pmovable, uint16_t mov_x, uint16_t mov_y);
uint8_t checkAxis(uint16_t x1, uint16_t y1, uint8_t dir, uint8_t speed, uint8_t axis);

/*----------------------------------------------------------------------------*/

void movePlayer() {

    // get current bearing
    uint8_t xdir = (beebram[PLAYER + ME_DIRX4_DIRY4] >> 4) & 0b11;
    uint8_t xspeed = (beebram[PLAYER + ME_DIRX4_DIRY4] >> 4) >> 2;
    uint8_t ydir = (beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F) & 0b11;
    uint8_t yspeed = (beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F) >> 2;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8);
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8);

    // if player is at a screen edge, abort movement
    // current room code
    uint8_t current_room = beebram[CAMERA + CAM_ROOMID];
    uint8_t exit_to = 0xFF, exit_dir = 0xFF;

    if (y0 < 2 && ydir == DIR_NEGATIVE) { // up
        exit_dir = ROOMEXIT_U;
        exit_to = beebram[ROOMS + (current_room * 4) + exit_dir];
        ydir = DIR_ZERO;
    }
    if (y0 > (CAMERA_HEIGHT - 19) && ydir == DIR_POSITIVE) { // down
        exit_dir = ROOMEXIT_D;
        exit_to = beebram[ROOMS + (current_room * 4) + exit_dir];
        ydir = DIR_ZERO;
    }
    if (x0 < 1 && xdir == DIR_NEGATIVE) // left
        xdir = DIR_ZERO;
    if (x0 > (CAMERA_WIDTH - 18) && xdir == DIR_POSITIVE) // right
        xdir = DIR_ZERO;

    if (exit_to != 0xFF) {
        // moving up means player I should be set to screen bottom
        if (exit_dir == ROOMEXIT_U) {
            beebram[PLAYER + ME_Y_LO] = (CAMERA_HEIGHT - 16 - 1) & 0xFF;
            beebram[PLAYER + ME_Y_HI] = (CAMERA_HEIGHT - 16 - 1) >> 8;
        }
        if (exit_dir == ROOMEXIT_D) {
            beebram[PLAYER + ME_Y_LO] = 0;
            beebram[PLAYER + ME_Y_HI] = 0;
        }
        if (exit_dir == ROOMEXIT_L) {
            beebram[PLAYER + ME_X_LO] = 0;
            beebram[PLAYER + ME_X_HI] = 0;
        }
        if (exit_dir == ROOMEXIT_R) {
            beebram[PLAYER + ME_X_LO] = (320 - 16) & 0xFF;
            beebram[PLAYER + ME_X_HI] = (320 - 16) >> 8;
        }

        updateSpriteContainer(PLAYER);
        beebram[PLAYER + CE_ROOMID6_CLEAN1_REDRAW1] |= 0b1;
        loadRoom(exit_to);
        renderCambuffer();
        renderStatics();
        return;
    }

    // check player path for walls or statics
    uint16_t x1 = x0, y1 = y0;
    if (xdir != DIR_ZERO) {
        x1 += (xdir == DIR_NEGATIVE) ? -xspeed : xspeed; // x1 to check
        uint8_t collision_type = checkAxis(x1, y0, xdir, xspeed, X_AXIS);
        if (collision_type != OBSTACLE_NONE) {
            x1 = x0;
        }
    }

    if (ydir != DIR_ZERO) {
        y1 += (ydir == DIR_NEGATIVE) ? -yspeed : yspeed; // y1 to check
        // note: passing x1 here helps avoid some sticking when moving diagonally
        uint8_t collision_type = checkAxis(x1, y1, ydir, yspeed, Y_AXIS);
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
    uint8_t collision_type = checkStaticCollisions(PLAYER, x1, y1);
    switch (collision_type) {
    case OBSTACLE_NONE:
        break;
    case SETYPE_DOORLOCKED:
        if (axis == X_AXIS) {
            if (dir != DIR_ZERO) {
                fprintf(stderr, "\nLOCKED DOOR on X-axis ");
                (dir == DIR_NEGATIVE) ? fprintf(stderr, " -") : fprintf(stderr, " +");
                return OBSTACLE_WALL + SETYPE_DOORLOCKED;
            }
        } else {
            if (dir != DIR_ZERO) {
                fprintf(stderr, "\nLOCKED DOOR on Y-axis ");
                (dir == DIR_NEGATIVE) ? fprintf(stderr, " -") : fprintf(stderr, " +");
                return OBSTACLE_WALL + SETYPE_DOORLOCKED;
            }
        }
    }

    return OBSTACLE_NONE;
}

/*----------------------------------------------------------------------------*/

// returns code for the type involved in the collision, or else -1
uint8_t checkStaticCollisions(uint16_t pmovable, uint16_t mov_x, uint16_t mov_y) {

    uint8_t movable_i = mov_y >> 3;
    uint8_t movable_j = mov_x >> 3;

    uint16_t pse_base = CAMERA + CAM_PSE0_LO;
    for (uint8_t idx = 0; idx < 20; idx += 2) {

        uint16_t pse = beebram[pse_base + idx] | (beebram[pse_base + idx + 1] << 8);
        if (pse == 0xFFFF)
            break;

        uint8_t static_type = beebram[pse + SE_TYPE4_NQUADS4] >> 4;
        uint8_t nquads = beebram[pse + SE_TYPE4_NQUADS4] & 0x0F;

        for (uint8_t q = 0; q < nquads; q++) {

            // does the movable's container overlap the static's quad(s)?
            uint8_t se_i = beebram[pse + CE_I + (4 * q)];
            uint8_t se_j = beebram[pse + CE_J + (4 * q)];
            uint8_t collision_intercepts = 0, redraw_intercepts = 0;

            uint8_t idelta = (se_i - movable_i);
            uint8_t jdelta = (se_j - movable_j);
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

            // yes the sprite container overlaps, so check for collision
            uint16_t qx = beebram[pse + CE_J + (4 * q)] << 3;
            uint16_t qy = beebram[pse + CE_I + (4 * q)] << 3;

            uint8_t xdelta = mov_x - qx;
            xdelta = (xdelta >= 0x80) ? (xdelta ^ 0xFF) + 1 : xdelta;
            uint8_t ydelta = mov_y - qy;
            ydelta = (ydelta >= 0x80) ? (ydelta ^ 0xFF) + 1 : ydelta;

            if (xdelta < 16 && ydelta < 16)
                return static_type;
        }
    }

    return OBSTACLE_NONE;
}
