#include "scene.h"
#include "sprite.h"

void render_title(Backend* be, int x0, int y);
void fade_effect(Backend* be, float phase, int color);
void lose_life(GameState* gs);

void render_title(Backend* be, int x0, int y) {
    for (int i = 0; i < 8; i++) {
        int x = x0 + i * TILE_W;
        be_blit_tile(be, x, y, 62 + i);
    }
}

void fade_effect(Backend* be, float phase, int color) {
    int m = (int) (192.0f / powf(2, phase * 7.0f));

    for (int i = 8; i < 184; i++) {
        if (i % m != 0) {
            be_draw_line(be, 8, i, 184, i, color);
            be_draw_line(be, i, 8, i, 184, color);
        }
    }
}

void lose_life(GameState* gs) {
    gs->lives -= 1;

    if (gs->lives > 0) {
        gs_load_level(gs);
        gs_set_scene(gs, sc_start_level, 2);
    } else {
        gs_set_scene(gs, sc_game_over, 5);
    }
}

bool sc_splash(GameState* gs, Backend* be) {
    float phase = gs_phase(gs);
    int x0 = 88;
    int y0 = 80;

    if (phase < 0.8f) {
        be_fill_rect(be, 0, 0, WINDOW_W, WINDOW_H, 4);

        if (phase > 0.4) {
            be_blit_text(be, x0 + 24, y0 - 8, "(Not)");
        }

        be_blit_text(be, x0, y0,  "MSX  system");
        be_blit_text(be, x0, y0 + 8, "version 1.0");
        be_blit_text(be, x0 - 48, y0 + 24, "Copyright 2020 by mkfoo");
    }

    if (phase == 1.0f) {
        gs_set_scene(gs, sc_title_anim, 2);
    }

    return true;
}

bool sc_title_anim(GameState* gs, Backend* be) {
    float phase = gs_phase(gs);
    int x0 = 64;
    int y0 = 88;

    for (int i = 0; i < 8; i++) {
        float x = (float) i * 8.0f * (1.0f - phase);
        float y = (float) i * 16.0f * (1.0f - phase);
        render_title(be, x0 + (int) x, y0 - (int) y);
        render_title(be, x0 - (int) x, y0 + (int) y);
    }

    sg_play(gs->sound, 1);

    if (phase == 1.0f) {
        gs_set_scene(gs, sc_title_move, 1);
    }

    return true;
}

bool sc_title_move(GameState* gs, Backend* be) {
    be_get_event(be);
    float phase = gs_phase(gs);
    int x0 = 64;
    int y0 = 88;
    int y = y0 - phase * 32.0f;

    render_title(be, x0, y);

    if (phase == 1.0f) {
        gs_set_scene(gs, sc_title, 0);
    }

    return true;
}

bool sc_title(GameState* gs, Backend* be) {
    static char high[8];
    snprintf(high, 8, "%7d", gs->high);
    be_blit_text(be, 60, 24, "HIGH-SCORE");
    be_blit_text(be, 60 + 88, 24, high);
    be_blit_text(be, 72, 160, "(c) 2020 mkfoo");
    render_title(be, 64, 56);
    sg_play(gs->sound, 1);

    if (gs_phase(gs) > 0.25) {
        be_blit_text(be, 72, 112, "PUSH SPACE KEY");
    }

    switch(be_get_event(be)) {
        case KD_SPC:
            gs->level = 0;
            gs->lives = 4;
            gs->score = 0;
            gs_load_level(gs);
            gs_set_scene(gs, sc_start_level, 2);
            sg_stop(gs->sound);
            break;
        case KD_F2:
            sg_toggle_mute(gs->sound);
            break;
        case KD_F5:
            sg_change_vol(gs->sound, -1);
            break;
        case KD_F6:
            sg_change_vol(gs->sound, 1);
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
    be_get_event(be);
    sg_play(gs->sound, 2);
    gs_render_sprites(gs, be);
    float phase = gs_phase(gs);
    fade_effect(be, phase, 1);
    gs_render_default(gs, be);

    if (phase == 1.0f) {
        gs_set_scene(gs, sc_playing, 0);
        sg_stop(gs->sound);
        sg_play(gs->sound, 3);
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
            gs_set_scene(gs, sc_swap, 1);
            sg_play(gs->sound, 11);
            gs->sprites[ID_ANTI].tile += 4;
            gs->sprites[ID_MATTER].tile += 4;
            break;
        case KD_ESC:
            gs_set_scene(gs, sc_paused, 0);
            sg_stop(gs->sound);
            sg_play(gs->sound, 4);
            break;
        case KD_F1:
            gs->energy = 0;
            break;
        case KD_F2:
            sg_toggle_mute(gs->sound);
            break;
        case KD_F5:
            sg_change_vol(gs->sound, -1);
            break;
        case KD_F6:
            sg_change_vol(gs->sound, 1);
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
            sg_stop(gs->sound);
            sg_play(gs->sound, 3);
            break;
        case KD_F1:
            gs->energy = 0;
            gs_set_scene(gs, sc_playing, 0);
            break;
        case QUIT:
            return false;
        default:
            break;
    }

    return true;
}

bool sc_wait(GameState* gs, Backend* be) {
    be_get_event(be);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (gs_phase(gs) == 1.0f) {
        gs_set_scene(gs, sc_playing, 0);
    } 

    return true;
}

bool sc_swap(GameState* gs, Backend* be) {
    be_get_event(be);
    static bool swapped = false;
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (gs_phase(gs) >= 0.5f && !swapped) {
        gs_swap_sprites(gs);
        gs->energy -= 160;
        swapped = true;
    } else if (gs_phase(gs) == 1.0f) {
        gs->sprites[ID_ANTI].tile -= 4;
        gs->sprites[ID_MATTER].tile -= 4;
        gs_set_scene(gs, sc_playing, 0);
        swapped = false;
    } 

    return true;
}

bool sc_level_clear(GameState* gs, Backend* be) {
    be_get_event(be);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);
    sg_play(gs->sound, 10);
    gs->energy -= 8;

    if (gs->energy > 0) {
        gs_score(gs, 1);
    } else {
        gs->energy = 0;
        gs->level = (int16_t) (gs->level + 1) % MAX_LEVEL;
        gs_load_level(gs);
        gs_set_scene(gs, sc_start_level, 2);
    }

    return true;
}

bool sc_death1(GameState* gs, Backend* be) {
    be_get_event(be);
    gs_render_sprites(gs, be);
    gs_render_default(gs, be);

    if (gs_phase(gs) == 1.0f) {
        lose_life(gs);
    }

    return true;
}

bool sc_death2(GameState* gs, Backend* be) {
    be_get_event(be);
    float phase = gs_phase(gs);
    int color = 15;

    if (((int) (phase * 41.0f)) % 2 == 0) {
        color = 14;
    }

    be_fill_rect(be, 0, 0, WINDOW_W, WINDOW_H, color); 
    gs_render_sprites(gs, be);
    fade_effect(be, (1.0f - phase), color);
    gs_render_default(gs, be);

    if (phase == 1.0f) {
        lose_life(gs);
    }

    return true;
}

bool sc_game_over(GameState* gs, Backend* be) {
    be_get_event(be);
    be_blit_text(be, 64, 92, "GAME OVER");
    gs_render_default(gs, be);
    sg_play(gs->sound, 8);

    if (gs_phase(gs) == 1.0f) {
        sg_stop(gs->sound);
        gs_set_scene(gs, sc_title_anim, 2);
    }

    return true;
}
