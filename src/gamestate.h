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
    float spd_mod;
    int64_t start;
    int64_t delay;
    int64_t prev;
    int64_t lag;
    int32_t level;
    int32_t high;
    int32_t score;
    int32_t energy;
    int32_t lives;
    int32_t to_clear;
    uint32_t n_sprites;
    Adjacent adj_a;
    Adjacent adj_m;
    Sprite sprites[MAX_SPRITES];
};

GameState* gs_init(double start_t);
float gs_phase(GameState* gs);
bool gs_update(GameState* gs, Backend* be, double timestamp);
void gs_limit_fps(GameState* self);
void gs_set_scene(GameState* gs, SceneFn* scene, uint32_t delay);
void gs_load_level(GameState* gs);
void gs_adv_state(GameState* gs);
void gs_move_pcs(GameState* gs, Backend* be, int8_t dx, int8_t dy);
void gs_swap_sprites(GameState* gs);
void gs_post_update(GameState* gs, Backend* be);
void gs_score(GameState* gs, int32_t n);
void gs_render_default(GameState* gs, Backend* be);
void gs_render_sprites(GameState* gs, Backend* be);
void gs_render_help(GameState* gs, Backend* be);
void gs_decorate(Backend* be);
void gs_quit(GameState* gs);
