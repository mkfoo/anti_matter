#include <math.h>
#include <assert.h>
#include <stdio.h>
#include "gamestate.h"
#include "level_data.h"
#include "scene.h"

static bool advance_clock(GameState* self, double timestamp);
static void add_sprite(GameState* gs, int16_t x, int16_t y, uint8_t id);
static void set_sprite_pos(GameState* gs, int16_t x, int16_t y, uint8_t id);
static void add_wall(GameState* gs, int16_t x, int16_t y, uint8_t tile);
static bool check_overlap(GameState* gs, Backend* be, Sprite* s1, Sprite* s2);
static bool check_los(GameState* gs, Sprite* s1, Sprite* s2);
static void render_stats(GameState* gs, Backend* be);
static void remove_destroyed(GameState* gs);
static Adjacent find_adjacent(GameState* gs, Sprite* s, Delta d);

GameState* gs_init(double start_t) {
    GameState* gs = calloc(1, sizeof(GameState));
    LOG_ERR(gs == NULL, "alloc failure")
    gs->prev = (int64_t) start_t;
    gs->spd_mod = -8.0f;
    gs->high = 1000;
    gs->los = false;
    add_sprite(gs, -1, -1, ID_NIL);
    gs_set_scene(gs, sc_splash, 5); 
    return gs;
}

float gs_phase(GameState* self) {
    if (self->delay == 0) {
        return self->phase;
    }

    if (self->prev < (self->start + self->delay)) {
        return (float) (self->prev - self->start) / (float) self->delay;
    }

    return 1.0f;
}

bool gs_update(GameState* gs, Backend* be, double timestamp) {
    if (advance_clock(gs, timestamp)) {
        be_clear(be);
        SceneFn* scene = gs->scene;
        bool retval = scene(gs, be);
        be_present(be);
        return retval;
    }

    return true;
}

void gs_limit_fps(GameState* self) {
    int64_t next = self->prev + MS_PER_FRAME;
    int64_t now = (int64_t) be_get_millis();

    if (next > now) {
        be_delay(next - now);
    }
}

void gs_set_scene(GameState* gs, SceneFn* scene, uint32_t delay) {
    gs->start = gs->prev;
    gs->delay = delay * 1000;
    gs->scene = scene;
}

void gs_load_level(GameState* gs) {
    gs->n_sprites = 1;
    gs->to_clear = 0;
    gs->los = false;
    gs->energy = LEVEL_ENERGY[gs->level];
    Sprite* nil = &gs->sprites[ID_NIL];
    gs->adj_a = (Adjacent) { nil, nil, nil, { 0, 0 } };
    gs->adj_m = (Adjacent) { nil, nil, nil, { -1, -1 } };
    add_sprite(gs, 160, 160, ID_ANTI);
    add_sprite(gs, 0, 0, ID_MATTER);

    int16_t map_len = MAP_W * MAP_H;
    int16_t start = map_len * gs->level;

    for (int16_t i = 0; i < map_len; i++) {
        int16_t x = i % MAP_W * TILE_W;
        int16_t y = i / MAP_H * TILE_H;
        uint8_t id = LEVEL_DATA[start + i];

        switch (id) {
            case ID_NIL:
                continue;
            case ID_ANTI:
            case ID_MATTER:
                set_sprite_pos(gs, x, y, id);
                break;
            case ID_BLOB_B:
            case ID_BLOB_R:
                gs->to_clear++;
                add_sprite(gs, x, y, id);
                break;
            default:
                if (id < WALL_TILE_BASE) {
                    add_sprite(gs, x, y, id);
                } else {
                    add_wall(gs, x, y, id);
                }
        }
    }
}

void gs_adv_state(GameState* gs) {
    uint32_t moves = gs->lag * MOVEMENT_SPEED;
    Sprite* anti = &gs->sprites[ID_ANTI];
    Sprite* matter = &gs->sprites[ID_MATTER];

    for (size_t t = 0; t < moves; t++) {
        if (is_moving(anti)) {
            update_sprite(anti);
            update_sprite(matter);
            gs->energy--;
        }

        for (size_t i = 3; i < gs->n_sprites; i++) {
            Sprite* s = &gs->sprites[i];

            if (is_moving(s)) {
                update_sprite(s);
            }
        }

        bool prev_los = gs->los;
        gs->los = check_los(gs, anti, matter);

        if (!prev_los && gs->los)
            return;

        if (is_overlapping(anti, matter))
            return; 
    }

}

void gs_post_update(GameState* gs, Backend* be) {
    remove_destroyed(gs);

    Sprite* anti = &gs->sprites[ID_ANTI];
    Sprite* matter = &gs->sprites[ID_MATTER];

    if (gs->energy < 1) { 
        destroy_sprite(anti);
        destroy_sprite(matter);
        gs_set_scene(gs, sc_death1, 2);
        be_send_audiomsg(be, MSG_STOP);
        be_send_audiomsg(be, MSG_PLAY | 6);
        return;
    } 

    bool lose = false;

    lose |= check_overlap(gs, be, gs->adj_a.front, gs->adj_m.front);
    lose |= check_overlap(gs, be, gs->adj_a.front, gs->adj_a.next);
    lose |= check_overlap(gs, be, gs->adj_m.front, gs->adj_m.next);
    lose |= gs->los;

    if (gs->to_clear <= 0 && !lose) {
        gs_set_scene(gs, sc_level_clear, 0);
        be_send_audiomsg(be, MSG_STOP);
        be_send_audiomsg(be, MSG_REPEAT | MSG_PLAY | 10);
        return;
    }

    check_overlap(gs, be, anti, matter);
}

