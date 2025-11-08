#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

void getTextureAndMask(uint8_t i, uint8_t j, uint16_t quad, uint16_t *texture, uint16_t *mask);
void bufferTextureAndMask(uint8_t i, uint8_t j, uint16_t texture, uint16_t mask);
void bufferBG(uint8_t abs_i, uint8_t abs_j, uint8_t dim);
void renderBG();
void renderOffbuffer(uint8_t i, uint8_t j, uint8_t dim);
void renderPlayer();
void renderMovables();
void renderStatics();
void renderCleanup(uint16_t pentity);
void renderEraseSlot();
void statiks2container(uint8_t MEi_screen, uint8_t MEj_screen);

#endif