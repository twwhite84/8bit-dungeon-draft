#ifndef SHARED_H
#define SHARED_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define WIDTH 320
#define HEIGHT 256

extern uint8_t beebram[0x8000];
extern SDL_Window *window;

#endif