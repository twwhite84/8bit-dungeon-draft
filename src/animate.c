#include "animate.h"
#include "shared.h"
#include <stdio.h>

void animateEntity(uint16_t pentity) {
    uint16_t pvizdef = beebram[pentity + CE_PVIZDEF_LO] + (beebram[pentity + CE_PVIZDEF_HI] << 8);
    if (pvizdef < ANIMDEFS)
        return;

    // movables should have their animdef updated to match the current direction of travel
    if (pentity >= PLAYER) {
        uint8_t dir_x = beebram[pentity + ME_DX4_DY4] >> 4;
        uint8_t dir_y = beebram[pentity + ME_DX4_DY4] & 0x0F;

        if (dir_y == PLRDIR_U && pvizdef != ADPTR_DOGWALKU)
            pvizdef = ADPTR_DOGWALKU;
        if (dir_y == PLRDIR_D && pvizdef != ADPTR_DOGWALKD)
            pvizdef = ADPTR_DOGWALKD;
        if (dir_x == PLRDIR_L && pvizdef != ADPTR_DOGWALKL)
            pvizdef = ADPTR_DOGWALKL;
        if (dir_x == PLRDIR_R && pvizdef != ADPTR_DOGWALKR)
            pvizdef = ADPTR_DOGWALKR;

        beebram[pentity + CE_PVIZDEF_LO] = pvizdef & 0xFF;
        beebram[pentity + CE_PVIZDEF_HI] = pvizdef >> 8;
        beebram[pentity + ME_DX4_DY4] = 0;
    }

    uint8_t elapsed = beebram[pentity + CE_FELAPSED5_FCURRENT3] >> 3;
    uint8_t current = beebram[pentity + CE_FELAPSED5_FCURRENT3] & 0b111;
    uint8_t frames = beebram[pvizdef + AD_FRAMES4_YOYO4] >> 4;
    uint8_t yoyo = beebram[pvizdef + AD_FRAMES4_YOYO4] & 0x0F;
    uint8_t period;

    // update the elapsed frame count
    elapsed++;
    beebram[pentity + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
    beebram[pentity + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

    // fetch the period for the current frame index
    switch (current) {
    case 0:
        period = (beebram[pvizdef + AD_PERIODA4_PERIODB4] & 0b11110000) >> 4;
        break;

    case 1:
        period = (beebram[pvizdef + AD_PERIODA4_PERIODB4] & 0b00001111);
        break;

    case 2:
        period = (beebram[pvizdef + AD_PERIODC4_PERIODD4] & 0b11110000) >> 4;
        break;

    case 3:
        period = (beebram[pvizdef + AD_PERIODC4_PERIODD4] & 0b00001111);
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
        beebram[pvizdef + AD_FRAMES4_YOYO4] &= 0xF0;
        beebram[pvizdef + AD_FRAMES4_YOYO4] |= yoyo;

        // write elapsed frames
        elapsed = 0;
        beebram[pentity + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
        beebram[pentity + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

        // make sure pentity has the correct animdef before it gets rendered
        // beebram[pentity + CE_PVIZDEF_LO] = pvizdef & 0x0F;
        // beebram[pentity + CE_PVIZDEF_HI] = pvizdef >> 8;

        // raise redraw flag so that renderStatics() draws it
        beebram[pentity + CE_ROOMID6_REDRAW2] |= 1;
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