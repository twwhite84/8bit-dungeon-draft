#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

void init();
void input();
void update();
void render();

const int FRAME_CAP = 33; // ~33.3ms for 30fps

/*----------------------------------------------------------------------------*/

// main function needs this signature to make SDL happy
int main(int argc, char *args[]) {

  init();

  SDL_Event e;
  bool done = false;
  while (!done) {
    uint32_t et = SDL_GetTicks();

    // INPUTS
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        done = true;
    }

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
}

/*----------------------------------------------------------------------------*/

void input() {}

/*----------------------------------------------------------------------------*/

void update() {}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }