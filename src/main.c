#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

typedef struct {
    bool jump_to_L0;
    bool jump_to_L1;
} GameFlags;

void init();
bool input();
void update();
void render();

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

    inflate_map(0);
    eraseScreen();

    // walk the objects and find which of them match current screen

    // those which do match should be added to the camera

    // the camera should have a redraw function which:
    // -- draws the background
    // -- draws objects
    // -- draws sprites
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
            case SDLK_0:
                gf.jump_to_L0 = true;
                break;

            case SDLK_1:
                gf.jump_to_L1 = true;
                break;
            }
        }
    }
    return true;
}

/*----------------------------------------------------------------------------*/

void update() {
    if (gf.jump_to_L0) {
        fprintf(stderr, "\nJump to L0");
        inflate_map(0);
        eraseScreen();
        gf.jump_to_L0 = false;
    }
    if (gf.jump_to_L1) {
        fprintf(stderr, "\nJump to L1");
        inflate_map(1);
        eraseScreen();
        gf.jump_to_L1 = false;
    }
}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }