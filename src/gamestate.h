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
    int16_t level;
    int32_t high;
    int32_t score;
    int32_t energy;
    int16_t lives;
    uint8_t n_sprites;
    Adjacent adj_a;
    Adjacent adj_m;
    Sprite sprites[MAX_SPRITES];
};

GameState* gs_init(void);

void gs_set_scene(GameState* gs, SceneFn* scene);

void gs_load_level(GameState* gs);

bool gs_update(GameState* gs, Backend* be);

void gs_limit_fps(GameState* gs);

void gs_move_pcs(GameState* gs, int8_t dx, int8_t dy);

void gs_swap_sprites(GameState* gs);

void gs_post_update(GameState* gs);

void gs_render_default(GameState* gs, Backend* be);

void gs_render_sprites(GameState* gs, Backend* be);

void gs_render_help(GameState* gs, Backend* be);

void gs_quit(GameState* gs);
