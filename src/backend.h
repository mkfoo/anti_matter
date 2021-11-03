#pragma once

#include <SDL.h>
#include "antimatter.h"

typedef struct {
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
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
    KU_UP,
    KU_DOWN,
    KU_LEFT,
    KU_RIGHT,
    IDLE,
} Event;

Backend* be_init(void);

Event be_get_event(Backend* be);

void be_present(Backend* be);

void be_blit_tile(Backend* be, int x, int y, int n);

void be_blit_text(Backend* be, int x, int y, char* str);

void be_fill_rect(Backend* be, int x, int y, int w, int h, int color);

uint32_t be_get_millis(void);

void be_delay(uint32_t dur);

void be_quit(Backend* be);
