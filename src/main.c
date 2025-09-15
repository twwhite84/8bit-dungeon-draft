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
void animateStaticEntities();

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
    renderStaticEntities();
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
        loadRoom(1);
        drawTilebuffer();
        renderStaticEntities();
        gf.jump_to_L1 = false;
    }
    if (gf.jump_to_L2) {
        fprintf(stderr, "\nJump to L1");
        loadRoom(2);
        drawTilebuffer();
        renderStaticEntities();
        gf.jump_to_L2 = false;
    }

    animateStaticEntities();
    renderStaticEntities();
}

/*----------------------------------------------------------------------------*/

// walk the camera buffer and update any static entities which have animated tiles
void animateStaticEntities() {
    uint8_t se_n = beebram[CAMERA + 0x0C];
    uint16_t se_ptr = CAMERA + 0x0D;

    for (int i = 0; i < se_n; i++) {
        // fetch the next static entity
        uint16_t se_addr = beebram[se_ptr] + (beebram[se_ptr + 1] << 8);
        se_ptr += 2;

        // get its first vizdef (you only need one to determine if the entity is animated)
        uint16_t se_pvizdef = beebram[se_addr + SE_PVIZDEF_LO] + (beebram[se_addr + SE_PVIZDEF_HI] << 8);

        // skip if not an animdef
        if (se_pvizdef < ANIMDEFS)
            continue;

        // if animdef, it will be a pointer from animdef index to an animdef def
        uint16_t animdef = (beebram[se_pvizdef] & 0xFF) | (beebram[se_pvizdef + 1] << 8);
        uint8_t se_elapsed_frames = (beebram[se_addr + SE_ELAPSED5_TYPE3] & 0b11111000) >> 3;
        uint8_t frames = ((beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b11100000) >> 5) + 1;
        uint8_t current = (beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b00011100) >> 2;
        uint8_t yoyo = beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] & 0b00000011;
        uint8_t period;

        // update the static entity's elapsed frame count
        se_elapsed_frames++;
        beebram[se_addr + SE_ELAPSED5_TYPE3] &= 0b00000111;
        beebram[se_addr + SE_ELAPSED5_TYPE3] |= (se_elapsed_frames << 3);

        // fetch the period for the current frame index
        switch (current) {
        case 0:
            period = (beebram[animdef + AD_PERIOD0_PERIOD1] & 0b11110000) >> 4;
            break;

        case 1:
            period = (beebram[animdef + AD_PERIOD0_PERIOD1] & 0b00001111);
            break;

        case 2:
            period = (beebram[animdef + AD_PERIOD2_PERIOD3] & 0b11110000) >> 4;
            break;

        case 3:
            period = (beebram[animdef + AD_PERIOD2_PERIOD3] & 0b00001111);
            break;
        }

        // if the elapsed frame count > period, cycle the frame and reset the elapsed
        if (se_elapsed_frames > period) {
            current++;
            if (current == frames)
                current = 0;

            // write current
            beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] &= 0b11100011;
            beebram[animdef + AD_FRAMES3_CURRENT3_YOYO2] |= (current << 2);

            // write elapsed frames
            se_elapsed_frames = 0;
            beebram[se_addr + SE_ELAPSED5_TYPE3] &= 0b00000111;
            beebram[se_addr + SE_ELAPSED5_TYPE3] |= (se_elapsed_frames << 3);

            // raise redraw flag so that renderStaticEntities() draws it
            beebram[se_addr + SE_REDRAW1_DATA7] |= 0b10000000;
        }
    }
}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }

/*----------------------------------------------------------------------------*/

void loadRoom(int roomID) {
    memset(&beebram[TILEBUFFER], 0, (size_t)(OFFBUFFER - TILEBUFFER));
    inflate_map(roomID);

    // find the static entities for this room and copy their pointers into camera
    uint8_t se_count = 0;
    uint16_t se_ptr = STATENTS, camera_se_ptr_base = CAMERA + 0x0D;
    for (int i = 0; i < 32; i++) {
        uint16_t se_addr = beebram[se_ptr] + (beebram[se_ptr + 1] << 8);
        se_ptr += 2;

        uint8_t se_roomID = (beebram[se_addr + SE_NQUADS2_ROOMID6] & 0b00111111);
        if (se_roomID == roomID) {
            // add the static entity to the camera
            uint16_t camera_se_ptr = camera_se_ptr_base + (2 * se_count);
            beebram[camera_se_ptr + 0] = se_addr & 0xFF; // lo
            beebram[camera_se_ptr + 1] = se_addr >> 8;   // hi

            // its redraw flag should be raised on insert
            beebram[se_addr + SE_REDRAW1_DATA7] |= 0b10000000;

            se_count++;
            if (se_count > 4)
                break;
        }
    }
    beebram[CAMERA + 0x0C] = se_count;
}

/*----------------------------------------------------------------------------*/
