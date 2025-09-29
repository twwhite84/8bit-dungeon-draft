/* All my SDL specific stuff goes in this module. It covers SDL setup and
functions for dumping the simulated framebuffer to a window canvas. */

#ifndef MYSDL_H
#define MYSDL_H

#include <SDL2/SDL.h>

extern SDL_Window *window;

void mySDLInitRenderer();
void mySDLRender();

#endif