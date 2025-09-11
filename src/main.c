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
        drawStatents();
        gf.jump_to_L1 = false;
    }
    if (gf.jump_to_L2) {
        fprintf(stderr, "\nJump to L1");
        inflate_map(2);
        drawTilebuffer();
        drawStatents();
        gf.jump_to_L2 = false;
    }

    return;

    // walk the camera buffer and update any static entity animdefs
    uint16_t camera_se_base = CAMERA + 0x0D;
    uint8_t se_base_offset = 0;
    for (int i = 0; i < 5; i++) {
        uint16_t se_ptr_addr = camera_se_base + se_base_offset;

        // go to the se and process each of its vizdefs
        uint16_t se_addr = beebram[se_ptr_addr] + (beebram[se_ptr_addr + 1] << 8);
        uint16_t se_length = beebram[se_addr + 0] & 0b00001111;
        for (int vdidx = 0; vdidx < se_length; vdidx++) {
            uint16_t se_vizdef_ptr_addr = se_addr + 4 + 4 * vdidx;
            uint16_t se_vizdef_addr = beebram[se_vizdef_ptr_addr] + (beebram[se_vizdef_ptr_addr + 1] << 8);
            if (se_vizdef_addr >= ANIMDEFS) {
                fprintf(stderr, "animdef at %x\n", se_vizdef_addr);

                // it's an animdef, so update its current frame
                // the issue here is that the current frame is stored on the vizdef
                // but in a se of 3 tiles that same vizdef gets reused
                // so move the current frame to the se

                // increment the elapsed frames

                //
            }
            continue;
        }

        // go to the vizdef

        se_base_offset += 2;
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

    uint16_t penbase;
    uint16_t offbase;

    for (uint16_t i = (CAMERA + 0x0D); i < ((CAMERA + 0x0D) + 10); i += 2) {

        uint16_t ptr_statent = beebram[i] + (beebram[i + 1] << 8);
        if (ptr_statent == 0)
            break;

        // entity fields
        uint8_t se_type = beebram[ptr_statent] >> 4;
        uint8_t se_length = beebram[ptr_statent] & 0b00001111;
        uint8_t se_current_frame = (beebram[ptr_statent + 1] & 0b11000000) >> 6;
        uint8_t se_roomID = beebram[ptr_statent + 1] & 0b00111111;

        for (int t = 0; t < se_length; t++) {
            uint8_t se_TLi = beebram[ptr_statent + 2 + 4 * t];                                               // 6
            uint8_t se_TLj = beebram[ptr_statent + 3 + 4 * t];                                               // 7
            uint16_t se_vizdef = beebram[ptr_statent + 4 + 4 * t] + (beebram[ptr_statent + 5 + 4 * t] << 8); // 8 and 9

            if (se_vizdef >= 0x3300 && se_vizdef < 0x3500) {
                goto quaddef;
            }

            else if (se_vizdef >= 0x3500 && se_vizdef < 0x3540) {
                goto animdef;
            }

            else {
                continue;
            }

        animdef:
            // figure out what the current frame is

            // set the se_vizdef to point to that, then jump to quaddef block

            // animdef:
            // [frames|current|elapsed|yoyo]
            // [period 0 | 1 | 2 | 3]
            // [ptr_quad_0]
            // [ptr_quad_1]

            // temp: just get the pointer to frame 0
            uint16_t def = beebram[se_vizdef] + (beebram[se_vizdef + 1] << 8);
            uint8_t frames = (beebram[def + 0] & 0b11000000) >> 6;
            uint8_t elapsed = (beebram[def + 0] & 0b00111100) >> 2;
            uint8_t yoyo = beebram[def + 0] & 0b00000011;

            uint16_t frame = beebram[def + 3 + (1 * se_current_frame)] + (beebram[def + 3 + (1 * se_current_frame) + 1] << 8);
            se_vizdef = frame;
            goto quaddef;

        quaddef:
            if (se_vizdef < (0x3300 + 48 * 8))
                goto plaindef;

        compdef:

            // fetch corresponding background tiles and paint to offbuffer
            offbase = OFFBUFFER;
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    uint8_t tileID = beebram[TILEBUFFER + (se_TLi + i) * 40 + se_TLj + j];
                    uint16_t bg_texture_addr = getTileTextureAddr(tileID);
                    for (int s = 7; s >= 0; s--) {
                        beebram[offbase + s] = beebram[bg_texture_addr + s];
                    }
                    offbase += 8;
                }
            }

            // paint static entity textures into the offbuffer, compositing for a compdef
            offbase = OFFBUFFER;
            for (int i = 0; i < 4; i++) {
                uint16_t texture = beebram[se_vizdef + 2 * i] + (beebram[se_vizdef + 2 * i + 1] << 8);
                uint16_t mask = texture + 32;
                for (int s = 7; s >= 0; s--) {
                    beebram[offbase + s] &= (beebram[mask + s] ^ 0xFF);
                    beebram[offbase + s] |= (beebram[texture + s] & beebram[mask + s]);
                }
                offbase += 8;
            }

            // paint the offbuffer back to screen at static entity's coordinates
            penbase = 0x5800 + se_TLi * 0x140 + se_TLj * 8;
            for (int s = 7; s >= 0; s--) {
                beebram[penbase + s] = beebram[OFFBUFFER + s];
                beebram[penbase + s + 8] = beebram[OFFBUFFER + 8 + s];
                beebram[penbase + s + 320] = beebram[OFFBUFFER + 16 + s];
                beebram[penbase + s + 328] = beebram[OFFBUFFER + 24 + s];
            }

            continue;

        plaindef:

            // fetch corresponding background tiles and paint to offbuffer
            offbase = OFFBUFFER;
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    uint8_t tileID = beebram[TILEBUFFER + (se_TLi + i) * 40 + se_TLj + j];
                    uint16_t bg_texture_addr = getTileTextureAddr(tileID);
                    for (int s = 7; s >= 0; s--) {
                        beebram[offbase + s] = beebram[bg_texture_addr + s];
                    }
                    offbase += 8;
                }
            }

            // paint static entity textures into the offbuffer, no compositing for a plaindef
            offbase = OFFBUFFER;
            for (int i = 0; i < 4; i++) {
                uint16_t texture = beebram[se_vizdef + 2 * i] + (beebram[se_vizdef + 2 * i + 1] << 8);
                for (int s = 7; s >= 0; s--) {
                    beebram[offbase + s] = beebram[texture + s];
                }
                offbase += 8;
            }

            // paint the offbuffer back to screen at static entity's coordinates
            penbase = 0x5800 + se_TLi * 0x140 + se_TLj * 8;
            for (int s = 7; s >= 0; s--) {
                beebram[penbase + s] = beebram[OFFBUFFER + s];
                beebram[penbase + s + 8] = beebram[OFFBUFFER + 8 + s];
                beebram[penbase + s + 320] = beebram[OFFBUFFER + 16 + s];
                beebram[penbase + s + 328] = beebram[OFFBUFFER + 24 + s];
            }

            continue;
        }
    }
}
