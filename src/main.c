#include "animate.h"
#include "camera.h"
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
        beebram[PLAYER + ME_XMD4_YMD4] = 0;
        uint8_t speed = (inputFlags.player_moveRun) ? 2 : 1;

        if (inputFlags.player_moveUp) {
            beebram[PLAYER + ME_XMD4_YMD4] |= ((speed << 2) | DIR_UP);
            inputFlags.player_moveUp = false;
        }

        if (inputFlags.player_moveDown) {
            beebram[PLAYER + ME_XMD4_YMD4] |= ((speed << 2) | DIR_DOWN);
            inputFlags.player_moveDown = false;
        }

        if (inputFlags.player_moveLeft) {
            beebram[PLAYER + ME_XMD4_YMD4] |= ((speed << 6) | DIR_LEFT << 4);
            inputFlags.player_moveLeft = false;
        }

        if (inputFlags.player_moveRight) {
            beebram[PLAYER + ME_XMD4_YMD4] |= ((speed << 6) | DIR_RIGHT << 4);
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
