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

uint16_t getTileTextureAddr(uint8_t tid) {
    uint16_t ptr_location = QUADS_PLAIN + 2 * tid;
    uint16_t ptr = beebram[ptr_location] | (beebram[ptr_location + 1] << 8);
    return ptr;
}

uint16_t ij2ramloc(uint8_t i, uint8_t j) {
    uint16_t result = (i << 8) + (i << 6) + (j << 3);
    return result;
}

uint16_t ramloc2ij(uint16_t ramloc) {
    uint8_t i = ramloc / 0x140;
    uint8_t j = (ramloc % 0x140) >> 3;
    uint16_t result = (j | (i << 8));
    return result;
}

uint16_t xy2ramloc(uint16_t x, uint16_t y) {
    uint16_t result = (y >> 3) * 0x0140 + (x >> 3) * 8;
    return result;
}

// returns i (8) | j (8)
uint16_t xy2ij(uint16_t x, uint16_t y) {
    uint16_t result = (x >> 3) | ((y >> 3) << 8);
    return result;
}