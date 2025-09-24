#include "shared.h"

uint8_t beebram[0x8000];

// floor divides a by b
FloorResults floordiv(int a, int b) {
    int q = 0;
    while (a >= b) {
        a -= b;
        q++;
    }
    FloorResults results;
    results.q = q;
    results.r = a;
    return results;
}

// these can probably just go in zero page somewhere as globals
int bhops[4] = {0, 8, 24, 32};

uint16_t getTileTextureAddr(uint8_t tid) {
    uint16_t ptr_location = QUADDEFS + 2 * tid;
    uint16_t ptr = beebram[ptr_location] | (beebram[ptr_location + 1] << 8);
    return ptr;
}