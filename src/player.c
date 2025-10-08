#include "player.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>
#include <stdio.h>

uint8_t checkStaticCollisions(uint16_t pmovable, uint16_t mov_x, uint16_t mov_y);

/*----------------------------------------------------------------------------*/

// this is called when a request for movement is made
void movePlayer() {

    // get current bearing
    uint8_t xdir = (beebram[PLAYER + ME_DIRX4_DIRY4] >> 4) & 0b11;
    uint8_t xboost = (beebram[PLAYER + ME_DIRX4_DIRY4] >> 4) >> 2;
    uint8_t ydir = (beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F) & 0b11;
    uint8_t yboost = (beebram[PLAYER + ME_DIRX4_DIRY4] & 0x0F) >> 2;

    // get the current coordinates
    uint16_t x0 = beebram[PLAYER + ME_X_LO] | (beebram[PLAYER + ME_X_HI] << 8), x1 = x0;
    uint16_t y0 = beebram[PLAYER + ME_Y_LO] | (beebram[PLAYER + ME_Y_HI] << 8), y1 = y0;

    // if player is at a screen edge, abort movement
    if (y1 < 2 && ydir == DIR_NEGATIVE)
        ydir = DIR_ZERO;
    if (y1 > (CAMERA_HEIGHT - 19) && ydir == DIR_POSITIVE)
        ydir = DIR_ZERO;
    if (x1 < 1 && xdir == DIR_NEGATIVE)
        xdir = DIR_ZERO;
    if (x1 > (CAMERA_WIDTH - 18) && xdir == DIR_POSITIVE)
        xdir = DIR_ZERO;

    // don't move player if path blocked by a wall
    {
    movex:
        if (xdir != DIR_ZERO) {
            x1 = (xdir == DIR_NEGATIVE) ? (x1 - 1 - xboost) : (x1 + 1 + xboost); // potential target x
            uint8_t i1 = y1 >> 3;
            uint8_t j1 = x1 >> 3;
            uint8_t hshift1 = x1 & 0b111;
            uint8_t vshift1 = y1 & 0b111;

            // when target is flush with container top, dont check bottom container row
            // this is the only case where a player can travel x through player-sized gaps
            uint8_t ilimit = (vshift1 == 0) ? 2 : 3;

            // check for presence of a wall tile in the 2x3 or 3x3 container space
            for (uint8_t i = i1; i < (i1 + ilimit); i++) {
                for (uint8_t j = j1; j < (j1 + 3); j++) {
                    uint8_t tid = beebram[CAMBUFFER + 40 * i + j];

                    if (tid >= TID_WALLS) {

                        // stop x movement if wall present, unless moving to a grid-aligned position
                        // (this exception allows subsequent y movement through player-wide gaps)
                        if (hshift1 != 0) {
                            x1 = x0;
                        }
                    }
                }
            }
        }

    movey:
        if (ydir != DIR_ZERO) {
            y1 = (ydir == DIR_NEGATIVE) ? (y1 - 1 - yboost) : (y1 + 1 + yboost); // potential movement
            uint8_t i1 = y1 >> 3;
            uint8_t j1 = x1 >> 3;
            uint8_t hshift1 = x1 & 0b111;
            uint8_t vshift1 = y1 & 0b111;
            uint8_t jlimit = (hshift1 == 0) ? 2 : 3;

            for (uint8_t i = i1; i < (i1 + 3); i++) {
                for (uint8_t j = j1; j < (j1 + jlimit); j++) {
                    uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
                    if (tid >= TID_WALLS) {

                        // stop y movement if wall present, unless moving to a grid-aligned position
                        // (this exception allows subsequent x movement through player-height gaps)
                        if (vshift1 != 0) {
                            y1 = y0;
                        }
                    }
                }
            }
        }
    }

check_se:
    uint8_t collision_type = checkStaticCollisions(PLAYER, x1, y1);
    switch (collision_type) {
    case 0xFF:
        break;
    case SETYPE_DOORLOCKED:
        if (xdir != DIR_ZERO)
            x1 = x0;
        if (ydir != DIR_ZERO)
            y1 = y0;
        break;
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
    beebram[PLAYER + CE_ROOMID6_CLEAN1_REDRAW1] |= true;
}

/*----------------------------------------------------------------------------*/

// returns code for the type involved in the collision, or else -1
uint8_t checkStaticCollisions(uint16_t pmovable, uint16_t mov_x, uint16_t mov_y) {

    uint8_t static_type = 0xFF; // no type
    uint8_t movable_i = mov_y >> 3;
    uint8_t movable_j = mov_x >> 3;

    uint16_t pse_base = CAMERA + CAM_PSE0_LO;
    for (uint8_t idx = 0; idx < 20; idx += 2) {

        uint16_t pse = beebram[pse_base + idx] | (beebram[pse_base + idx + 1] << 8);
        if (pse == 0xFFFF)
            break;

        uint8_t type = beebram[pse + SE_TYPE4_NQUADS4] >> 4;
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
            // this ensure statics are redrawn when pulling away
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

            // yes: mark the static for redraw and check for collision
            uint16_t qx = beebram[pse + CE_J + (4 * q)] << 3;
            uint16_t qy = beebram[pse + CE_I + (4 * q)] << 3;

            collision_intercepts = 0;
            if (abs(mov_x - qx) < 16 && abs(mov_y - qy) < 16)
                collision_intercepts++;
            if (abs((mov_x + 16) - (qx + 16)) < 16 && abs((mov_y + 16) - (qy + 16)) < 16)
                collision_intercepts++;

            if (collision_intercepts == 2) {
                static_type = type;
            }
        }
    }

    return static_type;
}
