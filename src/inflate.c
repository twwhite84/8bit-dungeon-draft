#include "inflate.h"
#include <stdbool.h>

// depacker modes
#define TILEMAP 0
#define TILEMAP_RLE 1
#define TEXT 2

static void inflate(uint16_t src_addr, uint16_t dest_addr);
static void upsize(uint16_t inidx, uint8_t inval, uint16_t base_addr);
static uint16_t mapIDtoAddr(uint8_t id);

/*----------------------------------------------------------------------------*/

// [npacked, mode, bp, bp, ...]
// this function wont just be for tilemaps, so use src and dest addresses
static void inflate(uint16_t src, uint16_t dest) {
    uint16_t outidx = 0;
    uint8_t inidx = 2, a = 0, b = 0, outbyte = 0, seq = 0;
    bool newpair = true;

    uint8_t npacked = beebram[src];
    uint8_t mode = beebram[src + 1];

    while (inidx < npacked) {
        switch (seq) {
        case 0:
            outbyte = (beebram[src + inidx] >> 3) & 0x1F;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 1;
            break;

        case 1:
            outbyte = (beebram[src + inidx] << 2 | beebram[src + inidx + 1] >> 6) & 0x1F;
            inidx++;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 2;
            break;

        case 2:
            outbyte = (beebram[src + inidx] >> 1) & 0x1F;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 3;
            break;

        case 3:
            outbyte = (beebram[src + inidx] << 4 | beebram[src + inidx + 1] >> 4) & 0x1F;
            inidx++;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 4;
            break;

        case 4:
            outbyte = (beebram[src + inidx] << 1 | beebram[src + inidx + 1] >> 7) & 0x1F;
            inidx++;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 5;
            break;

        case 5:
            outbyte = (beebram[src + inidx] >> 2) & 0x1F;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 6;
            break;

        case 6:
            outbyte = (beebram[src + inidx] << 3 | beebram[src + inidx + 1] >> 5) & 0x1F;
            inidx++;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 7;
            break;

        case 7:
            outbyte = beebram[src + inidx++] & 0x1F;
            if (newpair) {
                a = outbyte;
                newpair = false;
            } else {
                b = outbyte;
                newpair = true;
            }
            seq = 0;
            break;
        }

        // a is value, b is run
        if (newpair && mode == TILEMAP_RLE) {
            while (b > 0) {
                upsize(outidx++, a, dest);
                b--;
            }
        }

        // a is value, b is value
        if (newpair && mode == TILEMAP) {
            upsize(outidx++, a, dest);
            upsize(outidx++, b, dest);
        }
    }

    // hack fix for when last value doesn't get filled in, seems to work
    while (outidx < (STORED_ROWS * STORED_COLUMNS) && mode == TILEMAP_RLE) {
        upsize(outidx++, a, dest);
    }
}

/*----------------------------------------------------------------------------*/

static void upsize(uint16_t inidx, uint8_t inval, uint16_t base_addr) {
    FloorResults fr = floordiv(inidx, STORED_COLUMNS);
    uint8_t inrow = fr.q, incol = fr.r;

    uint16_t outidx_a = (inrow * (4 * STORED_COLUMNS)) + (2 * incol);
    uint16_t outidx_b = outidx_a + 1;
    uint16_t outidx_c = outidx_a + 2 * STORED_COLUMNS;
    uint16_t outidx_d = outidx_c + 1;

    // data to write: output[output_idx_n]=4*inval+n
    beebram[base_addr + outidx_a] = 4 * inval + 0;
    beebram[base_addr + outidx_b] = 4 * inval + 1;
    beebram[base_addr + outidx_c] = 4 * inval + 2;
    beebram[base_addr + outidx_d] = 4 * inval + 3;

    return;
}

/*----------------------------------------------------------------------------*/

static uint16_t mapIDtoAddr(uint8_t id) {
    uint16_t pmap = TMAP_TABLE + 2 * id;
    uint16_t map = beebram[pmap] | (beebram[pmap + 1] << 8);
    return map;
}

/*----------------------------------------------------------------------------*/

// expand the given room's stored 13x20 map into the 26x40 tilebuffer
void inflateMap(uint8_t mapID) { inflate(mapIDtoAddr(mapID), CAMBUFFER); }