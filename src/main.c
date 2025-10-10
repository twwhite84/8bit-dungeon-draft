#include "animate.h"
#include "inflate.h"
#include "init.h"
#include "mySDL.h"
#include "player.h"
#include "renderer.h"
#include "shared.h"

#include <stdbool.h>

typedef struct {
    bool loadRoom_0;
    bool loadroom_1;
    bool player_moveRequested;
    bool player_moveLeft;
    bool player_moveRight;
    bool player_moveUp;
    bool player_moveDown;
    bool player_moveRun;
} InputFlags;

void init();
bool input();
void update();
void render();
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

        // LIMIT TO 30FPS
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
    mySDLInitRenderer();
    init_ram();

    inputFlags.player_moveRequested = false;

    uint8_t roomID = beebram[PLAYER + CE_ROOMID6_CLEAN1_REDRAW1] >> 2;

    loadRoom(roomID);
    renderCambuffer();
    renderStatics();
}

/*----------------------------------------------------------------------------*/

bool input() {

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;
    }

    const uint8_t *keystates = SDL_GetKeyboardState(NULL);

    // PLAYER MOVEMENT
    if (keystates[SDL_SCANCODE_LSHIFT] || keystates[SDL_SCANCODE_RSHIFT]) {
        inputFlags.player_moveRun = true;
    }

    if (keystates[SDL_SCANCODE_UP]) {
        inputFlags.player_moveRequested = true;
        inputFlags.player_moveUp = true;
    }

    if (keystates[SDL_SCANCODE_DOWN]) {
        inputFlags.player_moveRequested = true;
        inputFlags.player_moveDown = true;
    }

    if (keystates[SDL_SCANCODE_LEFT]) {
        inputFlags.player_moveRequested = true;
        inputFlags.player_moveLeft = true;
    }

    if (keystates[SDL_SCANCODE_RIGHT]) {
        inputFlags.player_moveRequested = true;
        inputFlags.player_moveRight = true;
    }

    if (keystates[SDL_SCANCODE_UP] && keystates[SDL_SCANCODE_DOWN]) {
        inputFlags.player_moveUp = false;
        inputFlags.player_moveDown = false;
    }

    if (keystates[SDL_SCANCODE_LEFT] && keystates[SDL_SCANCODE_RIGHT]) {
        inputFlags.player_moveLeft = false;
        inputFlags.player_moveRight = false;
    }

    // MAP TEST
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
        renderCambuffer();
        renderStatics();
        inputFlags.loadRoom_0 = false;
    }

    if (inputFlags.loadroom_1) {
        loadRoom(1);
        renderCambuffer();
        renderStatics();
        inputFlags.loadroom_1 = false;
    }

    if (inputFlags.player_moveRequested) {
        beebram[PLAYER + ME_DIRX4_DIRY4] = 0;
        uint8_t speed = (inputFlags.player_moveRun) ? 2 : 1;

        if (inputFlags.player_moveUp) {
            beebram[PLAYER + ME_DIRX4_DIRY4] |= ((speed << 2) | DIR_NEGATIVE);
            inputFlags.player_moveUp = false;
        }

        if (inputFlags.player_moveDown) {
            beebram[PLAYER + ME_DIRX4_DIRY4] |= ((speed << 2) | DIR_POSITIVE);
            inputFlags.player_moveDown = false;
        }

        if (inputFlags.player_moveLeft) {
            beebram[PLAYER + ME_DIRX4_DIRY4] |= ((speed << 6) | DIR_NEGATIVE << 4);
            inputFlags.player_moveLeft = false;
        }

        if (inputFlags.player_moveRight) {
            beebram[PLAYER + ME_DIRX4_DIRY4] |= ((speed << 6) | DIR_POSITIVE << 4);
            inputFlags.player_moveRight = false;
        }

        inputFlags.player_moveRequested = false;
        inputFlags.player_moveRun = false;
        movePlayer();
        animateEntity(PLAYER);
    }

    animateStatics();
}

/*----------------------------------------------------------------------------*/

void render() {
    renderMovable(PLAYER); // because player isn't held in camera
    renderStatics();
    // renderMovables();      // non-player movables
    mySDLRender();
}

/*----------------------------------------------------------------------------*/

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