#include "animate.h"
#include "shared.h"

void animateEntity(uint16_t pentity) {

    // if pentity is SE, then its quad(s) are determined to be animated based on its first pvizdef
    // this means you cant mix animated and non-animated quads in a static entity
    // e.g. if you want a glowing spot on a large static, you'll need two statics
    uint16_t pvizdef = beebram[pentity + CE_PVIZDEF_LO] + (beebram[pentity + CE_PVIZDEF_HI] << 8);

    // bail if not an animdef
    if (pvizdef < ANIMDEFS)
        return;

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

        // raise redraw flag so that renderStaticEntities() draws it
        beebram[pentity + CE_ROOMID6_REDRAW2] |= 1;
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
        animateEntity(se_ptr);
    }
}