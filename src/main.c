#include "animate.h"
#include "inflate.h"
#include "init.h"
#include "renderer.h"
#include "shared.h"
#include <stdbool.h>

typedef struct {
    bool level_1;
    bool level_2;
    bool player_moveLeft;
    bool player_moveRight;
    bool player_moveUp;
    bool player_moveDown;
} GameFlags;

void init();
bool input();
void update();
void loadRoom(int roomID);
void movePlayer(uint8_t dir);
void updateSpriteContainer(uint16_t actor);
void bufferSpriteBackground(uint16_t actor);
void bufferSpriteForeground(uint16_t actor);

GameFlags gameFlags;

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
    }

    const uint8_t *keystates = SDL_GetKeyboardState(NULL);
    if (keystates[SDL_SCANCODE_UP])
        gameFlags.player_moveUp = true;
    if (keystates[SDL_SCANCODE_DOWN])
        gameFlags.player_moveDown = true;
    if (keystates[SDL_SCANCODE_LEFT])
        gameFlags.player_moveLeft = true;
    if (keystates[SDL_SCANCODE_RIGHT])
        gameFlags.player_moveRight = true;
    if (keystates[SDL_SCANCODE_1])
        gameFlags.level_1 = true;
    if (keystates[SDL_SCANCODE_2])
        gameFlags.level_2 = true;

    return true;
}

/*----------------------------------------------------------------------------*/

void update() {
    if (gameFlags.level_1) {
        fprintf(stderr, "\nLoad room: 1");
        loadRoom(1);
        drawTilebuffer();
        renderStaticEntities();
        gameFlags.level_1 = false;
    }

    if (gameFlags.level_2) {
        fprintf(stderr, "\nLoad room: 2");
        loadRoom(2);
        drawTilebuffer();
        renderStaticEntities();
        gameFlags.level_2 = false;
    }

    if (gameFlags.player_moveLeft) {
        movePlayer(PLRDIR_W);
        gameFlags.player_moveLeft = false;
    }

    if (gameFlags.player_moveRight) {
        movePlayer(PLRDIR_E);
        gameFlags.player_moveRight = false;
    }

    if (gameFlags.player_moveUp) {
        movePlayer(PLRDIR_N);
        gameFlags.player_moveUp = false;
    }

    if (gameFlags.player_moveDown) {
        movePlayer(PLRDIR_S);
        gameFlags.player_moveDown = false;
    }

    animateStaticEntities();
    renderStaticEntities();

    // if player's redraw flag is raised, redraw
    uint8_t redraw = beebram[PLAYER + PLR_ROOM6_REDRAW2] & 0b11;
    if (redraw) {
        updateSpriteContainer(PLAYER);
        bufferSpriteBackground(PLAYER);
        bufferSpriteForeground(PLAYER);
        renderPlayer();
    }
}

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

void movePlayer(uint8_t dir) {
    fprintf(stderr, "Player direction: %d\n", dir);

    // raise the redraw flag
    beebram[PLAYER + PLR_ROOM6_REDRAW2] &= 0b11111100;
    beebram[PLAYER + PLR_ROOM6_REDRAW2] |= true;

    // for now simply increment/decrement the player x or y
    uint16_t x, y;
    switch (dir) {
    case PLRDIR_E:
        x = beebram[PLAYER + PLR_X_LO] | (beebram[PLAYER + PLR_X_HI] << 8);
        x++;
        beebram[PLAYER + PLR_X_LO] = x & 0xFF;
        beebram[PLAYER + PLR_X_HI] = x >> 8;
        break;
    case PLRDIR_W:
        x = beebram[PLAYER + PLR_X_LO] | (beebram[PLAYER + PLR_X_HI] << 8);
        x--;
        beebram[PLAYER + PLR_X_LO] = x & 0xFF;
        beebram[PLAYER + PLR_X_HI] = x >> 8;
        break;
    case PLRDIR_N:
        y = beebram[PLAYER + PLR_Y_LO] | (beebram[PLAYER + PLR_Y_HI] << 8);
        y--;
        beebram[PLAYER + PLR_Y_LO] = y & 0xFF;
        beebram[PLAYER + PLR_Y_HI] = y >> 8;
        break;
    case PLRDIR_S:
        y = beebram[PLAYER + PLR_Y_LO] | (beebram[PLAYER + PLR_Y_HI] << 8);
        y++;
        beebram[PLAYER + PLR_Y_LO] = y & 0xFF;
        beebram[PLAYER + PLR_Y_HI] = y >> 8;
        break;
    }

    // set the player direction vector

    // check the next tile a pixel of movement in that direction will land

    // take appropriate action
}

