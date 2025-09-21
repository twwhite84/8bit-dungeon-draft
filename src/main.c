#include "animate.h"
#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

typedef struct {
    bool level_1;
    bool level_2;
    bool player_moveLeft;
    bool player_moveRight;
    bool player_moveUp;
    bool player_moveDown;
} GameFlags;

void init();
bool input();
void update();
void loadRoom(int roomID);
void movePlayer(uint8_t dir);
void updateSpriteContainer(uint16_t ptr_actor);

GameFlags gameFlags;

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

    loadRoom(1);
    drawTilebuffer();
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
        gameFlags.player_moveUp = true;
    if (keystates[SDL_SCANCODE_DOWN])
        gameFlags.player_moveDown = true;
    if (keystates[SDL_SCANCODE_LEFT])
        gameFlags.player_moveLeft = true;
    if (keystates[SDL_SCANCODE_RIGHT])
        gameFlags.player_moveRight = true;
    if (keystates[SDL_SCANCODE_1])
        gameFlags.level_1 = true;
    if (keystates[SDL_SCANCODE_2])
        gameFlags.level_2 = true;

    return true;
}

/*----------------------------------------------------------------------------*/

void update() {
    if (gameFlags.level_1) {
        fprintf(stderr, "\nLoad room: 1");
        loadRoom(1);
        drawTilebuffer();
        renderStaticEntities();
        gameFlags.level_1 = false;
    }

    if (gameFlags.level_2) {
        fprintf(stderr, "\nLoad room: 2");
        loadRoom(2);
        drawTilebuffer();
        renderStaticEntities();
        gameFlags.level_2 = false;
    }

    if (gameFlags.player_moveLeft) {
        movePlayer(PLRDIR_W);
        gameFlags.player_moveLeft = false;
    }

    if (gameFlags.player_moveRight) {
        movePlayer(PLRDIR_E);
        gameFlags.player_moveRight = false;
    }

    if (gameFlags.player_moveUp) {
        movePlayer(PLRDIR_N);
        gameFlags.player_moveUp = false;
    }

    if (gameFlags.player_moveDown) {
        movePlayer(PLRDIR_S);
        gameFlags.player_moveDown = false;
    }

    animateStaticEntities();
    renderStaticEntities();

    updateSpriteContainer(PLAYER);

    renderPlayer();
}

/*----------------------------------------------------------------------------*/

void loadRoom(int roomID) {
    memset(&beebram[TILEBUFFER], 0, (size_t)(OFFBUFFER - TILEBUFFER));
    inflate_map(roomID);

    // find the static entities for this room and copy their pointers into camera
    uint8_t se_count = 0;
    uint16_t se_ptr = STATENTS, camera_se_ptr_base = CAMERA + 0x0D;
    for (int i = 0; i < 32; i++) {
        uint16_t se_addr = beebram[se_ptr] + (beebram[se_ptr + 1] << 8);
        se_ptr += 2;

        uint8_t se_roomID = (beebram[se_addr + SE_NQUADS2_ROOMID6] & 0b00111111);
        if (se_roomID == roomID) {
            // add the static entity to the camera
            uint16_t camera_se_ptr = camera_se_ptr_base + (2 * se_count);
            beebram[camera_se_ptr + 0] = se_addr & 0xFF; // lo
            beebram[camera_se_ptr + 1] = se_addr >> 8;   // hi

            // its redraw flag should be raised on insert
            beebram[se_addr + SE_REDRAW1_DATA7] |= 0b10000000;

            se_count++;
            if (se_count > 4)
                break;
        }
    }
    beebram[CAMERA + 0x0C] = se_count;
}

/*----------------------------------------------------------------------------*/

void movePlayer(uint8_t dir) {
    fprintf(stderr, "Player direction: %d\n", dir);

    // set the player direction vector

    // check the next tile a pixel of movement in that direction will land

    // take appropriate action
}

void updateSpriteContainer(uint16_t ptr_actor) {
    // designed for player for the moment

    // get current player position
    uint16_t x = beebram[PLAYER + PLR_X_LO] | (beebram[PLAYER + PLR_X_HI] << 8);
    uint16_t y = beebram[PLAYER + PLR_Y_LO] | (beebram[PLAYER + PLR_Y_HI] << 8);

    // compute and set the shifts for the sprite container
    beebram[PLAYER + PLR_HSHIFT] = x & 0b111;
    beebram[PLAYER + PLR_VSHIFT] = y & 0b111;

    // compute relative screen address for origin of sprite container (top-left corner)
    uint16_t corner_new = (y >> 3) * 0x0140 + (x >> 3) * 8;
    uint16_t corner_old = corner_new;

    // on first sprite container rendering, set cleanup to false
    if (beebram[PLAYER + PLR_CLEANUP] == 0xFF) {
        beebram[PLAYER + PLR_CLEANUP] = false;
    }

    // on subsequent renderings, cleanup is raised if sprite container has moved
    else {
        corner_old = beebram[PLAYER + PLR_PCORNER_LO] | (beebram[PLAYER + PLR_PCORNER_HI] << 8);
        if ((corner_new - corner_old) != 0)
            beebram[PLAYER + PLR_CLEANUP] = true;
    }

    // write the new sprite container corner to the player
    beebram[PLAYER + PLR_PCORNER_LO] = corner_new & 0xFF;
    beebram[PLAYER + PLR_PCORNER_HI] = corner_new >> 8;
}