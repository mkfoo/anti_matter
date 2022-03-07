#pragma once
#include "antimatter.h"
#include "sound.h"

#ifdef WASM_BACKEND

typedef struct {
    float* ptr;
    size_t offset;
    size_t len;
    size_t cap;
    int prev_dr;
    int prev_dt;
} VertexBuf;

typedef struct {
    int gl;
    int stat;
    VertexBuf sprites;
    VertexBuf lines;
} Backend;

#else

#include <SDL.h>

typedef struct {
    int gl;
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* tex;
    SDL_Texture* stx;
    SDL_AudioDeviceID dev;
    SoundGen* snd;
} Backend;

#endif

typedef enum {
    IDLE,
    QUIT,
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
    KD_F10,
} Event;

Backend* be_init(int gl);
Event be_get_event(Backend* be);
void be_set_color(Backend* be, int color);
void be_set_render_target(Backend* be, int tgt);
void be_clear(Backend* be);
void be_present(Backend* be);
void be_blit_tile(Backend* be, int x, int y, int n);
void be_blit_text(Backend* be, int x, int y, char* str);
void be_blit_static(Backend* be);
void be_draw_line(Backend* be, int x1, int y1, int x2, int y2);
void be_fill_rect(Backend* be, int x, int y, int w, int h);
void be_send_audiomsg(Backend* be, int msg);
uint64_t be_get_millis(void);
void be_delay(uint32_t dur);
void be_quit(Backend* be);
