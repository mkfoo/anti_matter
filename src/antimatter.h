#pragma once

#define ANIM_SPEED 0.0012f
#define BONUS_LIMIT 4800
#define BUF_LEN 1024
#define CHARS_PER_ROW 20
#define DECOR_TILE_BASE 70
#define FONT_H 8
#define FONT_OFFSET 328
#define FONT_W 8
#define FRAME_W 8
#define MAP_H 11
#define MAP_W 11
#define MAX_LEVEL 5
#define MAX_SPRITES 122
#define MAX_LAG 200
#define MAX_X 176
#define MAX_Y 176
#define MOVEMENT_SPEED 0.12f
#define MS_PER_FRAME 20
#define MS_PER_TICK 6
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

#include <stdio.h>

#define LOG_ERR(expr, msg) \
    if (expr) { \
        printf("%s [file %s, line %d]\n", (msg), __FILE__, __LINE__); \
        return NULL; \
    }
