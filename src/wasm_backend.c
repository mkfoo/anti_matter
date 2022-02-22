#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include "antimatter.h"
#include "backend.h"
#include "texture_data.h"

typedef struct {
    uint8_t* buf;
    int32_t width;
    int32_t height;
} PixelData;

__attribute__((export_name("wbe_load_pixel_data")))
PixelData* wbe_load_pixel_data(void);

__attribute__((import_name("wbe_get_keydown")))
int wbe_get_keydown(void);

__attribute__((import_name("wbe_texture_copy")))
void wbe_texture_copy(int sx, int sy, int sw, int sh, int dx, int dy);

__attribute__((import_name("wbe_fill_rect")))
void wbe_fill_rect(int x, int y, int w, int h, int c);

__attribute__((import_name("wbe_draw_line")))
void wbe_draw_line(int x1, int y1, int x2, int y2, int c);

__attribute__((import_name("wbe_toggle_scale_factor")))
void wbe_toggle_scale_factor(void);

__attribute__((import_name("wbe_send_audiomsg")))
void wbe_send_audiomsg(int msg);

PixelData* wbe_load_pixel_data(void) {
    PixelData* pd = calloc(1, sizeof(PixelData));
    LOG_ERR(pd == NULL, "alloc failure"); 
    pd->buf = (uint8_t*) TEXTURE_DATA;
    pd->width = TEXTURE_W;
    pd->height = TEXTURE_H; 
    return pd;
}

Backend* be_init(void) {
    Backend* be = calloc(1, sizeof(Backend));
    LOG_ERR(be == NULL, "alloc failure"); 
    return be;
}

Event be_get_event(Backend* be) {
    Event e = wbe_get_keydown(); 

    switch (e) {
        case KD_F3:
            wbe_toggle_scale_factor();
            return IDLE;
        case KD_F10:
            return QUIT;
        default:
            return e; 
    }
}

void be_present(Backend* be) {
    wbe_fill_rect(0, 0, WINDOW_W, WINDOW_H, 1);
}

void be_blit_tile(Backend* be, int x, int y, int n) {
    int sx = n % TILES_PER_ROW * TILE_W;
    int sy = n / TILES_PER_ROW * TILE_H;
    wbe_texture_copy(sx, sy, TILE_W, TILE_H, x, y); 
    return;
}

void be_blit_text(Backend* be, int x, int y, char* str) {
    while (*str) {
        int n = FONT_OFFSET + (int) *str;
        int sx = n % CHARS_PER_ROW * FONT_W;
        int sy = n / CHARS_PER_ROW * FONT_H;
        wbe_texture_copy(sx, sy, FONT_W, FONT_H, x, y); 
        x += FONT_W;
        str++;
    }
}

void be_fill_rect(Backend* be, int x, int y, int w, int h, int color) {
    wbe_fill_rect(x, y, w, h, color);
}

void be_draw_line(Backend* be, int x1, int y1, int x2, int y2, int color) {
    wbe_draw_line(x1, y1, x2, y2, color);
}

void be_send_audiomsg(Backend* be, int msg) {
   wbe_send_audiomsg(msg); 
}

uint64_t be_get_millis(void) {
    return 1;
}

void be_delay(uint32_t dur) {
    return;
}

void be_quit(Backend* be) {
    return;
}