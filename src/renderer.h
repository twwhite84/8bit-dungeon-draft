#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

#define INTERNAL_WIDTH 320
#define INTERNAL_HEIGHT 256
#define EXTERNAL_WIDTH 960
#define EXTERNAL_HEIGHT 768

extern SDL_Window *window;

void renderBeebram();
void init_renderer();
void eraseScreen();
void eraseTile(uint8_t i, uint8_t j);

#endif