void updateSpriteContainer(uint16_t actor) {
    // ONLY IMPLEMENTED FOR PLAYER FOR NOW

    // get current player position
    uint16_t x = beebram[actor + PLR_X_LO] | (beebram[actor + PLR_X_HI] << 8);
    uint16_t y = beebram[actor + PLR_Y_LO] | (beebram[actor + PLR_Y_HI] << 8);

    // compute and set the shifts for the sprite container
    uint8_t hshift = x & 0b111;
    uint8_t vshift = y & 0b111;
    beebram[actor + PLR_HSHIFT4_VSHIFT4] = (beebram[actor + PLR_HSHIFT4_VSHIFT4] & 0x0F) | (hshift << 4);
    beebram[actor + PLR_HSHIFT4_VSHIFT4] = (beebram[actor + PLR_HSHIFT4_VSHIFT4] & 0xF0) | vshift;

    // compute relative screen address for origin of sprite container (top-left corner)
    uint16_t corner_new = (y >> 3) * 0x0140 + (x >> 3) * 8;
    uint16_t corner_old = corner_new;

    // on first sprite container setup, set cleanup to false
    if (beebram[actor + PLR_ELAPSED6_CLEANUP2] & 0b11 == 2) {
        beebram[actor + PLR_ELAPSED6_CLEANUP2] &= 11111100;
    }

    // on subsequent computations, cleanup is raised if sprite container has moved
    else {
        corner_old = beebram[actor + PLR_PCORNER_LO] | (beebram[actor + PLR_PCORNER_HI] << 8);
        if ((corner_new - corner_old) != 0)
            beebram[actor + PLR_ELAPSED6_CLEANUP2] |= true;
    }

    // write the new sprite container corner to the player
    beebram[actor + PLR_PCORNER_LO] = corner_new & 0xFF;
    beebram[actor + PLR_PCORNER_HI] = corner_new >> 8;
}

/*----------------------------------------------------------------------------*/

// paint the background tiles for the sprite container into the offbuffer
void bufferSpriteBackground(uint16_t actor) {
    int boff = 0, roff = 0, t = 3;
    uint16_t corner = beebram[actor + PLR_PCORNER_LO] | (beebram[actor + PLR_PCORNER_HI] << 8);

    // for each of the 9 tiles of the sprite container
    for (int i = 8; i >= 0; i--) {
        // fetch the corresponding background tile id from the tilebuffer
        uint8_t tileID = beebram[TILEBUFFER + ((corner + roff + boff) >> 3)];

        // paint the background to the offbuffer
        uint16_t offstart = OFFBUFFER + boff;
        uint16_t bg_texture_addr = getTileTextureAddr(tileID);
        for (int s = 7; s >= 0; s--) {
            beebram[offstart + s] = beebram[bg_texture_addr + s];
        }
        t--;
        if (t == 0) {
            roff += 0x0128;
            t = 3;
        }
        boff += 8;
    }
}

/*----------------------------------------------------------------------------*/

void bufferSpriteForeground(uint16_t actor) {
    // only implemented for player for now
    int rshift = beebram[actor + PLR_HSHIFT4_VSHIFT4] >> 4;
    int lshift = 8 - rshift;

    int dshift = beebram[actor + PLR_HSHIFT4_VSHIFT4] & 0x0F;
    int ushift = 8 - dshift;

    uint16_t pvizdef = beebram[actor + PLR_PVIZDEF_LO] | (beebram[actor + PLR_PVIZDEF_HI] << 8);
    uint16_t pcompdef;

    // the vizdef can either be a compdef directly, or else a frame in an animdef
    if (pvizdef >= ANIMDEFS) {
        uint16_t panimdef = beebram[pvizdef] | (beebram[pvizdef + 1] << 8);
        uint8_t current = (beebram[panimdef + AD_FRAMES3_CURRENT3_YOYO2] >> 2) & 0b00000111;
        current *= 2;
        pcompdef = beebram[panimdef + AD_PQUADDEF_LO + current] | (beebram[panimdef + AD_PQUADDEF_HI + current] << 8);
    } else {
        pcompdef = pvizdef;
    }

    uint16_t penbase = OFFBUFFER + dshift;

    // position each portion of the quad into place
    int tidx_lo = 6; // [(0,1),(2,3),(4,5),(6,7)]
    for (int t = 3; t >= 0; t--) {
        uint16_t penstart = penbase + bhops[t]; // +15 and penstart -=16 to decrement inner loop

        uint16_t ptexture = beebram[pcompdef + tidx_lo] | (beebram[pcompdef + tidx_lo + 1] << 8);
        uint16_t pmask = beebram[pcompdef + 8 + tidx_lo] | (beebram[pcompdef + 8 + tidx_lo + 1] << 8);
        tidx_lo -= 2;

        // to get this to decrement, penstart -= 16 with initial penbase + 15 (or something)
        for (int s = 0; s < 8; s++) {
            uint8_t overL = beebram[ptexture + s] >> rshift;
            uint8_t overR = beebram[ptexture + s] << lshift;
            uint8_t maskL = beebram[pmask + s] >> rshift;
            uint8_t maskR = beebram[pmask + s] << lshift;

            if (s == ushift) {
                penstart += 16;
            }

            // lhs
            beebram[penstart + s] = beebram[penstart + s] & (maskL ^ 0xFF);
            beebram[penstart + s] = beebram[penstart + s] | overL;

            // rhs
            beebram[penstart + 8 + s] = beebram[penstart + 8 + s] & (maskR ^ 0xFF);
            beebram[penstart + 8 + s] = beebram[penstart + 8 + s] | overR;
        }
    }
}