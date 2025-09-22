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

    updateSpriteContainer(PLAYER);
    bufferSpriteBackground(PLAYER);
    bufferSpriteForeground(PLAYER);

    renderPlayer();
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
    beebram[actor + PLR_HSHIFT] = x & 0b111;
    beebram[actor + PLR_VSHIFT] = y & 0b111;

    // compute relative screen address for origin of sprite container (top-left corner)
    uint16_t corner_new = (y >> 3) * 0x0140 + (x >> 3) * 8;
    uint16_t corner_old = corner_new;

    // on first sprite container setup, set cleanup to false
    if (beebram[actor + PLR_CLEANUP] == 0xFF) {
        beebram[actor + PLR_CLEANUP] = false;
    }

    // on subsequent computations, cleanup is raised if sprite container has moved
    else {
        corner_old = beebram[actor + PLR_PCORNER_LO] | (beebram[actor + PLR_PCORNER_HI] << 8);
        if ((corner_new - corner_old) != 0)
            beebram[actor + PLR_CLEANUP] = true;
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
        uint8_t tileID = beebram[TILEBUFFER + ((corner + roff + boff) / 8)];

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
    int rshift = beebram[actor + PLR_HSHIFT];
    int lshift = 8 - rshift;
    int dshift = beebram[actor + PLR_VSHIFT];
    int ushift = 8 - dshift; // minv8

    // just assume a compdef for now, expand to animdef frame later
    uint16_t pcompdef = beebram[actor + PLR_PVIZDEF_LO] | (beebram[actor + PLR_PVIZDEF_HI] << 8);
    uint16_t penstart;

    // position each portion of the quad into place
    for (int t = 0; t < 4; t++) {
        penstart = OFFBUFFER + bhops[t] + dshift;

        uint16_t ptexture = beebram[pcompdef + (2 * t)] | (beebram[pcompdef + (2 * t) + 1] << 8);
        uint16_t pmask = beebram[pcompdef + 8 + (2 * t)] | (beebram[pcompdef + 8 + (2 * t) + 1] << 8);

        for (int j = 0; j < 8; j++) {
            uint8_t overL = beebram[ptexture + j] >> rshift;
            uint8_t overR = beebram[ptexture + j] << lshift;
            uint8_t maskL = beebram[pmask + j] >> rshift;
            uint8_t maskR = beebram[pmask + j] << lshift;

            if (j == ushift)
                penstart += 16;

            beebram[penstart + j] = beebram[penstart + j] & (maskL ^ 0xFF);
            beebram[penstart + j] = beebram[penstart + j] | overL;

            beebram[penstart + 8 + j] = beebram[penstart + 8 + j] & (maskR ^ 0xFF);
            beebram[penstart + 8 + j] = beebram[penstart + 8 + j] | overR;
        }

        // dummy code
        /* penstart = OFFBUFFER + (8 * t);
        for (int s = 0; s <= 7; s++) {
            beebram[penstart + s] = 0;
        } */

        /* penstart = bhops[t] + dshift;
        for (int s = 0; s <= (7 - dshift); s++) {
            // TL
            // beebram[OFFBUFFER + penstart + s] &= (0x00);
            beebram[OFFBUFFER + penstart + s] = (beebram[TEXTURES + s] >> rshift);

            // TR
            // beebram[OFFBUFFER + penstart + 8 + s] &= (0x00);
            beebram[OFFBUFFER + penstart + 8 + s] = (beebram[TEXTURES + s] << lshift);
        }

        penstart = bhops[t] + (24 - ushift);
        for (int s = (8 - dshift); s <= 7; s++) {
            // BL
            // beebram[OFFBUFFER + penstart + s] &= (0x00);
            beebram[OFFBUFFER + penstart + s] = (beebram[TEXTURES + s] >> rshift);

            // BR
            // beebram[OFFBUFFER + penstart + 8 + s] &= (0x00);
            beebram[OFFBUFFER + penstart + 8 + s] = (beebram[TEXTURES + s] << lshift);
        } */

        /* penstart = bhops[t] + dshift;
        for (int s = 0; s <= (7 - dshift); s++) {
            // TL
            beebram[OFFBUFFER + penstart + s] &= ((beebram[pmask + s] ^ 0xFF) >> rshift);
            beebram[OFFBUFFER + penstart + s] |= ((beebram[ptexture + s] & beebram[pmask + s]) >> rshift);

            // TR
            beebram[OFFBUFFER + penstart + 8 + s] &= ((beebram[pmask + s] ^ 0xFF) << lshift);
            beebram[OFFBUFFER + penstart + 8 + s] |= ((beebram[ptexture + s] & beebram[pmask + s]) << lshift);
        }

        penstart = bhops[t] + (24 - ushift);
        for (int s = (8 - dshift); s <= 7; s++) {
            // BL
            beebram[OFFBUFFER + penstart + s] &= ((beebram[pmask + s] ^ 0xFF) >> rshift);
            beebram[OFFBUFFER + penstart + s] |= ((beebram[ptexture + s] & beebram[pmask + s]) >> rshift);

            // BR
            beebram[OFFBUFFER + penstart + 8 + s] &= ((beebram[pmask + s] ^ 0xFF) << lshift);
            beebram[OFFBUFFER + penstart + 8 + s] |= ((beebram[ptexture + s] & beebram[pmask + s]) << lshift);
        } */
    }
}

/* DEF PROC_BufferSprite(sprite%, buffer%)
  REM BHOPS=8*[0,1,3,4]

  REM Composite sprite at offset
  rshift%=2^?buffer%:REM hshift
  lshift%=2^(8-?buffer%)
  vshift%=?(buffer%+1):REM vshift
  bfrspace%=?(buffer%+4)+?(buffer%+5)*256
  q%=48
  minv8%=8-vshift%

  FOR i%=3 TO 0 STEP -1
    penstart%=bfrspace%+?(&4752+i%)+vshift%
    sq%=sprite%+q%
    s8%=sq%+8
    FOR j%=0 TO 7
      overL%=?(sq%+j%) DIV rshift%
      overR%=?(sq%+j%) * lshift%
      maskL%=?(s8%+j%) DIV rshift%
      maskR%=?(s8+j%) * lsfhit%

      IF j%=minv8% penstart%=penstart%+16

      REM mask and comp - left
      ?(penstart%+j%)=?(penstart%+j%) AND (maskL% EOR &FF)
      ?(penstart%+j%)=?(penstart%+j%) OR overL%

      REM mask and comp - right
      ?(penstart%+8+j%)=?(penstart%+8+j%) AND (maskR% EOR &FF)
      ?(penstart%+8+j%)=?(penstart%+8+j%) OR overR%
    NEXT j%
    q%=q%-16
  NEXT i%
ENDPROC */