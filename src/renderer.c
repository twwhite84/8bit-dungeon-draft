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

// draws any static entities held in camera if their redraw flag is raised
void renderStaticEntities() {

    uint16_t penbase;
    uint16_t offbase;

    uint16_t se_ptr = CAMERA + CAM_PSE0_LO;
    uint8_t se_n = beebram[CAMERA + CAM_NME4_NSE4] & 0x0F;
    for (int i = 0; i < se_n; i++) {

        // get the entity
        uint16_t se_addr = beebram[se_ptr] + (beebram[se_ptr + 1] << 8);
        se_ptr += 2;

        // skip entity if redraw flag down, otherwise lower flag to skip in future calls
        uint8_t se_redraw = (beebram[se_addr + SE_ROOMID6_REDRAW2] & 0b00000011);
        if (!se_redraw)
            continue;
        else
            beebram[se_addr + SE_ROOMID6_REDRAW2] &= 0b11111100;

        uint8_t se_nquads = beebram[se_addr + SE_TYPE4_NQUADS4] & 0x0F;
        for (int q = 0; q < se_nquads; q++) {
            uint8_t se_TLi = beebram[(se_addr + SE_I) + (4 * q)]; // 4q because 4 fields per quad
            uint8_t se_TLj = beebram[(se_addr + SE_J) + (4 * q)];
            uint16_t se_vizdef =
                beebram[(se_addr + SE_PVIZDEF_LO) + (4 * q)] + (beebram[(se_addr + SE_PVIZDEF_HI) + (4 * q)] << 8);

            // if not animated, jump ahead to directly rendering the quad
            if (se_vizdef >= QUADDEFS && se_vizdef < ANIMDEFS) {
                goto quaddef;
            }

        animdef:
            uint16_t animdef = se_vizdef;
            uint8_t current = beebram[se_addr + SE_FELAPSED5_FCURRENT3] & 0b00000111;

            // get the current frame for rendering to offbuffer
            se_vizdef = beebram[(animdef + AD_PFRAME0_LB) + (2 * current)];
            se_vizdef |= (beebram[(animdef + AD_PFRAME0_HB) + (2 * current)] << 8);
            goto quaddef;

        quaddef:
            if (se_vizdef < Q_COMPDEFS)
                goto plaindef;

        compdef:

            // fetch corresponding background tiles and paint to offbuffer
            offbase = OFFBUFFER;
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    uint8_t tileID = beebram[CAMBUFFER + (se_TLi + i) * 40 + se_TLj + j];
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
                uint16_t mask = beebram[se_vizdef + 8 + 2 * i] + (beebram[se_vizdef + 8 + 2 * i + 1] << 8);
                uint8_t hflipped = texture >> 15;

                if (!hflipped) {
                    for (int s = 7; s >= 0; s--) {
                        beebram[offbase + s] &= (beebram[mask + s] ^ 0xFF);
                        beebram[offbase + s] |= (beebram[texture + s] & beebram[mask + s]);
                    }
                }

                else if (hflipped) {
                    texture &= 0x7FFF;
                    mask &= 0x7FFF;
                    for (int s = 7; s >= 0; s--) {
                        uint8_t mask_data = beebram[mask + s];
                        uint8_t texture_data = beebram[texture + s];

                        beebram[offbase + s] &= (beebram[LUT_REVERSE + mask_data] ^ 0xFF);
                        beebram[offbase + s] |=
                            (beebram[LUT_REVERSE + texture_data] & beebram[LUT_REVERSE + mask_data]);
                    }
                }

                offbase += 8;
            }

            // paint the offbuffer back to screen at static entity's coordinates
            penbase = SCREEN + se_TLi * 0x140 + se_TLj * 8;
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
                    uint8_t tileID = beebram[CAMBUFFER + (se_TLi + i) * 40 + se_TLj + j];
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

/*----------------------------------------------------------------------------*/

void renderPlayer() {
    uint16_t corner = beebram[PLAYER + PLR_PCORNER_LO] | (beebram[PLAYER + PLR_PCORNER_HI] << 8);
    uint16_t penbase = SCREEN + corner;
    for (int s = 7; s >= 0; s--) {
        beebram[penbase + s] = beebram[OFFBUFFER + s];
        beebram[penbase + s + 8] = beebram[OFFBUFFER + 8 + s];
        beebram[penbase + s + 16] = beebram[OFFBUFFER + 16 + s];
        beebram[penbase + s + 320] = beebram[OFFBUFFER + 24 + s];
        beebram[penbase + s + 328] = beebram[OFFBUFFER + 32 + s];
        beebram[penbase + s + 336] = beebram[OFFBUFFER + 40 + s];
        beebram[penbase + s + 640] = beebram[OFFBUFFER + 48 + s];
        beebram[penbase + s + 648] = beebram[OFFBUFFER + 56 + s];
        beebram[penbase + s + 656] = beebram[OFFBUFFER + 64 + s];
    }
    beebram[PLAYER + PLR_ROOM6_REDRAW2] &= 0b11111100;
}

/*----------------------------------------------------------------------------*/

void render() { renderBeebram(); }

/*----------------------------------------------------------------------------*/
