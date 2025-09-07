#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

typedef struct {
    bool jump_to_L1;
    bool jump_to_L2;
} GameFlags;

void init();
bool input();
void update();
void render();

void loadRoom(int roomID);
void drawStatents();

GameFlags gf;

/*----------------------------------------------------------------------------*/

// main function needs this signature to make SDL happy
int main(int argc, char *args[]) {
    const int FRAME_CAP = 33; // ~33.3ms for 30fps
    bool gameActive = true;

    init();

    while (gameActive) {
        uint32_t et = SDL_GetTicks();

        gameActive = input();

        update();

        render();

        // LIMIT TO 60FPS
        et = SDL_GetTicks() - et;
        if (et < FRAME_CAP) {
            SDL_Delay(FRAME_CAP - et);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

/*----------------------------------------------------------------------------*/

void init() {
    init_ram();
    init_renderer();
    loadRoom(1);

    drawTilebuffer();
    drawStatents();

    // walk the objects and find which of them match current screen

    // those which do match should be added to the camera

    // the camera should have a redraw function which:
    // -- draws the background
    // -- draws objects
    // -- draws sprites
}

/*----------------------------------------------------------------------------*/

bool input() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return false;

        // KEYUP means KEYRELEASED, it's not !KEYDOWN
        if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
            case SDLK_1:
                gf.jump_to_L1 = true;
                break;

            case SDLK_2:
                gf.jump_to_L2 = true;
                break;
            }
        }
    }
    return true;
}

/*----------------------------------------------------------------------------*/

void update() {
    if (gf.jump_to_L1) {
        fprintf(stderr, "\nJump to L0");
        inflate_map(1);
        drawTilebuffer();
        gf.jump_to_L1 = false;
    }
    if (gf.jump_to_L2) {
        fprintf(stderr, "\nJump to L1");
        inflate_map(2);
        drawTilebuffer();
        gf.jump_to_L2 = false;
    }
}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }

/*----------------------------------------------------------------------------*/

void loadRoom(int roomID) {
    inflate_map(roomID);

    // static entities offsets
    // 0: type | length
    // 1: room_id
    // 2: room_i
    // 3: room_j
    // 4: ptr_vizdef
    uint8_t statent_idx = 0;
    for (uint16_t i = STATENTS; i < (STATENTS + 64); i += 2) {
        uint16_t ptr_statent = beebram[i] + (beebram[i + 1] << 8);
        uint8_t statent_roomID = beebram[ptr_statent + 1];
        if (statent_roomID == roomID) {
            // add the static entity to the camera
            beebram[CAMERA + 0x0D + 2 * statent_idx] = ptr_statent & 0xFF;
            beebram[CAMERA + 0x0D + 2 * statent_idx + 1] = ptr_statent >> 8;
            statent_idx++;
            if (statent_idx > 4)
                break;
        }

        continue;
    }
}

/*----------------------------------------------------------------------------*/

