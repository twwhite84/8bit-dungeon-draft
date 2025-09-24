#include "animate.h"
#include "shared.h"
#include <stdint.h>

// walk the camera buffer and update any static entities which have animated tiles
void animateStaticEntities() {
    uint8_t se_n = beebram[CAMERA + 0x0C];
    uint16_t se_ptr = CAMERA + 0x0D;

    for (int i = 0; i < se_n; i++) {
        // fetch the next static entity
        uint16_t se_addr = beebram[se_ptr] + (beebram[se_ptr + 1] << 8);
        se_ptr += 2;

        // get its first vizdef (you only need one to determine if the entity is animated)
        uint16_t se_pvizdef = beebram[se_addr + SE_PVIZDEF_LO] + (beebram[se_addr + SE_PVIZDEF_HI] << 8);

        // skip if not an animdef
        if (se_pvizdef < ANIMDEFS)
            continue;

        uint8_t se_elapsed_frames = (beebram[se_addr + SE_ELAPSED5_TYPE3] & 0b11111000) >> 3;
        uint8_t frames = ((beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b11100000) >> 5) + 1;
        uint8_t current = (beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b00011100) >> 2;
        uint8_t yoyo = beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b000000011;
        uint8_t period;

        // update the static entity's elapsed frame count
        se_elapsed_frames++;
        beebram[se_addr + SE_ELAPSED5_TYPE3] &= 0b00000111;
        beebram[se_addr + SE_ELAPSED5_TYPE3] |= (se_elapsed_frames << 3);

        // fetch the period for the current frame index
        switch (current) {
        case 0:
            period = (beebram[se_pvizdef + AD_PERIOD0_PERIOD1] & 0b11110000) >> 4;
            break;

        case 1:
            period = (beebram[se_pvizdef + AD_PERIOD0_PERIOD1] & 0b00001111);
            break;

        case 2:
            period = (beebram[se_pvizdef + AD_PERIOD2_PERIOD3] & 0b11110000) >> 4;
            break;

        case 3:
            period = (beebram[se_pvizdef + AD_PERIOD2_PERIOD3] & 0b00001111);
            break;
        }

        // if the elapsed frame count > period, cycle the frame and reset the elapsed
        if (se_elapsed_frames > period) {

            // 00: yoyo off
            // 01: yoyo on, direction forward
            // 11: yoyo on, direction negative
            (yoyo == 0b00 || yoyo == 0b01) ? current++ : current--;

            // if past upper end
            if (current >= frames) {
                if (!yoyo)
                    current = 0;
                if (yoyo == 0b01) {
                    current -= 2;
                    yoyo = 0b11;
                }
            }

            // if past lower end ($80 to &FF are - in uint8)
            if (current >= 0x80) {
                current += 2;
                yoyo = 0b01;
            }

            // write current
            beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] &= 0b11100011;
            beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] |= (current << 2);

            // write yoyo
            beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] &= 0b11111100;
            beebram[se_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] |= yoyo;

            // write elapsed frames
            se_elapsed_frames = 0;
            beebram[se_addr + SE_ELAPSED5_TYPE3] &= 0b00000111;
            beebram[se_addr + SE_ELAPSED5_TYPE3] |= (se_elapsed_frames << 3);

            // raise redraw flag so that renderStaticEntities() draws it
            beebram[se_addr + SE_REDRAW1_DATA7] |= 0b10000000;
        }
    }
}

/*----------------------------------------------------------------------------*/

void animatePlayer() {

    // get the player's vizdef. this points to a specific animdef, not the animdefs index
    uint16_t plr_pvizdef = beebram[PLAYER + PLR_PVIZDEF_LO] | (beebram[PLAYER + PLR_PVIZDEF_HI] << 8);

    // skip if not an animdef (sometimes the player vizdef will just be a quad, eg for idle)
    if (plr_pvizdef < ANIMDEFS)
        return;

    uint8_t elapsed_frames = (beebram[PLAYER + PLR_ELAPSED6_CLEANUP2] & 0b11111100) >> 2;
    uint8_t frames = ((beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b11100000) >> 5) + 1;
    uint8_t current = (beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b00011100) >> 2;
    uint8_t yoyo = beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b000000011;
    uint8_t period;

    // update the player's elapsed frame count
    elapsed_frames++;
    beebram[PLAYER + PLR_ELAPSED6_CLEANUP2] &= 0b00000011;
    beebram[PLAYER + PLR_ELAPSED6_CLEANUP2] |= (elapsed_frames << 2);

    // fetch the period for the current frame index
    switch (current) {
    case 0:
        period = (beebram[plr_pvizdef + AD_PERIOD0_PERIOD1] & 0b11110000) >> 4;
        break;

    case 1:
        period = (beebram[plr_pvizdef + AD_PERIOD0_PERIOD1] & 0b00001111);
        break;

    case 2:
        period = (beebram[plr_pvizdef + AD_PERIOD2_PERIOD3] & 0b11110000) >> 4;
        break;

    case 3:
        period = (beebram[plr_pvizdef + AD_PERIOD2_PERIOD3] & 0b00001111);
        break;
    }

    // if the elapsed frame count > period, cycle the frame and reset the elapsed
    if (elapsed_frames > period) {

        // 00: yoyo off
        // 01: yoyo on, direction forward
        // 11: yoyo on, direction negative
        (yoyo == 0b00 || yoyo == 0b01) ? current++ : current--;

        // if past upper end
        if (current >= frames) {
            if (!yoyo)
                current = 0;
            if (yoyo == 0b01) {
                current -= 2;
                yoyo = 0b11;
            }
        }

        // if past lower end ($80 to &FF are - in uint8)
        if (current >= 0x80) {
            current += 2;
            yoyo = 0b01;
        }

        // write current
        beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] &= 0b11100011;
        beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] |= (current << 2);

        // write yoyo
        beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] &= 0b11111100;
        beebram[plr_pvizdef + AD_FRAMES3_CURRENT3_YOYO2] |= yoyo;

        // write elapsed frames
        elapsed_frames = 0;
        beebram[PLAYER + PLR_ELAPSED6_CLEANUP2] &= 0b00000011;
        beebram[PLAYER + PLR_ELAPSED6_CLEANUP2] |= (elapsed_frames << 2);

        // raise redraw flag so that renderStaticEntities() draws it
        beebram[PLAYER + PLR_ROOM6_REDRAW2] &= 0b11111100;
        beebram[PLAYER + PLR_ROOM6_REDRAW2] |= 0b00000001;
    }
}
