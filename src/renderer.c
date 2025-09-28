#include "renderer.h"
#include "shared.h"

typedef struct {
    void *pixels;
    int pitch;
} CanvasContext;

SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *canvas = NULL;

static void plot(int x, int y, int color, CanvasContext ctx);

/*----------------------------------------------------------------------------*/

void init_renderer() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        printf("ERROR: %s\n", SDL_GetError());

    window = SDL_CreateWindow("8-BIT ENGINE WIP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, EXTERNAL_WIDTH,
                              EXTERNAL_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("ERROR: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        printf("ERROR: %s\n", SDL_GetError());

    canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, INTERNAL_WIDTH,
                               INTERNAL_HEIGHT);
    if (!canvas) {
        printf("ERROR: %s\n", SDL_GetError());
    }
}

/*----------------------------------------------------------------------------*/

void renderBackground() {
    for (uint8_t i = 0; i < 26; i++) {
        for (uint8_t j = 0; j < 40; j++) {
            eraseTile(i, j);
        }
    }
}

/*----------------------------------------------------------------------------*/

// i,j are in 26x40 layout
void eraseTile(uint8_t i, uint8_t j) {
    uint8_t tid = beebram[CAMBUFFER + 40 * i + j];
    uint16_t tileptr = getTileTextureAddr(tid);
    uint16_t screenpos = SCREEN + (0x0140 * i) + (8 * j);

    // using int in the loop because uint8_t screws up on wrap around
    for (int s = 7; s >= 0; s--) {
        beebram[screenpos + s] = beebram[tileptr + s];
    }
}

/*----------------------------------------------------------------------------*/

void renderBeebram() {
    CanvasContext ctx;
    if (SDL_LockTexture(canvas, NULL, &ctx.pixels, &ctx.pitch) < 0) {
        printf("\nERROR: Couldnt lock texture. %s", SDL_GetError());
        return;
    }

    int screenbase = 0x5800;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 40; j++) {
            for (int s = 0; s < 8; s++) {
                uint16_t memloc = screenbase + i * 0x140 + j * 8 + s;
                uint8_t stripe = beebram[memloc];
                for (int p = 7; p >= 0; p--) {
                    int value = ((stripe >> (7 - p)) & 0x01);
                    int x = j * 8 + p;
                    int y = i * 8 + s;
                    plot(x, y, value, ctx);
                }
            }
        }
    }

    SDL_UnlockTexture(canvas);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, canvas, NULL, NULL);
    SDL_RenderPresent(renderer);
}

/*----------------------------------------------------------------------------*/

static void plot(int x, int y, int color, CanvasContext ctx) {
    uint32_t *pixel_buffer = (uint32_t *)ctx.pixels;

    // 32-bit pixel buffer, so 4 bytes per pixel
    int xy_index = y * (ctx.pitch / 4) + x;

    switch (color) {
    case 1:
        pixel_buffer[xy_index] = 0xFFFFFFFF;
        break;
    case 0:
        pixel_buffer[xy_index] = 0;
        break;
    }
}

/*----------------------------------------------------------------------------*/

void bufferBG(uint8_t abs_i, uint8_t abs_j, uint8_t dim) {
    uint16_t penstart = OFFBUFFER;

    for (int rel_i = 0; rel_i < dim; rel_i++) {
        for (int rel_j = 0; rel_j < dim; rel_j++) {
            uint8_t tileID = beebram[CAMBUFFER + 40 * (abs_i + rel_i) + (abs_j + rel_j)];
            uint16_t texture = getTileTextureAddr(tileID);
            bufferTile(penstart, texture, 0xFFFF);
            penstart += 8;
        }
    }
}

/*----------------------------------------------------------------------------*/

void renderFGQuadToBuffer(uint16_t pquad) {
    uint16_t penstart = OFFBUFFER;
    uint16_t mask = 0xFFFF, texture = 0;

    for (uint8_t tile = 0; tile < 4; tile++) {
        texture = beebram[pquad + 2 * tile] + (beebram[pquad + 2 * tile + 1] << 8);

        if (pquad >= QUADS_COMP)
            mask = beebram[pquad + 8 + 2 * tile] + (beebram[pquad + 8 + 2 * tile + 1] << 8);

        bufferTile(penstart, texture, mask);
        penstart += 8;
    }
}

/*----------------------------------------------------------------------------*/

void bufferTile(uint16_t penstart, uint16_t texture, uint16_t mask) {
    if (mask != 0xFFFF)
        goto compdef;

plaindef:
    for (int s = 7; s >= 0; s--) {
        beebram[penstart + s] = beebram[texture + s];
    }
    return;

compdef:
    uint8_t hflipped = texture >> 15;

    if (!hflipped) {
        for (int s = 7; s >= 0; s--) {
            beebram[penstart + s] &= (beebram[mask + s] ^ 0xFF);
            beebram[penstart + s] |= (beebram[texture + s] & beebram[mask + s]);
        }
    }

    else {
        texture &= 0x7FFF;
        mask &= 0x7FFF;
        for (int s = 7; s >= 0; s--) {
            uint8_t mask_data = beebram[mask + s];
            uint8_t texture_data = beebram[texture + s];
            beebram[penstart + s] &= (beebram[LUT_REVERSE + mask_data] ^ 0xFF);
            beebram[penstart + s] |= (beebram[LUT_REVERSE + texture_data] & beebram[LUT_REVERSE + mask_data]);
        }
    }
}

