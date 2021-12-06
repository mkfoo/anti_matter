#pragma once

#include <SDL.h>
#include "antimatter.h"

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
    SDL_AudioDeviceID dev;
} Backend;

typedef enum {
    QUIT,
    KD_ESC,
    KD_UP,
    KD_DOWN,
    KD_LEFT,
    KD_RIGHT,
    KD_SPC,
    KD_F1,
    IDLE,
} Event;

Backend* be_init(void);

Event be_get_event(Backend* be);

void be_present(Backend* be);

void be_blit_tile(Backend* be, int x, int y, int n);

void be_blit_text(Backend* be, int x, int y, char* str);

void be_fill_rect(Backend* be, int x, int y, int w, int h, int color);

void be_draw_line(Backend* be, int x1, int y1, int x2, int y2, int color);

void be_queue_audio(Backend* be, const uint8_t* data, uint32_t len);

uint32_t be_get_millis(void);

void be_delay(uint32_t dur);

void be_quit(Backend* be);
