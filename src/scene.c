#include "scene.h"
#include "sprite.h"
#include "timer.h"

void render_title(Backend* be, int x0, int y);

void render_title(Backend* be, int x0, int y) {
    for (int i = 0; i < 8; i++) {
        int x = x0 + i * TILE_W;
        be_blit_tile(be, x, y, 62 + i); 
    }
}

bool sc_splash(GameState* gs, Backend* be) {
    float phase = t_get_phase(&gs->t);

    if (phase < 0.8f) {
        be_fill_rect(be, 0, 0, WINDOW_W, WINDOW_H, 5); 
    } else if (phase == 1.0f) {
        gs_set_scene(gs, sc_title_anim, 2);
    }

    return true;
}

bool sc_title_anim(GameState* gs, Backend* be) {
    float phase = t_get_phase(&gs->t);
    int x0 = 64;
    int y0 = 64;

    for (int i = 0; i < 8; i++) {
        float x = (float) i * 8.0f * (1.0f - phase);
        float y = (float) i * 16.0f * (1.0f - phase);
        render_title(be, x0 + (int) x, y0 - (int) y);
        render_title(be, x0 - (int) x, y0 + (int) y);
    }

    if (phase == 1.0f) {
        gs_set_scene(gs, sc_title, 0);
    }

    return true;
}

bool sc_title(GameState* gs, Backend* be) {
    render_title(be, 64, 64);

    if (t_get_phase(&gs->t) > 0.25) {
        be_blit_text(be, 72, 128, "PUSH SPACE KEY");
    }

    switch(be_get_event(be)) {
        case KD_SPC:
            gs->level = 0;
            gs->lives = 4;
            gs->score = 0;
            gs_load_level(gs);
            gs_set_scene(gs, sc_start_level, 2);
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
    gs_render_sprites(gs, be);
    float phase = t_get_phase(&gs->t);
    int m = (int) (192.0f / powf(2, phase * 7.0f));

    for (int i = 0; i < WINDOW_H; i++) {
        if (i % m != 0) {
            be_draw_line(be, 0, i, 192, i, 1);
            be_draw_line(be, i, 0, i, 192, 1);
        }
    }

    gs_render_default(gs, be);

    if (phase == 1.0f) {
        gs_set_scene(gs, sc_playing, 0);
    } 

    return true;
}

bool sc_playing(GameState* gs, Backend* be) {
    gs_adv_state(gs);
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
            gs_set_scene(gs, sc_paused, 0);
            break;
        case KD_F1:
            gs_set_scene(gs, sc_death1, 0);
            break;
        case QUIT:
            return false;
        default:
            break;
    }

    return true;
}

bool sc_paused(GameState* gs, Backend* be) {
    gs_render_help(gs, be);
    gs_render_default(gs, be);

    switch(be_get_event(be)) {
        case KD_SPC:
        case KD_ESC:
            gs_set_scene(gs, sc_playing, 0);
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
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (t_get_phase(&gs->t) > 0.95) {
        gs_set_scene(gs, sc_playing, 0);
    } 

    return true;
}

bool sc_level_clear(GameState* gs, Backend* be) {
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);
    gs->energy -= 7;

    if (gs->energy > 0) {
        gs_score(gs, 1);
    } else {
        gs->energy = 0;
        gs->level = (int16_t) (gs->level + 1) % MAX_LEVEL;
        gs_load_level(gs);
        gs_set_scene(gs, sc_start_level, 0);
    }

    return true;
}

bool sc_death1(GameState* gs, Backend* be) {
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (t_get_phase(&gs->t) > 0.95) {
        gs->lives -= 1;

        if (gs->lives > 0) {
            gs_load_level(gs);
            gs_set_scene(gs, sc_start_level, 2);
        } else {
            gs_set_scene(gs, sc_game_over, 5);
        }
    }

    return true;
}

bool sc_game_over(GameState* gs, Backend* be) {
    be_blit_text(be, 64, 92, "GAME OVER");
    gs_render_default(gs, be);

    if (t_get_phase(&gs->t) == 1.0f) {
        gs_set_scene(gs, sc_title_anim, 2);
    }

    return true;
}
