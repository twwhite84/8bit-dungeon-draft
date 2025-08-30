#include "renderer.h"
#include "shared.h"

typedef struct {
  void *pixels;
  int pitch;
} CanvasContext;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *canvas;

void plot(int x, int y, int color, CanvasContext ctx);

void init_renderer() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    printf("ERROR: %s\n", SDL_GetError());

  window = SDL_CreateWindow("BEEB MODE 4", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN);
  if (!window) {
    printf("ERROR: %s\n", SDL_GetError());
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
    printf("ERROR: %s\n", SDL_GetError());

  canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                             SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (!canvas) {
    printf("ERROR: %s\n", SDL_GetError());
  }
}

void render() {
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
          int value = ((stripe >> p) & 0x01);
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

void plot(int x, int y, int color, CanvasContext ctx) {
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