/*----------------------------------------------------------------------------*/

void renderSEQuad(uint16_t pvizdef, uint8_t abs_i, uint8_t abs_j) {
    bufferBG(abs_i, abs_j, 2);
    renderFGQuadToBuffer(pvizdef);

render:
    // uint16_t penstart = SCREEN + abs_i * 0x140 + abs_j * 8;
    // for (int s = 7; s >= 0; s--) {
    //     beebram[penstart + s] = beebram[OFFBUFFER + s];
    //     beebram[penstart + s + 8] = beebram[OFFBUFFER + 8 + s];
    //     beebram[penstart + s + 320] = beebram[OFFBUFFER + 16 + s];
    //     beebram[penstart + s + 328] = beebram[OFFBUFFER + 24 + s];
    // }
    renderWhatever(abs_i, abs_j, 2);
}

/*----------------------------------------------------------------------------*/

// draws any static entities held in camera if their redraw flag is raised
void renderStaticEntities() {

    uint16_t se_ptr = CAMERA + CAM_PSE0_LO;
    uint8_t se_n = beebram[CAMERA + CAM_NME4_NSE4] & 0x0F;
    for (int i = 0; i < se_n; i++) {

        // get the entity
        uint16_t se_addr = beebram[se_ptr] + (beebram[se_ptr + 1] << 8);
        se_ptr += 2;

        // skip entity if redraw flag down, otherwise lower flag to skip in future calls
        uint8_t se_redraw = (beebram[se_addr + CE_ROOMID6_REDRAW2] & 0b00000011);
        if (!se_redraw)
            continue;
        else
            beebram[se_addr + CE_ROOMID6_REDRAW2] &= 0b11111100;

        uint8_t se_nquads = beebram[se_addr + SE_TYPE4_NQUADS4] & 0x0F;
        for (int q = 0; q < se_nquads; q++) {
            uint8_t qi = beebram[(se_addr + CE_I) + (4 * q)]; // 4q because repeating section 4 fields long
            uint8_t qj = beebram[(se_addr + CE_J) + (4 * q)];
            uint16_t pvizdef =
                beebram[(se_addr + CE_PVIZDEF_LO) + (4 * q)] | (beebram[(se_addr + CE_PVIZDEF_HI) + (4 * q)] << 8);

            // if not animated, jump ahead to directly rendering the quad
            if (pvizdef < ANIMDEFS) {
                renderSEQuad(pvizdef, qi, qj);
                continue;
            }

        animdef:
            uint16_t animdef = pvizdef;
            uint8_t current = beebram[se_addr + CE_FELAPSED5_FCURRENT3] & 0b00000111;

            // get the current frame for rendering to offbuffer
            pvizdef = beebram[(animdef + AD_PFRAME_LO) + (2 * current)];
            pvizdef |= (beebram[(animdef + AD_PFRAME_HI) + (2 * current)] << 8);
            renderSEQuad(pvizdef, qi, qj);
        }
    }
}

/*----------------------------------------------------------------------------*/

// PAINTS THE OFFBUFFER TO THE SCREEN
void renderPlayer() {
    // uint16_t corner = ij2ramloc(beebram[PLAYER + CE_I], beebram[PLAYER + CE_J]);
    // uint16_t penbase = SCREEN + corner;
    // for (int s = 7; s >= 0; s--) {
    //     beebram[penbase + s] = beebram[OFFBUFFER + s];
    //     beebram[penbase + s + 8] = beebram[OFFBUFFER + 8 + s];
    //     beebram[penbase + s + 16] = beebram[OFFBUFFER + 16 + s];
    //     beebram[penbase + s + 320] = beebram[OFFBUFFER + 24 + s];
    //     beebram[penbase + s + 328] = beebram[OFFBUFFER + 32 + s];
    //     beebram[penbase + s + 336] = beebram[OFFBUFFER + 40 + s];
    //     beebram[penbase + s + 640] = beebram[OFFBUFFER + 48 + s];
    //     beebram[penbase + s + 648] = beebram[OFFBUFFER + 56 + s];
    //     beebram[penbase + s + 656] = beebram[OFFBUFFER + 64 + s];
    // }
    uint8_t i = beebram[PLAYER + CE_I];
    uint8_t j = beebram[PLAYER + CE_J];
    renderWhatever(i, j, 3);

    beebram[PLAYER + CE_ROOMID6_REDRAW2] &= 0b11111100;
}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }

/*----------------------------------------------------------------------------*/

void renderWhatever(uint8_t abs_i, uint8_t abs_j, uint8_t dim) {
    uint16_t penstart = SCREEN + abs_i * 0x140 + abs_j * 8;
    uint16_t offbase = OFFBUFFER;

    for (int rel_i = 0; rel_i < dim; rel_i++) {
        for (int rel_j = 0; rel_j < dim; rel_j++) {
            for (int s = 7; s >= 0; s--) {
                beebram[penstart + s] = beebram[offbase + s];
            }
            offbase += 8;
            penstart += 8;
        }
        penstart += 320 - (dim * 8);
    }
}