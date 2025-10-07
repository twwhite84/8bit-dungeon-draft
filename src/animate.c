#include "animate.h"
#include "shared.h"
#include <stdio.h>

void animateEntity(uint16_t pentity) {

    // vizbase refers to an animdef table entry
    uint16_t pvizbase = beebram[pentity + CE_PVIZBASE_LO] | (beebram[pentity + CE_PVIZBASE_HI] << 8);
    if (pvizbase < AD_TABLE)
        return;

    uint8_t animset = 0;
    if (pentity >= PLAYER)
        animset = beebram[pentity + ME_ANIMSET];

    uint16_t panimdef = beebram[pvizbase + animset] | (beebram[pvizbase + animset + 1] << 8);

    // movables should have their animdef updated to match the current direction of travel
    if (pentity >= PLAYER) {
        uint8_t dir_x = (beebram[pentity + ME_DIRX4_DIRY4] >> 4) & 0b11;
        uint8_t dir_y = (beebram[pentity + ME_DIRX4_DIRY4] & 0x0F) & 0b11;

        if (dir_y == DIR_NEGATIVE && animset != ANIMSET_WALKU) {
            animset = ANIMSET_WALKU;
            panimdef = beebram[pvizbase + animset] | (beebram[pvizbase + animset + 1] << 8);
        }

        if (dir_y == DIR_POSITIVE && animset != ANIMSET_WALKD) {
            animset = ANIMSET_WALKD;
            panimdef = beebram[pvizbase + animset] | (beebram[pvizbase + animset + 1] << 8);
        }

        if (dir_x == DIR_NEGATIVE && animset != ANIMSET_WALKL) {
            animset = ANIMSET_WALKL;
            panimdef = beebram[pvizbase + animset] | (beebram[pvizbase + animset + 1] << 8);
        }

        if (dir_x == DIR_POSITIVE && animset != ANIMSET_WALKR) {
            animset = ANIMSET_WALKR;
            panimdef = beebram[pvizbase + animset] | (beebram[pvizbase + animset + 1] << 8);
        }

        beebram[pentity + ME_ANIMSET] = animset;
    }

    uint8_t elapsed = beebram[pentity + CE_FELAPSED5_FCURRENT3] >> 3;
    uint8_t current = beebram[pentity + CE_FELAPSED5_FCURRENT3] & 0b111;
    uint8_t frames = beebram[panimdef + AD_FRAMES4_YOYO4] >> 4;
    uint8_t yoyo = beebram[panimdef + AD_FRAMES4_YOYO4] & 0x0F;
    uint8_t period;

    // update the elapsed frame count
    elapsed++;
    beebram[pentity + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
    beebram[pentity + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

    // fetch the period for the current frame index
    switch (current) {
    case 0:
        period = (beebram[panimdef + AD_PERIODA4_PERIODB4] & 0b11110000) >> 4;
        break;

    case 1:
        period = (beebram[panimdef + AD_PERIODA4_PERIODB4] & 0b00001111);
        break;

    case 2:
        period = (beebram[panimdef + AD_PERIODC4_PERIODD4] & 0b11110000) >> 4;
        break;

    case 3:
        period = (beebram[panimdef + AD_PERIODC4_PERIODD4] & 0b00001111);
        break;
    }

    // if the elapsed frame count > period, cycle the frame and reset the elapsed
    if (elapsed > period) {

        (yoyo == YOYO_OFF || yoyo == YOYO_FORWARD) ? current++ : current--;

        // current is beyond end of animation sequence
        if (current >= frames) {
            if (yoyo == YOYO_OFF)
                current = 0;
            if (yoyo == YOYO_FORWARD) {
                current -= 2;
                yoyo = YOYO_BACKWARD;
            }
        }

        // current is before start of animation sequence ($80 to &FF are - in uint8)
        if (current >= 0x80) {
            current += 2;
            yoyo = YOYO_FORWARD;
        }

        // save current
        beebram[pentity + CE_FELAPSED5_FCURRENT3] &= 0b11111000;
        beebram[pentity + CE_FELAPSED5_FCURRENT3] |= current;

        // save yoyo
        beebram[panimdef + AD_FRAMES4_YOYO4] &= 0xF0;
        beebram[panimdef + AD_FRAMES4_YOYO4] |= yoyo;

        // write elapsed frames
        elapsed = 0;
        beebram[pentity + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
        beebram[pentity + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

        // raise redraw flag so that renderStatics() draws it
        beebram[pentity + CE_ROOMID6_CLEAN1_REDRAW1] |= 1;
    }
}

/*----------------------------------------------------------------------------*/

// walk the camera buffer and update any static entities which have animated tiles
void animateStatics() {
    uint16_t pstart = CAMERA + CAM_PSE0_LO;
    for (uint8_t i = 0; i < 10; i++) {
        uint16_t pstatic = beebram[pstart] | (beebram[pstart + 1] << 8);
        pstart += 2;
        if (pstatic == 0xFFFF)
            break;
        animateEntity(pstatic);
    }
}