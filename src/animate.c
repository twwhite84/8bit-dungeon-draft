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

        // if animdef, it will be a pointer from animdef index to an animdef def
        uint16_t animdef = (beebram[se_pvizdef] & 0xFF) | (beebram[se_pvizdef + 1] << 8);
        uint8_t se_elapsed_frames = (beebram[se_addr + SE_ELAPSED5_TYPE3] & 0b11111000) >> 3;
        uint8_t frames = ((beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b11100000) >> 5) + 1;
        uint8_t current = (beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b00011100) >> 2;
        uint8_t yoyo = beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b00000011;
        uint8_t period;

        // update the static entity's elapsed frame count
        se_elapsed_frames++;
        beebram[se_addr + SE_ELAPSED5_TYPE3] &= 0b00000111;
        beebram[se_addr + SE_ELAPSED5_TYPE3] |= (se_elapsed_frames << 3);

        // fetch the period for the current frame index
        switch (current) {
        case 0:
            period = (beebram[animdef + AD_PERIOD0_PERIOD1] & 0b11110000) >> 4;
            break;

        case 1:
            period = (beebram[animdef + AD_PERIOD0_PERIOD1] & 0b00001111);
            break;

        case 2:
            period = (beebram[animdef + AD_PERIOD2_PERIOD3] & 0b11110000) >> 4;
            break;

        case 3:
            period = (beebram[animdef + AD_PERIOD2_PERIOD3] & 0b00001111);
            break;
        }

        // if the elapsed frame count > period, cycle the frame and reset the elapsed
        if (se_elapsed_frames > period) {
            current++;
            if (current == frames)
                current = 0;

            // write current
            beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] &= 0b11100011;
            beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] |= (current << 2);

            // write elapsed frames
            se_elapsed_frames = 0;
            beebram[se_addr + SE_ELAPSED5_TYPE3] &= 0b00000111;
            beebram[se_addr + SE_ELAPSED5_TYPE3] |= (se_elapsed_frames << 3);

            // raise redraw flag so that renderStaticEntities() draws it
            beebram[se_addr + SE_REDRAW1_DATA7] |= 0b10000000;
        }
    }
}