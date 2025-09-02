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
static uint16_t getTileTextureAddr(uint8_t tid);

/*----------------------------------------------------------------------------*/

void init_renderer() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        printf("ERROR: %s\n", SDL_GetError());

    window = SDL_CreateWindow("BEEB MODE 4", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, EXTERNAL_WIDTH,
                              EXTERNAL_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("ERROR: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        printf("ERROR: %s\n", SDL_GetError());

    canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_STREAMING, INTERNAL_WIDTH,
                               INTERNAL_HEIGHT);
    if (!canvas) {
        printf("ERROR: %s\n", SDL_GetError());
    }
}

/*----------------------------------------------------------------------------*/

void eraseScreen() {
    for (uint8_t i = 0; i < 26; i++) {
        for (uint8_t j = 0; j < 40; j++) {
            eraseTile(i, j);
        }
    }
}

/*----------------------------------------------------------------------------*/

void eraseTile(uint8_t i, uint8_t j) {
    uint8_t tid = beebram[TILEBUFFER + 40 * i + j];
    uint16_t tileptr = getTileTextureAddr(tid);
    uint16_t screenpos = 0x5800 + (0x0140 * i) + (8 * j);

    // using int in the loop because uint8_t screws up on wrap around
    for (int s = 7; s >= 0; s--) {
        beebram[screenpos + s] = beebram[tileptr + s];
    }
}

/*----------------------------------------------------------------------------*/

uint16_t getTileTextureAddr(uint8_t tid) {
    uint16_t ptr_location = QUADDEFS + 2 * tid;
    uint16_t ptr = beebram[ptr_location] + (beebram[ptr_location + 1] << 8);
    return ptr;
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
