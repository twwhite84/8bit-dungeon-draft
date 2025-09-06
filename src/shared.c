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