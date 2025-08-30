#include "depacker.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

void update();

const int FRAME_DELAY = 60 * (1 / 1000);

/*----------------------------------------------------------------------------*/

// main function needs this signature to make SDL happy
int main(int argc, char *args[]) {
  init_ram();
  init_renderer();

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
    if (et < FRAME_DELAY) {
      SDL_Delay(FRAME_DELAY - et);
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

/*----------------------------------------------------------------------------*/

void update() {
  // make row 1, column 1 tile a plain white block
  uint16_t start = 0x5948;
  beebram[start + 0] = 0xF5;
  beebram[start + 1] = 0xF5;
  beebram[start + 2] = 0xF5;
  beebram[start + 3] = 0xF5;
  beebram[start + 4] = 0xAF;
  beebram[start + 5] = 0xAF;
  beebram[start + 6] = 0xAF;
  beebram[start + 7] = 0xAF;

  // expand map 1 into the room buffer
  uint16_t map_addr = mapIDtoAddr(0);
  inflate(map_addr, TILEBUFFER);
  return;
}