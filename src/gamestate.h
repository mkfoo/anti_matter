#pragma once

#include <stdbool.h>
#include "antimatter.h"
#include "backend.h"
#include "sprite.h"

typedef enum {
    S_PLAYING,
    S_PAUSED,
    S_GAME_OVER,
} Scene;

typedef struct {
    Scene scene;
    float phase;
    uint32_t prev;
    uint32_t lag;
    int32_t level;
    int32_t high;
    int32_t score;
    int32_t energy;
    int32_t lives;
    uint8_t n_sprites;
    Sprite sprites[MAX_SPRITES];
} GameState;

GameState* gs_init(void);

bool gs_update(GameState* gs, Backend* be);

void gs_quit(GameState* gs);
