#include <math.h>
#include <assert.h>
#include "gamestate.h"
#include "level_data.h"
#include "scene.h"

uint32_t get_lag(GameState* gs);

void advance(GameState* gs);

void adv_phase(GameState* gs, float speed, float ticks); 

void adv_movement(GameState* gs, uint32_t ticks);

void add_sprite(GameState* gs, int16_t x, int16_t y, uint8_t id);

void set_sprite_pos(GameState* gs, int16_t x, int16_t y, uint8_t id);

void add_wall(GameState* gs, int16_t x, int16_t y, uint8_t tile);

void check_overlap(GameState* gs, Sprite* s1, Sprite* s2);

void render_bg(Backend* be);

void decorate(Backend* be);

void render_stats(GameState* gs, Backend* be);

void remove_destroyed(GameState* gs);

Adjacent find_adjacent(GameState* gs, Sprite* s, Delta d);

GameState* gs_init(void) {
    GameState* gs = calloc(1, sizeof(GameState));

    if (gs == NULL) { 
        return NULL;
    }

    gs_set_scene(gs, sc_title); 
    gs->prev = be_get_millis();
    gs->lag = 0;
    gs->level = 0;
    gs->high = 10000;
    gs->score = 0;
    gs->energy = 1000;
    gs->lives = 2; 
    add_sprite(gs, 0, 0, ID_NIL);

    return gs;
}

void gs_set_scene(GameState* gs, SceneFn* scene) {
    gs->phase = 0.0;
    gs->speed = (float) ANIM_SPEED_DEFAULT;
    gs->scene = scene;
}

void gs_load_level(GameState* gs) {
    gs->n_sprites = 1;
    Sprite* nil = &gs->sprites[ID_NIL];
    gs->adj_a = (Adjacent) { nil, nil, nil };
    gs->adj_m = (Adjacent) { nil, nil, nil };
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
                set_sprite_pos(gs, x, y, ID_ANTI);
                break;
            case ID_MATTER:
                set_sprite_pos(gs, x, y, ID_MATTER);
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

bool gs_update(GameState* gs, Backend* be) {
    advance(gs);
    SceneFn* scene = gs->scene;
    return scene(gs, be);
}

void gs_post_update(GameState* gs) {
    remove_destroyed(gs);

    Sprite* anti = &gs->sprites[ID_ANTI];
    Sprite* matter = &gs->sprites[ID_MATTER];

    if (has_flag(anti, F_NIL) || has_flag(matter, F_NIL)) { 
        gs_set_scene(gs, sc_death1);
        return;
    }

    check_overlap(gs, anti, matter);
    check_overlap(gs, gs->adj_a.front, gs->adj_m.front);
    check_overlap(gs, gs->adj_a.front, gs->adj_a.next);
    check_overlap(gs, gs->adj_m.front, gs->adj_m.next);

    if (is_aligned(anti) && is_aligned(matter)) {
        Delta d = get_delta(anti, matter);

        if (d.x == 0 || d.y == 0) {
            for (int i = 3; i < gs->n_sprites; i++) {
                Sprite* s = &gs->sprites[i];

                if (point_between(s->p, anti->p, matter->p)) {
                    return;
                }
            }

            anti->d = d;
            matter->d = invert_delta(d);
        }
    }
}

void gs_swap_sprites(GameState* gs) {
    Sprite* s1 = &gs->sprites[ID_ANTI];
    Sprite* s2 = &gs->sprites[ID_MATTER];
    Point p1 = s1->p; 
    s1->p = s2->p; 
    s2->p = p1; 
}

void gs_move_pcs(GameState* gs, int8_t dx, int8_t dy) {
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
        }
    }
}

void gs_render_default(GameState* gs, Backend* be) {
    render_stats(gs, be);
    decorate(be);
    gs_limit_fps(gs);
    be_present(be);
}

