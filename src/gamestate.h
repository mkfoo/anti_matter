#pragma once

#include <stdbool.h>
#include "antimatter.h"
#include "backend.h"
#include "sprite.h"

typedef struct GameState GameState;

typedef bool SceneFn(GameState* gs, Backend* be);

struct GameState {
    SceneFn* scene;
    float phase;
    float speed;
    uint32_t prev;
    uint32_t lag;
    int32_t level;
    int32_t high;
    int32_t score;
    int32_t energy;
    int32_t lives;
    uint8_t n_sprites;
    Adjacent adj_a;
    Adjacent adj_m;
    Sprite sprites[MAX_SPRITES];
};

GameState* gs_init(void);

void gs_set_scene(GameState* gs, SceneFn* scene);

bool gs_update(GameState* gs, Backend* be);

void gs_render_default(GameState* gs, Backend* be);

void gs_quit(GameState* gs);
