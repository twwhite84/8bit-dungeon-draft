#include "animate.h"
#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

typedef struct {
    bool jump_to_L1;
    bool jump_to_L2;
} GameFlags;

void init();
bool input();
void update();
void loadRoom(int roomID);

GameFlags gf;

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

        // KEYUP means KEYRELEASED, it's not !KEYDOWN
        if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_1:
                gf.jump_to_L1 = true;
                break;

            case SDLK_2:
                gf.jump_to_L2 = true;
                break;
            }
        }
    }
    return true;
}

/*----------------------------------------------------------------------------*/

void update() {
    if (gf.jump_to_L1) {
        fprintf(stderr, "\nJump to L0");
        loadRoom(1);
        drawTilebuffer();
        renderStaticEntities();
        gf.jump_to_L1 = false;
    }
    if (gf.jump_to_L2) {
        fprintf(stderr, "\nJump to L1");
        loadRoom(2);
        drawTilebuffer();
        renderStaticEntities();
        gf.jump_to_L2 = false;
    }

    animateStaticEntities();
    renderStaticEntities();
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
