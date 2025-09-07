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

    // for each entity in view of the camera
    for (uint16_t i = (CAMERA + 0x0D); i < ((CAMERA + 0x0D) + 10); i += 2) {

        uint16_t ptr_statent = beebram[i] + (beebram[i + 1] << 8);
        if (ptr_statent == 0)
            break;

        // entity fields
        uint8_t se_type = beebram[ptr_statent] >> 4;
        uint8_t se_length = beebram[ptr_statent] & 0b00001111;
        uint8_t se_roomID = beebram[ptr_statent + 1];
        uint8_t se_i = beebram[ptr_statent + 2];
        uint8_t se_j = beebram[ptr_statent + 3];
        uint16_t se_vizdef = beebram[ptr_statent + 4] + (beebram[ptr_statent + 5] << 8);

        if (se_vizdef < 0x3500)
            goto quaddef;

    animdef:
        continue;

    quaddef:
        if (se_vizdef < (0x3300 + 48 * 8))
            goto plaindef;

    compdef:
        continue;

    plaindef:

        // fetch tileids from the tilebuffer corresponding to 16x16px entity area
        uint8_t bgtileids[8] = {
            beebram[TILEBUFFER + se_i * 40 + se_j],
            beebram[TILEBUFFER + se_i * 40 + se_j + 1],
            beebram[TILEBUFFER + (se_i + 1) * 40 + se_j],
            beebram[TILEBUFFER + (se_i + 1) * 40 + se_j + 1]};

        // paint those background tile pixels into the offbuffer
        uint16_t screenpos = OFFBUFFER;
        for (int i = 0; i < 4; i++) {
            uint16_t texture_addr = getTileTextureAddr(bgtileids[i]);
            uint16_t screenpos = OFFBUFFER;
            for (int s = 7; s >= 0; s--) {
                beebram[screenpos + s] = beebram[texture_addr + s];
            }
            screenpos += 8;
        }

        // get the texture addresses from the quad (via ptr_vizdef)
        uint16_t texture_ptrs[8] = {
            beebram[se_vizdef + 0] + (beebram[se_vizdef + 1] << 8), // 0x25b0
            beebram[se_vizdef + 2] + (beebram[se_vizdef + 3] << 8), // 0x25b8
            beebram[se_vizdef + 4] + (beebram[se_vizdef + 5] << 8), // 0x25c0
            beebram[se_vizdef + 6] + (beebram[se_vizdef + 7] << 8)  // 0x25c8
        };

        // no compositing for a plaindef, so just overwrite the offbuffer
        screenpos = OFFBUFFER;
        for (int i = 0; i < 4; i++) {
            uint16_t texture = texture_ptrs[i];
            for (int s = 7; s >= 0; s--) {
                beebram[screenpos + s] = beebram[texture + s];
            }
            screenpos += 8;
        }

        // paint the 4 tiles used in the offbuffer back to screen
        uint16_t penbase = 0x5800 + se_i * 0x140 + se_j * 8;
        for (int s = 7; s >= 0; s--) {
            beebram[penbase + s] = beebram[OFFBUFFER + s];
            beebram[penbase + s + 8] = beebram[OFFBUFFER + 8 + s];
            beebram[penbase + s + 320] = beebram[OFFBUFFER + 16 + s];
            beebram[penbase + s + 328] = beebram[OFFBUFFER + 24 + s];
        }

        continue;
    }
}