void gs_swap_sprites(GameState* gs) {
    Sprite* s1 = &gs->sprites[ID_ANTI];
    Sprite* s2 = &gs->sprites[ID_MATTER];
    Point p1 = s1->p; 
    s1->p = s2->p; 
    s2->p = p1; 
}

void gs_move_pcs(GameState* gs, Backend* be, int8_t dx, int8_t dy) {
    Sprite* anti = &gs->sprites[ID_ANTI];
    Sprite* matter = &gs->sprites[ID_MATTER];
    Delta forward = { dx, dy };
    Delta backward = invert_delta(forward);

    if (!is_moving(anti) && !is_moving(matter)) {
        gs->adj_a = find_adjacent(gs, anti, backward);
        gs->adj_m = find_adjacent(gs, matter, forward);

        if (can_move_both(&gs->adj_a, &gs->adj_m)) {
            move_sprite(anti, &gs->adj_a, backward);
            move_sprite(matter, &gs->adj_m, forward);
            be_send_audiomsg(be, MSG_PLAY | 5);
        }
    }
}

void gs_render_default(GameState* gs, Backend* be) {
    be_fill_rect(be, 184, 0, 72, 192); 
    be_blit_static(be);
    render_stats(gs, be);
}

void gs_render_help(GameState* gs, Backend* be) {
    int x = 42;
    int y = 28;
    int m = 16;

    if (gs->phase > 0.25f) {
        be_blit_text(be, x + 31, y, "PAUSED");
    }

    be_blit_text(be, x, y + m * 1, "ESC     RESUME"); 
    be_blit_text(be, x, y + m * 2, "F1     RESTART");
    be_blit_text(be, x, y + m * 3, "F2        MUTE"); 
    be_blit_text(be, x, y + m * 4, "F3       SCALE"); 
    be_blit_text(be, x, y + m * 5, "F4     FULLSCR"); 
    be_blit_text(be, x, y + m * 6, "F5       VOL -"); 
    be_blit_text(be, x, y + m * 7, "F6       VOL +"); 
    be_blit_text(be, x, y + m * 8, "F10       QUIT"); 
}

void gs_render_sprites(GameState* gs, Backend* be) {
    int16_t bound_x = MAX_X - TILE_W;
    int16_t bound_y = MAX_Y - TILE_H;
    int16_t fw = FRAME_W;

    for (size_t i = 1; i < gs->n_sprites; i++) {
        Sprite* s = &gs->sprites[i];

        if (!has_flag(s, F_NIL)) {
            int16_t x = s->p.x;
            int16_t y = s->p.y;
            int tile = s->tile;
            float offset = gs->phase * gs->spd_mod;

            if (has_flag(s, F_ANIMATED)) {
                tile += (4 - (int) offset % 4) % 4;
            } 

            if (x > bound_x) {
                int16_t x2 = x - MAX_X; 
                be_blit_tile(be, x2 + fw, y + fw, tile);
            } else if (y > bound_y) {
                int16_t y2 = y - MAX_Y; 
                be_blit_tile(be, x + fw, y2 + fw, tile);
            }

            be_blit_tile(be, x + fw, y + fw, tile);
        }
    }
}

void gs_quit(GameState* gs) {
    free(gs);
}

void gs_score(GameState* gs, int32_t num) {
    gs->score += num;

    if (gs->score > gs->high) {
        gs->high = gs->score;
    }
}

static bool advance_clock(GameState* self, double timestamp) {
    int64_t now = (int64_t) timestamp;
    int64_t lag = now - self->prev;
    self->prev = now;

    if (lag > 0 && lag < MAX_LAG) {
        float incr = ANIM_SPEED * (float) lag;
        self->phase = fmodf(self->phase + incr, 1.0f);
        self->lag = lag; 
        return true;
    }

    return false;
}

static void add_sprite(GameState* gs, int16_t x, int16_t y, uint8_t id) {
    assert(gs->n_sprites < MAX_SPRITES);
    Sprite* s = &gs->sprites[gs->n_sprites];
    *s = PROTOTYPES[id];
    s->p = (Point) { x, y }; 
    gs->n_sprites++;
}

static void set_sprite_pos(GameState* gs, int16_t x, int16_t y, uint8_t id) {
    assert(id < gs->n_sprites);
    Sprite* s = &gs->sprites[id];
    s->p = (Point) { x, y }; 
}

