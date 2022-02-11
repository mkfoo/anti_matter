#pragma once
#include "antimatter.h"

#ifdef WASM_BACKEND

typedef struct {
    int dummy_value;
} Backend;

#else

#include <SDL.h>

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
    SDL_AudioDeviceID dev;
} Backend;

#endif

typedef enum {
    QUIT,
    IDLE,
    KD_ESC,
    KD_UP,
    KD_DOWN,
    KD_LEFT,
    KD_RIGHT,
    KD_SPC,
    KD_F1,
    KD_F2,
    KD_F3,
    KD_F4,
    KD_F5,
    KD_F6,
} Event;

Backend* be_init(void);
Event be_get_event(Backend* be);
void be_present(Backend* be);
void be_blit_tile(Backend* be, int x, int y, int n);
void be_blit_text(Backend* be, int x, int y, char* str);
void be_fill_rect(Backend* be, int x, int y, int w, int h, int color);
void be_draw_line(Backend* be, int x1, int y1, int x2, int y2, int color);
void be_queue_audio(Backend* be, const int16_t* data, uint32_t len);
uint64_t be_get_millis(void);
void be_delay(uint32_t dur);
void be_quit(Backend* be);
