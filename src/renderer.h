#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

#define INTERNAL_WIDTH 320
#define INTERNAL_HEIGHT 256
#define EXTERNAL_WIDTH 960
#define EXTERNAL_HEIGHT 768

extern SDL_Window *window;

void init_renderer();
void render();

void renderBeebram();
void renderBackground();
void eraseTile(uint8_t i, uint8_t j);
void bufferTile(uint16_t penstart, uint16_t texture, uint16_t mask);
void bufferBG(uint8_t abs_i, uint8_t abs_j, uint8_t dim);
void renderStaticEntities();
void renderPlayer();
void renderWhatever(uint8_t abs_i, uint8_t abs_j, uint8_t dim);

#endif