#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 768

extern SDL_Window *window;

void render();
void init_renderer();

#endif