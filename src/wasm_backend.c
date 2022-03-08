#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdint.h>
#include "antimatter.h"
#include "backend.h"
#include "texture_data.h"

#define SPRITE_BUF_SIZE 8192
#define LINE_BUF_SIZE 1600

typedef struct {
    uint8_t* buf;
    int32_t width;
    int32_t height;
} PixelData;

__attribute__((export_name("wbe_load_pixel_data")))
PixelData* wbe_load_pixel_data(void);

__attribute__((import_name("wbe_get_keydown")))
int wbe_get_keydown(void);

__attribute__((import_name("wbe_set_color")))
void wbe_set_color(int color);

__attribute__((import_name("wbe_clear")))
void wbe_clear(void);

__attribute__((import_name("wbe_update_buf")))
void wbe_update_buf(float* ptr, size_t offset, size_t len);

__attribute__((import_name("wbe_draw_buf")))
void wbe_draw_buf(size_t offset, size_t len);

__attribute__((import_name("wbe_draw_lines")))
void wbe_draw_lines(float* ptr, size_t len);

__attribute__((import_name("wbe_toggle_scale_factor")))
void wbe_toggle_scale_factor(void);

__attribute__((import_name("wbe_send_audiomsg")))
void wbe_send_audiomsg(int msg);

static VertexBuf vb_init(size_t cap);
static void vb_push(VertexBuf* self, int x, int y, int z, int w);
static void vb_push_quad(VertexBuf* self, int dx, int dy, int sx, int sy, int w, int h);
static void vb_push_quad_gl(VertexBuf* self, int dx, int dy, int sx, int sy, int w, int h);
static void vb_flush_s(VertexBuf* self, int draw);
static void vb_flush_l(VertexBuf* self);

static VertexBuf vb_init(size_t cap) {
    float* ptr = calloc(cap, sizeof(float));
    if (ptr == NULL) return (VertexBuf) { NULL };
    return (VertexBuf) { .ptr = ptr, .cap = cap };
}

static void vb_push(VertexBuf* self, int x, int y, int z, int w) {
    if (self->len <= self->cap - 4) {
        self->ptr[self->len + 0] = (float) x;
        self->ptr[self->len + 1] = (float) y;
        self->ptr[self->len + 2] = (float) z;
        self->ptr[self->len + 3] = (float) w;
        self->len += 4;
    }
}

static void vb_push_quad(VertexBuf* self, int dl, int dt, int sl, int st, int w, int h) {
    int dr = dl + w;
    int db = dt + h;
    int sr = sl + w;
    int sb = st + h;
    vb_push(self, dl, db, sl, sb);
    vb_push(self, dl, dt, sl, st);
    vb_push(self, dr, db, sr, sb);
    vb_push(self, dr, dt, sr, st);
}

static void vb_flush_s(VertexBuf* self, int draw) {
    if (self->len) {
        wbe_update_buf(self->ptr, self->offset, self->len);

        if (draw) {
            wbe_draw_buf(self->offset, self->len);
        }

        self->len = 0;
    }
}

static void vb_flush_l(VertexBuf* self) {
    if (self->len) {
        wbe_draw_lines(self->ptr, self->len);
        self->len = 0;
    }
}

PixelData* wbe_load_pixel_data(void) {
    PixelData* pd = calloc(1, sizeof(PixelData));
    if (pd == NULL) return NULL; 
    pd->buf = (uint8_t*) TEXTURE_DATA;
    pd->width = TEXTURE_W;
    pd->height = TEXTURE_H; 
    return pd;
}

Backend* be_init(void) {
    Backend* be = calloc(1, sizeof(Backend));
    if (be == NULL) return NULL; 
    be->sprites = vb_init(SPRITE_BUF_SIZE);
    be->lines = vb_init(LINE_BUF_SIZE);
    if (!be->sprites.cap || !be->lines.cap) return NULL;
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

void be_set_color(Backend* be, int color) {
    wbe_set_color(color);
}

void be_set_render_target(Backend* be, int tgt) {
    if (!tgt) {
        size_t len = be->sprites.len;
        vb_flush_s(&be->sprites, 0); 
        be->sprites.offset = len;
    }
}

void be_clear(Backend* be) {
    wbe_clear();
}

void be_present(Backend* be) {
    vb_flush_s(&be->sprites, 1); 
    vb_flush_l(&be->lines); 

    if (be->stat) {
        wbe_draw_buf(0, be->sprites.offset);
        be->stat = 0; 
    }
}

void be_blit_tile(Backend* be, int x, int y, int n) {
    int sx = n % TILES_PER_ROW * TILE_W;
    int sy = n / TILES_PER_ROW * TILE_H;
    vb_push_quad(&be->sprites, x, y, sx, sy, TILE_W, TILE_H);
}

void be_blit_text(Backend* be, int x, int y, char* str) {
    while (*str) {
        int n = FONT_OFFSET + (int) *str;
        int sx = n % CHARS_PER_ROW * FONT_W;
        int sy = n / CHARS_PER_ROW * FONT_H;
        vb_push_quad(&be->sprites, x, y, sx, sy, FONT_W, FONT_H);
        x += FONT_W;
        str++;
    }
}

void be_blit_static(Backend* be) {
    be->stat = 1;
}

void be_draw_line(Backend* be, int x1, int y1, int x2, int y2) {
    vb_push(&be->lines, x1, y1, x2, y2);
}

void be_fill_rect(Backend* be, int dl, int dt, int w, int h) {
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
