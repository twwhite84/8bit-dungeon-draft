#include "camera.h"
#include "inflate.h"
#include "shared.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void loadRoom(uint8_t roomID) {

    // clear the camera
    memset(&beebram[CAMERA], SENTINEL8, (size_t)(CAMBUFFER - CAMERA));

    beebram[CAMERA + CAM_ROOMID] = roomID;

    // inflate the compressed tilemap into the cambuffer
    inflateMap(roomID);
    beebram[CAMERA + CAM_REDRAW] |= REDRAW_BACKGROUND;

    // load the statics for the room into the camera
    loadStatics(roomID, true);
    beebram[CAMERA + CAM_REDRAW] |= REDRAW_STATICS;
}

/*----------------------------------------------------------------------------*/

void loadStatics(uint8_t roomID, bool redraw_all) {
    memset(&beebram[CAMERA + CAM_PSE0_LO], SENTINEL8, (size_t)(CAMCON_SEMAX << 1));

    // find subset of statics for this room and copy their pointers into the camera
    uint16_t ptable = SE_TABLE;
    uint16_t pcam = (CAMERA + CAM_PSE0_LO);

    // walk all statics and copy those with matching roomID
    for (uint8_t i = 0; i < SECON_SEMAX; i++) {
        uint16_t pse = beebram[ptable] | (beebram[ptable + 1] << 8);

        if (pse == SENTINEL16)
            break;

        ptable += 2;

        uint8_t se_roomID = (beebram[pse + CE_ROOMID6_CLEAN1_REDRAW1]) >> 2;
        if (se_roomID == roomID) {

            // add the static entity to the camera
            beebram[pcam] = pse & 0xFF;   // lo
            beebram[pcam + 1] = pse >> 8; // hi
            pcam += 2;

            // make sure this entity is marked for redraw
            if (redraw_all)
                beebram[pse + CE_ROOMID6_CLEAN1_REDRAW1] |= 1;
        }
    }
}