static void add_wall(GameState* gs, int16_t x, int16_t y, uint8_t tile) {
    assert(gs->n_sprites < MAX_SPRITES);
    Sprite* s = &gs->sprites[gs->n_sprites];
    s->p = (Point) { x, y }; 
    s->flags = 0; 
    s->tile = tile; 
    gs->n_sprites++;
}

static bool check_overlap(GameState* gs, Backend* be, Sprite* s1, Sprite* s2) {
    if (is_overlapping(s1, s2)) {
        if (has_flag(s1, F_UNSTABLE) || has_flag(s2, F_UNSTABLE)) {
            s1->tile = 41;
            s2->tile = 41;
            gs_set_scene(gs, sc_death2, 2);
            be_send_audiomsg(be, MSG_STOP);
            be_send_audiomsg(be, MSG_PLAY | 7);
            return true;
        } else {
            gs_set_scene(gs, sc_wait, 1);
            destroy_sprite(s1);
            destroy_sprite(s2);
            be_send_audiomsg(be, MSG_PLAY | 9);
        }
    }

    return false;
}

static bool check_los(GameState* gs, Sprite* s1, Sprite* s2) {
    if (s1->p.x > 160 || s1->p.y > 160)
        return false;

    Delta d = get_delta(s1, s2);

    if (d.x == 0 || d.y == 0) {
        for (size_t i = 3; i < gs->n_sprites; i++) {
            Sprite* s0 = &gs->sprites[i];

            if (point_between(s0->p, s1->p, s2->p)) {
                return false;
            }
        }

        s1->d = d;
        s2->d = invert_delta(d);
        return true;
    }

    return false;
}

static void remove_destroyed(GameState* gs) {
    for (size_t i = 3; i < gs->n_sprites; i++) {
        Sprite* s = &gs->sprites[i];

        if (has_flag(s, F_DESTROY)) {
            gs->to_clear--;
            gs_score(gs, DESTROY_BONUS);
            destroy_sprite(s);
        }
    }
}

static Adjacent find_adjacent(GameState* gs, Sprite* s1, Delta d) {
    Point front = calc_tile(s1->p, d);
    Point back = calc_tile(s1->p, invert_delta(d));
    Point next = calc_tile(front, d);
    Sprite* nil = &gs->sprites[ID_NIL];
    Adjacent adj = { nil, nil, nil, next };

    for (size_t i = 1; i < gs->n_sprites; i++) {
        Sprite* s2 = &gs->sprites[i];

        if (point_equals(s2->p, front)) {
            adj.front = s2;
        } else if (point_equals(s2->p, back)) {
            adj.back = s2;
        } else if (point_equals(s2->p, next)) {
            adj.next = s2;
        }
    }

    return adj;
}

void gs_decorate(Backend* be) {
    uint8_t t = DECOR_TILE_BASE;

    be_blit_tile(be, 0, 0, t);
    for (int n = 16; n < 176; n += 16) {
        be_blit_tile(be, n, 0, t + 4);
    }
    be_blit_tile(be, 176, 0, t + 1);

    for (int n = 16; n < 176; n += 16) {
        be_blit_tile(be, 0, n, t + 7);
    }     
    be_blit_tile(be, 0, 176, t + 3);

    for (int n = 16; n < 176; n += 16) {
        be_blit_tile(be, n, 176, t + 6);
    }
    be_blit_tile(be, 176, 176, t + 2);

    for (int n = 16; n < 176; n += 16) {
        be_blit_tile(be, 176, n, t + 5);
    }
    for (int y = 27; y < 183; y += 26) {
        be_blit_tile(be, 176, y, t + 8);

        for (int x = 192; x < 240; x += 16) {
            be_blit_tile(be, x, y, t + 4);
        }

        be_blit_tile(be, 240, y, t + 9);
    }

    be_blit_tile(be, 204, 5, 80);
    be_blit_tile(be, 220, 5, 81);
    be_blit_tile(be, 236, 5, 82);

    be_blit_tile(be, 197, 170, 83);
    be_blit_tile(be, 197 + 16, 170, 84);
    be_blit_tile(be, 197 + 32, 170, 85);
    be_blit_tile(be, 197 + 48, 170, 86);

    be_blit_text(be, 196, 36, "LEVEL");
    be_blit_text(be, 196, 62, "HIGH");
    be_blit_text(be, 196, 88, "SCORE"); 
    be_blit_text(be, 196, 114, "ENERGY"); 
    be_blit_text(be, 196, 140, "LIVES"); 
}

static void render_stats(GameState* gs, Backend* be) {
    static char level[8], high[8], score[8], energy[8], lives[8];
    snprintf(level, 8, "%7d", gs->level);
    snprintf(high, 8, "%7d", gs->high);
    snprintf(score, 8, "%7d", gs->score);
    snprintf(energy, 8, "%7d", gs->energy);
    snprintf(lives, 8, "%7d", gs->lives);
    be_blit_text(be, 196, 45, level); 
    be_blit_text(be, 196, 71, high); 
    be_blit_text(be, 196, 97, score); 
    be_blit_text(be, 196, 123, energy); 
    be_blit_text(be, 196, 149, lives); 
}

