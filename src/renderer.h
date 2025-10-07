#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

void bufferTile(uint16_t penstart, uint16_t texture, uint16_t mask);
void bufferBG(uint8_t abs_i, uint8_t abs_j, uint8_t dim);

void renderCambuffer();
void renderOffbuffer(uint8_t abs_i, uint8_t abs_j, uint8_t dim);
void renderMovable(uint16_t pmovable);
void renderMovables();
void renderStatics();
void renderCleanup(uint16_t pentity);

#endif