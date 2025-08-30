#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

#define INTERNAL_WIDTH 320
#define INTERNAL_HEIGHT 256
#define EXTERNAL_WIDTH 960
#define EXTERNAL_HEIGHT 768

extern SDL_Window *window;

void render();
void init_renderer();

#endif