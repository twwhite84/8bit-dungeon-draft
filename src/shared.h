#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

#define WIDTH 320
#define HEIGHT 256
#define STORED_ROWS 13
#define STORED_COLUMNS 20
#define CAMERA_ROWS (2 * STORED_ROWS)
#define CAMERA_COLUMNS (2 * STORED_COLUMNS)

#define TILEBUFFER 0x3733
#define TILEMAPS 0x43C0

typedef struct {
  uint8_t q;
  uint8_t r;
} FloorResults;

extern uint8_t beebram[0x8000];

FloorResults floordiv(int a, int b);

#endif