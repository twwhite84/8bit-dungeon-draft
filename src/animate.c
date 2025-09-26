#include "animate.h"
#include "shared.h"
#include <stdio.h>

void animateEntity(uint16_t pse) {

    // havent implemented allowing an SE to have a mix of static and animated quads
    // this just assumes they're all one or the other, i may revisit this
    uint16_t pvizdef = beebram[pse + SE_PVIZDEF_LO] + (beebram[pse + SE_PVIZDEF_HI] << 8);

    // bail if not an animdef
    if (pvizdef < ANIMDEFS)
        return;

    uint8_t elapsed = beebram[pse + CE_FELAPSED5_FCURRENT3] >> 3;
    uint8_t current = beebram[pse + CE_FELAPSED5_FCURRENT3] & 0b111;
    uint8_t frames = beebram[pvizdef + AD_FRAMES4_YOYO4] >> 4;
    uint8_t yoyo = beebram[pvizdef + AD_FRAMES4_YOYO4] & 0x0F;
    uint8_t period;

    // update the elapsed frame count
    elapsed++;
    beebram[pse + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
    beebram[pse + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

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
        beebram[pse + CE_FELAPSED5_FCURRENT3] &= 0b11111000;
        beebram[pse + CE_FELAPSED5_FCURRENT3] |= current;

        // save yoyo
        beebram[pvizdef + AD_FRAMES4_YOYO4] &= 0xF0;
        beebram[pvizdef + AD_FRAMES4_YOYO4] |= yoyo;

        // write elapsed frames
        elapsed = 0;
        beebram[pse + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
        beebram[pse + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

        // raise redraw flag so that renderStaticEntities() draws it
        beebram[pse + CE_ROOMID6_REDRAW2] |= 1;
    }
}

/*----------------------------------------------------------------------------*/

void animateSE(uint16_t pse) {

    // havent implemented allowing an SE to have a mix of static and animated quads
    // this just assumes they're all one or the other, i may revisit this
    uint16_t pvizdef = beebram[pse + SE_PVIZDEF_LO] + (beebram[pse + SE_PVIZDEF_HI] << 8);

    // bail if not an animdef
    if (pvizdef < ANIMDEFS)
        return;

    uint8_t elapsed = beebram[pse + CE_FELAPSED5_FCURRENT3] >> 3;
    uint8_t current = beebram[pse + CE_FELAPSED5_FCURRENT3] & 0b111;
    uint8_t frames = beebram[pvizdef + AD_FRAMES4_YOYO4] >> 4;
    uint8_t yoyo = beebram[pvizdef + AD_FRAMES4_YOYO4] & 0x0F;
    uint8_t period;

    // update the elapsed frame count
    elapsed++;
    beebram[pse + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
    beebram[pse + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

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
        beebram[pse + CE_FELAPSED5_FCURRENT3] &= 0b11111000;
        beebram[pse + CE_FELAPSED5_FCURRENT3] |= current;

        // save yoyo
        beebram[pvizdef + AD_FRAMES4_YOYO4] &= 0xF0;
        beebram[pvizdef + AD_FRAMES4_YOYO4] |= yoyo;

        // write elapsed frames
        elapsed = 0;
        beebram[pse + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
        beebram[pse + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

        // raise redraw flag so that renderStaticEntities() draws it
        beebram[pse + CE_ROOMID6_REDRAW2] |= 1;
    }
}

/*----------------------------------------------------------------------------*/

// walk the camera buffer and update any static entities which have animated tiles
void animateCameraSE() {
    uint8_t se_n = beebram[CAMERA + CAM_NME4_NSE4] & 0x0F;
    uint16_t cam_se_start = CAMERA + CAM_PSE0_LO;

    for (uint8_t i = 0; i < se_n; i++) {
        uint16_t se_ptr = beebram[cam_se_start] | (beebram[cam_se_start + 1] << 8);
        cam_se_start += 2;
        animateSE(se_ptr);
    }
}

/*----------------------------------------------------------------------------*/

void animateME(uint16_t pme) {

    // get the player's vizdef. this points to a specific animdef, not the animdefs index
    uint16_t pvizdef = beebram[pme + ME_PVIZDEF_LO] | (beebram[pme + ME_PVIZDEF_HI] << 8);

    // skip if not an animdef (sometimes the player vizdef will just be a quad, eg for idle)
    if (pvizdef < ANIMDEFS)
        return;

    uint8_t elapsed = beebram[pme + CE_FELAPSED5_FCURRENT3] >> 3;
    uint8_t current = beebram[pme + CE_FELAPSED5_FCURRENT3] & 0b111;
    uint8_t frames = (beebram[pvizdef + AD_FRAMES4_YOYO4] & 0xF0) >> 4;
    uint8_t yoyo = beebram[pvizdef + AD_FRAMES4_YOYO4] & 0x0F;
    uint8_t period;

    // update the player's elapsed frame count
    elapsed++;
    beebram[pme + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
    beebram[pme + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

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

        // 0: yoyo off
        // 1: direction forward
        // 2: direction backward
        (yoyo == 0 || yoyo == 1) ? current++ : current--;

        // if past upper end
        if (current >= frames) {
            if (!yoyo)
                current = 0;
            if (yoyo == 1) {
                current -= 2;
                yoyo = 2;
            }
        }

        // if past lower end ($80 to &FF are - in uint8)
        if (current >= 0x80) {
            current += 2;
            yoyo = 1;
        }

        // save current
        beebram[pme + CE_FELAPSED5_FCURRENT3] &= 0b11111000;
        beebram[pme + CE_FELAPSED5_FCURRENT3] |= current;

        // save yoyo
        beebram[pvizdef + AD_FRAMES4_YOYO4] &= 0xF0;
        beebram[pvizdef + AD_FRAMES4_YOYO4] |= yoyo;

        // write elapsed frames
        elapsed = 0;
        beebram[pme + CE_FELAPSED5_FCURRENT3] &= 0b00000111;
        beebram[pme + CE_FELAPSED5_FCURRENT3] |= (elapsed << 3);

        // raise redraw flag so that renderStaticEntities() draws it
        beebram[pme + CE_ROOMID6_REDRAW2] |= 1;
    }
}