void gs_render_sprites(GameState* gs, Backend* be) {
    int16_t bound_x = MAX_X - TILE_W;
    int16_t bound_y = MAX_Y - TILE_H;
    int16_t fw = FRAME_W;

    render_bg(be);

    for (int i = 1; i < gs->n_sprites; i++) {
        Sprite* s = &gs->sprites[i];

        if (!has_flag(s, F_NIL)) {
            int16_t x = s->p.x;
            int16_t y = s->p.y;
            uint8_t tile = s->tile;
            uint8_t offset = gs->phase * 4.0;

            if (has_flag(s, F_ANIMATED)) {
                tile += offset;
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

void gs_limit_fps(GameState* gs) {
    uint32_t next = gs->prev + MS_PER_FRAME;
    uint32_t now = be_get_millis();

    if (next > now) {
        be_delay(next - now);
    }
}

void gs_quit(GameState* gs) {
    free(gs);
}

uint32_t get_lag(GameState* gs) {
    uint32_t curr = be_get_millis();
    uint32_t lag = curr - gs->prev + gs->lag;
    gs->prev = curr;
    return lag;
}

void advance(GameState* gs) {
    uint32_t lag = get_lag(gs);
    uint32_t ticks = lag / MS_PER_TICK;
    adv_phase(gs, gs->speed, (float) ticks);
    adv_movement(gs, ticks / MOVEMENT_SPEED);
    gs->lag = lag % MS_PER_TICK;
}

void adv_phase(GameState* gs, float speed, float ticks) {
    gs->phase = fmodf(gs->phase + speed * ticks, 1.0);
}

void adv_movement(GameState* gs, uint32_t ticks) {
    for (uint32_t t = 0; t < ticks; t++) {
        for (uint8_t i = 1; i < gs->n_sprites; i++) {
            Sprite* s = &gs->sprites[i];

            if (is_moving(s)) { 
                update_sprite(s);
            }
        }
    }
}

void add_sprite(GameState* gs, int16_t x, int16_t y, uint8_t id) {
    assert(gs->n_sprites < MAX_SPRITES);
    Sprite* s = &gs->sprites[gs->n_sprites];
    *s = PROTOTYPES[id];
    s->p = (Point) { x, y }; 
    gs->n_sprites++;
}

void set_sprite_pos(GameState* gs, int16_t x, int16_t y, uint8_t id) {
    assert(id < gs->n_sprites);
    Sprite* s = &gs->sprites[id];
    s->p = (Point) { x, y }; 
}

void add_wall(GameState* gs, int16_t x, int16_t y, uint8_t tile) {
    assert(gs->n_sprites < MAX_SPRITES);
    Sprite* s = &gs->sprites[gs->n_sprites];
    s->p = (Point) { x, y }; 
    s->flags = 0; 
    s->tile = tile; 
    gs->n_sprites++;
}

void check_overlap(GameState* gs, Sprite* s1, Sprite* s2) {
    if (is_overlapping(s1, s2)) {
        destroy_sprite(s1);
        destroy_sprite(s2);
        gs_set_scene(gs, sc_wait);
    }
}

void remove_destroyed(GameState* gs) {
    for (int i = 3; i < gs->n_sprites; i++) {
        Sprite* s = &gs->sprites[i];
        
        if (has_flag(s, F_DESTROY)) {
            destroy_sprite(s);
        }
    }
}


Adjacent find_adjacent(GameState* gs, Sprite* s1, Delta d) {
    Point front = calc_tile(s1->p, d);
    Point back = calc_tile(s1->p, invert_delta(d));
    Point next = calc_tile(front, d);
    Sprite* nil = &gs->sprites[ID_NIL];
    Adjacent adj = { nil, nil, nil };

    for (int i = 1; i < gs->n_sprites; i++) {
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

void render_bg(Backend* be) {
    for (int i = 0; i < MAP_W * MAP_H; i++) {
        int16_t x = i % MAP_W * TILE_W + FRAME_W;
        int16_t y = i / MAP_H * TILE_H + FRAME_W;
        be_blit_tile(be, x, y, 10);
    }
}

void decorate(Backend* be) {
    uint8_t t = DECOR_TILE_BASE;
    be_blit_tile(be, 0, 0, t);
    be_blit_tile(be, 176, 0, t + 1);
    be_blit_tile(be, 176, 176, t + 2);
    be_blit_tile(be, 0, 176, t + 3);

    for (int n = 16; n < 176; n += 16) {
        be_blit_tile(be, n, 0, t + 4);
        be_blit_tile(be, n, 176, t + 6);
        be_blit_tile(be, 0, n, t + 7);
        be_blit_tile(be, 176, n, t + 5);
    }    
 
    for (int x = 192; x < 240; x += 16) {
        for (int y = 27; y < 183; y += 26) {
            be_blit_tile(be, x, y, t + 4);
        }
    }
    
    for (int y = 27; y < 183; y += 26) {
        be_blit_tile(be, 176, y, t + 8);
        be_blit_tile(be, 240, y, t + 9);
    }

    be_blit_tile(be, 200, 5, 52);
    be_blit_tile(be, 216, 5, 53);
    be_blit_tile(be, 232, 5, 54);
    be_fill_rect(be, 243, 17, 5, 5, 1); 

    be_blit_tile(be, 188, 171, 55);
    be_blit_tile(be, 188 + 16, 171, 56);
    be_blit_tile(be, 188 + 32, 171, 57);
    be_blit_tile(be, 188 + 48, 171, 58);
}

void render_stats(GameState* gs, Backend* be) {
    static char level[8], high[8], score[8], energy[8], lives[8];

    be_fill_rect(be, 184, 0, 80, 192, 1); 
    
    snprintf(level, 8, "%7d", gs->level);
    snprintf(high, 8, "%7d", gs->high);
    snprintf(score, 8, "%7d", gs->score);
    snprintf(energy, 8, "%7d", gs->energy);
    snprintf(lives, 8, "%7d", gs->lives);

    be_blit_text(be, 196, 36, "LEVEL");
    be_blit_text(be, 196, 45, level); 
    be_blit_text(be, 196, 62, "HIGH");
    be_blit_text(be, 196, 71, high); 
    be_blit_text(be, 196, 88, "SCORE"); 
    be_blit_text(be, 196, 97, score); 
    be_blit_text(be, 196, 114, "ENERGY"); 
    be_blit_text(be, 196, 123, energy); 
    be_blit_text(be, 196, 140, "LIVES"); 
    be_blit_text(be, 196, 149, lives); 
}

void gs_render_help(GameState* gs, Backend* be) {
    int x = 42;
    int y = 28;
    int m = 16;
    
    if (gs->phase > 0.25) {
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
