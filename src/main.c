#include "animate.h"
#include "inflate.h"
#include "init.h"
#include "player.h"
#include "renderer.h"
#include "shared.h"
#include "sprite.h"
#include <stdbool.h>

typedef struct {
    bool loadRoom_0;
    bool loadroom_1;
    bool player_moveLeft;
    bool player_moveRight;
    bool player_moveUp;
    bool player_moveDown;
} InputFlags;

void init();
bool input();
void update();
void loadRoom(uint8_t roomID);

InputFlags inputFlags;

/*----------------------------------------------------------------------------*/

// main function needs this signature to make SDL happy
int main(int argc, char *args[]) {
    const int FRAME_CAP = 33; // ~33.3ms for 30fps
    bool gameActive = true;

    init();

    while (gameActive) {
        uint32_t et = SDL_GetTicks();

        gameActive = input();

        update();

        render();

        // LIMIT TO 60FPS
        et = SDL_GetTicks() - et;
        if (et < FRAME_CAP) {
            SDL_Delay(FRAME_CAP - et);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

/*----------------------------------------------------------------------------*/

void init() {
    init_ram();
    init_renderer();

    uint8_t roomID = beebram[PLAYER + CE_ROOMID6_REDRAW2] >> 2;

    loadRoom(roomID);
    renderBackground();
    renderStaticEntities();
}

/*----------------------------------------------------------------------------*/

bool input() {

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;
    }

    const uint8_t *keystates = SDL_GetKeyboardState(NULL);
    if (keystates[SDL_SCANCODE_UP])
        inputFlags.player_moveUp = true;
    if (keystates[SDL_SCANCODE_DOWN])
        inputFlags.player_moveDown = true;
    if (keystates[SDL_SCANCODE_LEFT])
        inputFlags.player_moveLeft = true;
    if (keystates[SDL_SCANCODE_RIGHT])
        inputFlags.player_moveRight = true;
    if (keystates[SDL_SCANCODE_1])
        inputFlags.loadRoom_0 = true;
    if (keystates[SDL_SCANCODE_2])
        inputFlags.loadroom_1 = true;

    return true;
}

/*----------------------------------------------------------------------------*/

void update() {
    if (inputFlags.loadRoom_0) {
        loadRoom(0);
        renderBackground();
        renderStaticEntities();
        inputFlags.loadRoom_0 = false;
    }

    if (inputFlags.loadroom_1) {
        loadRoom(1);
        renderBackground();
        renderStaticEntities();
        inputFlags.loadroom_1 = false;
    }

    if (inputFlags.player_moveUp) {
        movePlayer(PLRDIR_U);
        animateME(PLAYER);
        inputFlags.player_moveUp = false;
    }

    if (inputFlags.player_moveDown) {
        movePlayer(PLRDIR_D);
        animateME(PLAYER);
        inputFlags.player_moveDown = false;
    }

    if (inputFlags.player_moveLeft) {
        movePlayer(PLRDIR_L);
        animateME(PLAYER);
        inputFlags.player_moveLeft = false;
    }

    if (inputFlags.player_moveRight) {
        movePlayer(PLRDIR_R);
        animateME(PLAYER);
        inputFlags.player_moveRight = false;
    }

    // animate SEs held in camera
    // uint16_t cam_se_ptr = CAMERA + CAM_PSE0_LO;
    // for (uint8_t i = 0; i < (beebram[CAMERA + CAM_NME4_NSE4] & 0x0F); i++) {
    //     animateSE(beebram[cam_se_ptr] | (beebram[cam_se_ptr + 1] << 8));
    //     cam_se_ptr += 2;
    // }
    animateCameraSE();

    renderStaticEntities();

    // if player's redraw flag is raised, redraw
    uint8_t redraw = beebram[PLAYER + CE_ROOMID6_REDRAW2] & 0b11;
    if (redraw) {
        updateSpriteContainer(PLAYER);
        bufferSpriteBackground(PLAYER);
        bufferSpriteForeground(PLAYER);
        renderPlayer();
    }
}

/*----------------------------------------------------------------------------*/

void loadRoom(uint8_t roomID) {
    memset(&beebram[CAMERA], 0, (size_t)(OFFBUFFER - CAMERA));
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

        uint8_t se_roomID = (beebram[se_addr + CE_ROOMID6_REDRAW2]) >> 2;
        if (se_roomID == roomID) {

            // add the static entity to the camera
            beebram[se_ptr_camera] = se_addr & 0xFF;   // lo
            beebram[se_ptr_camera + 1] = se_addr >> 8; // hi
            se_ptr_camera += 2;

            // make sure this entity is marked for redraw
            beebram[se_addr + CE_ROOMID6_REDRAW2] |= 1;

            entities_copied++;
            if (entities_copied >= 10)
                break;
        }
    }

    // save number of entities copied
    beebram[CAMERA + CAM_NME4_NSE4] &= 0xF0;
    beebram[CAMERA + CAM_NME4_NSE4] |= (entities_copied & 0x0F);
}

/*----------------------------------------------------------------------------*/
