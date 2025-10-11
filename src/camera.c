#include "camera.h"
#include "inflate.h"
#include "shared.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void loadRoom(uint8_t roomID) {
    memset(&beebram[CAMERA], 0, (size_t)(OFFBUFFER - CAMERA));
    memset(&beebram[CAMERA + CAM_PME0_LO], 0xFF, (size_t)(CAMBUFFER - CAMERA - CAM_PME0_LO));
    beebram[CAMERA + CAM_ROOMID] = roomID;

    // inflate the stored map into the cambuffer
    inflateMap(roomID);

    // find subset of static entities for this room and copy their pointers into the camera
    uint8_t entities_copied = 0;
    uint16_t se_ptr_table = SE_TABLE;
    uint16_t se_ptr_camera = (CAMERA + CAM_PSE0_LO);

    // walk all static entities and copy those with matching roomID
    while (true) {
        uint16_t se_addr = beebram[se_ptr_table] | (beebram[se_ptr_table + 1] << 8);

        // 0xFFFF is sentinel, means no more pointers in the table
        if (se_addr == 0xFFFF)
            break;

        se_ptr_table += 2;

        uint8_t se_roomID = (beebram[se_addr + CE_ROOMID6_CLEAN1_REDRAW1]) >> 2;
        if (se_roomID == roomID) {

            // add the static entity to the camera
            beebram[se_ptr_camera] = se_addr & 0xFF;   // lo
            beebram[se_ptr_camera + 1] = se_addr >> 8; // hi
            se_ptr_camera += 2;

            // make sure this entity is marked for redraw
            beebram[se_addr + CE_ROOMID6_CLEAN1_REDRAW1] |= 1;

            entities_copied++;
            if (entities_copied >= 10)
                break;
        }
    }
}