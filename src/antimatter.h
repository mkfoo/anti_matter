#pragma once

#define ANIM_SPEED 0.0012f
#define BONUS_LIMIT 3600
#define BUF_LEN 1024
#define CHARS_PER_ROW 20
#define CLEAR_BONUS 8
#define DECOR_TILE_BASE 70
#define DESTROY_BONUS 32
#define FONT_H 8
#define FONT_OFFSET 328
#define FONT_W 8
#define FRAME_W 8
#define MAP_H 11
#define MAP_W 11
#define MAX_LEVEL 7
#define MAX_SPRITES 122
#define MAX_LAG 800
#define MAX_X 176
#define MAX_Y 176
#define MOVEMENT_SPEED 0.12f
#define MS_PER_FRAME 20
#define SAMPLE_RATE 44100
#define SAMPLES_PER_TICK 200
#define START_LEVEL 0
#define START_LIVES 4
#define SWAP_COST 400
#define TEXTURE_H 184
#define TEXTURE_W 160
#define TILES_PER_ROW 10
#define TILE_H 16
#define TILE_W 16
#define WALL_TILE_BASE 27
#define WINDOW_H 192
#define WINDOW_TITLE "ANTI/MATTER"
#define WINDOW_W 256

#ifdef WASM_BACKEND

#define LOG_ERR(expr, msg) \
    if (expr) { \
        return NULL; \
    }

#else

#include <stdio.h>

#define LOG_ERR(expr, msg) \
    if (expr) { \
        fprintf(stderr, "%s [file %s, line %d]\n", (msg), __FILE__, __LINE__); \
        return NULL; \
    }

#endif