void drawStatents() {
    // dont make static entities a part of the tile buffer

    // this function is used for the initial level draw

    // some statents will need to be composited, others simply drawn

    // simpler just to treat both the same way

    // a static entity, unlike a sprite, will overlay at most 4 tiles

    // that entity's i,j can be the top left corner of the offbuffer

    // this means we will fetch more tiles than strictly necessary but its ok

    // cyclce through statents in the camera
    for (uint16_t i = (CAMERA + 0x0D); i < ((CAMERA + 0x0D) + 10); i += 2) {

        // for each statent pointer, go to the statent and get the length
        uint16_t ptr_statent = beebram[i] + (beebram[i + 1] << 8);
        if (ptr_statent == 0)
            break;

        // static entities offsets
        // 0: type | length
        // 1: room_id
        // 2: room_i
        // 3: room_j
        // 4: ptr_vizdef
        uint8_t type = beebram[ptr_statent] >> 4;
        uint8_t length = beebram[ptr_statent] & 0b00001111;
        uint8_t room_id = beebram[ptr_statent + 1];
        uint8_t i = beebram[ptr_statent + 2];
        uint8_t j = beebram[ptr_statent + 3];
        uint16_t ptr_vizdef = beebram[ptr_statent + 4] + (beebram[ptr_statent + 5] << 8);

        // if 0x3300 < ptr_vizdef < 0x3500, we are looking at a quaddef

        // if 0x3500 < ptr_vizdef < 0x3600, we are looking at an animdef

        // just do a quaddef for now

        // if 0x3480 (0x3300 + 48*8) < quaddef < 0x3500, that's a composite pair

        // first fill the offbuffer with relevant tiles for i,j from the tilebuffer
        // the (i,j) for the static entity will be the 26x40 version, not 13x20

        // copy from tilebuffer to offbuffer:
        //[i+0_j+0, i+0_j+1, i+0_j+2]
        //[i+1_j+0, i+1_j+1, i+1_j+2]
        //[i+2_j+0, i+2_j+1, i+2_j+2]

        // tilebuffer holds tileIDs, 26x40 resolution from [0x3980, 0x3D90), 1 byte per tileid

        // offbuffer holds pixels, from [0x3DB0, 0x3DB4), 1 byte per tileid

        // lets get our tileIDs from the tilebuffer first, there will be 4 of them
        // our static entity is located at (10,10), i think this is tread tile
        uint8_t tileIDTL = beebram[TILEBUFFER + i * 40 + j];           // 8
        uint8_t tileIDTR = beebram[TILEBUFFER + i * 40 + j + 1];       // 9
        uint8_t tileIDBL = beebram[TILEBUFFER + (i + 1) * 40 + j];     // 10
        uint8_t tileIDBR = beebram[TILEBUFFER + (i + 1) * 40 + j + 1]; // 11

        // resolve to the textures and paint to the offbuffer
        uint16_t tileptr = getTileTextureAddr(tileIDTL); // gives 0x2520, 0x20 is 32 which is correct
        uint16_t screenpos = OFFBUFFER;
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[tileptr + s];
        }
        screenpos += 8;

        tileptr = getTileTextureAddr(tileIDTR);
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[tileptr + s];
        }
        screenpos += 8;

        tileptr = getTileTextureAddr(tileIDBL);
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[tileptr + s];
        }
        screenpos += 8;

        tileptr = getTileTextureAddr(tileIDBR);
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[tileptr + s];
        }

        // bg is in the offbuffer, now paint the statent over it
        // the statents ptr_vizdef will point to a quad pointing to 4 textures
        // this should give the 4 textures for the locked door icon
        uint16_t q0ptr = beebram[ptr_vizdef + 0] + (beebram[ptr_vizdef + 1] << 8); // 0x25b0
        uint16_t q1ptr = beebram[ptr_vizdef + 2] + (beebram[ptr_vizdef + 3] << 8); // 0x25b8
        uint16_t q2ptr = beebram[ptr_vizdef + 4] + (beebram[ptr_vizdef + 5] << 8); // 0x25c0
        uint16_t q3ptr = beebram[ptr_vizdef + 6] + (beebram[ptr_vizdef + 7] << 8); // 0x25c8

        // because ptr_vizdef (0x3408) < QUADS + 48*2*4 (0x3480), we know no compositing is used
        // so we simply overwrite the offbuffer
        screenpos = OFFBUFFER;
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[q0ptr + s];
        }
        screenpos += 8;

        tileptr = getTileTextureAddr(tileIDTR);
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[q1ptr + s];
        }
        screenpos += 8;

        tileptr = getTileTextureAddr(tileIDBL);
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[q2ptr + s];
        }
        screenpos += 8;

        tileptr = getTileTextureAddr(tileIDBR);
        for (int s = 7; s >= 0; s--) {
            beebram[screenpos + s] = beebram[q3ptr + s];
        }

        // slap the offbuffer back to screen, only need first 4 tiles in TL/TR/BL/BR
        uint16_t TL = 0x5800 + i * 0x0140 + j * 8;
        uint16_t TR = TL + 8;
        uint16_t BL = TL + 0x0140;
        uint16_t BR = BL + 8;

        for (int s = 7; s >= 0; s--) {
            beebram[TL + s] = beebram[OFFBUFFER + s];
        }
        for (int s = 7; s >= 0; s--) {
            beebram[TR + s] = beebram[OFFBUFFER + 8 + s];
        }
        for (int s = 7; s >= 0; s--) {
            beebram[BL + s] = beebram[OFFBUFFER + 16 + s];
        }
        for (int s = 7; s >= 0; s--) {
            beebram[BR + s] = beebram[OFFBUFFER + 24 + s];
        }

        continue;
    }
}
