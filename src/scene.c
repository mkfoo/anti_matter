#include "scene.h"

bool sc_title(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW;
    gs_limit_fps(gs);
    be_present(be);

    if (gs->phase > 0.25) {
        be_blit_text(be, 72, 128, "PUSH SPACE KEY");
    }

    switch(be_get_event(be)) {
        case KD_SPC:
            gs->level = 0;
            gs->lives = 2;
            gs->score = 0;
            gs_load_level(gs);
            gs_set_scene(gs, sc_playing);
            break;
        case KD_ESC:
        case QUIT:
            return false;
        default:
            break;
    }
    
    return true;
}

bool sc_playing(GameState* gs, Backend* be) {
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
        case QUIT:
            return false;
        default:
            break;
    }

    return true;
}

bool sc_paused(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW;
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

bool sc_death1(GameState* gs, Backend* be) {
    gs_render_default(gs, be);
    gs->lives -= 1;
    
    if (gs->lives > 0) {
        gs_load_level(gs);
        gs_set_scene(gs, sc_playing);
    } else {
        gs_set_scene(gs, sc_game_over);
    }

    return true;
}

bool sc_game_over(GameState* gs, Backend* be) {
    gs->speed = ANIM_SPEED_SLOW / 4.0;

    if (gs->phase < 0.99) {
        be_blit_text(be, 64, 92, "GAME OVER");
        gs_render_default(gs, be);
    } else {
        gs_set_scene(gs, sc_title);
    }

    return true;
}
