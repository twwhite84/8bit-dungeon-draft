#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

typedef struct {
  int screen_mode;
} UserState;

void init();
bool input();
void update();
void render();

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
  inflate_map(1);
  eraseScreen();
}

/*----------------------------------------------------------------------------*/

bool input() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      return false;
  }
  return true;
}

/*----------------------------------------------------------------------------*/

void update() {}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }