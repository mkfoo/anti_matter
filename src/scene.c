#include "scene.h"
#include "sprite.h"

bool sc_title(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW;
    gs_adv_clock(gs);

    int start_x = 64;
    int y = 64;

    for (int i = 0; i < 8; i++) {
        int x = start_x + i * TILE_W;
        be_blit_tile(be, x, y, 62 + i); 
    }

    if (gs->phase > 0.25) {
        be_blit_text(be, 72, 128, "PUSH SPACE KEY");
    }

    gs_limit_fps(gs);
    be_present(be);

    switch(be_get_event(be)) {
        case KD_SPC:
            gs->level = 0;
            gs->lives = 4;
            gs->score = 0;
            gs_load_level(gs);
            gs_set_scene(gs, sc_start_level);
            break;
        case KD_ESC:
        case QUIT:
            return false;
        default:
            break;
    }
    
    return true;
}

bool sc_start_level(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW / 2;
    gs_adv_clock(gs);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (gs->phase > 0.95) {
        gs_set_scene(gs, sc_playing);
    } 

    return true;
}

bool sc_playing(GameState* gs, Backend* be) {
    uint32_t ticks = gs_adv_clock(gs);
    gs_adv_state(gs, ticks);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);
    gs_post_update(gs);

    switch(be_get_event(be)) {
        case KD_UP:
            gs_move_pcs(gs, 0, -1);
            break;
        case KD_RIGHT:
            gs_move_pcs(gs, 1, 0);
            break;
        case KD_LEFT:
            gs_move_pcs(gs, -1, 0);
            break;
        case KD_DOWN:
            gs_move_pcs(gs, 0, 1);
            break;
        case KD_SPC:
            gs_swap_sprites(gs);
            break;
        case KD_ESC:
            gs_set_scene(gs, sc_paused);
            break;
        case KD_F1:
            gs_set_scene(gs, sc_death1);
            break;
        case QUIT:
            return false;
        default:
            break;
    }

    return true;
}

bool sc_paused(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW;
    gs_adv_clock(gs);
    gs_render_help(gs, be);
    gs_render_default(gs, be);

    switch(be_get_event(be)) {
        case KD_SPC:
        case KD_ESC:
            gs_set_scene(gs, sc_playing);
            break;
        case KD_F1:
        case QUIT:
            return false;
        default:
            break;
    }
    
    return true;
}

bool sc_wait(GameState* gs, Backend* be) {
    gs_adv_clock(gs);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (gs->phase > 0.95) {
        gs_set_scene(gs, sc_playing);
    } 

    return true;
}

bool sc_level_clear(GameState* gs, Backend* be) {
    gs_adv_clock(gs);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);
    gs->energy -= 7;

    if (gs->energy > 0) {
        gs_score(gs, 1);
    } else {
        gs->energy = 0;
        gs->level = (int16_t) (gs->level + 1) % MAX_LEVEL;
        gs_load_level(gs);
        gs_set_scene(gs, sc_start_level);
    }

    return true;
}

bool sc_death1(GameState* gs, Backend* be) {
    gs_adv_clock(gs);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (gs->phase > 0.95) {
        gs->lives -= 1;

        if (gs->lives > 0) {
            gs_load_level(gs);
            gs_set_scene(gs, sc_start_level);
        } else {
            gs_set_scene(gs, sc_game_over);
        }
    }

    return true;
}

bool sc_game_over(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW / 4.0;
    gs_adv_clock(gs);
    be_blit_text(be, 64, 92, "GAME OVER");
    gs_render_default(gs, be);

    if (gs->phase > 0.95) {
        gs_set_scene(gs, sc_title);
    }

    return true;
}
