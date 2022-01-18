#include <stddef.h>
#include <stdlib.h>
#include "sprite.h"

static bool should_stop(Sprite* self);
static bool num_between(int16_t self, int16_t a, int16_t b);
static bool opp_polarity(Sprite* self, Sprite* other);
static bool is_identical(Sprite* self, Sprite* other);

Delta get_delta(Sprite* self, Sprite* other) {
    int16_t x = other->p.x - self->p.x; 
    int16_t y = other->p.y - self->p.y; 
    int8_t dx = 0;
    int8_t dy = 0;

    if (x != 0) {
        dx = (int8_t) (x / abs(x));
    }

    if (y != 0) {
        dy = (int8_t) (y / abs(y));
    }

    return (Delta) { dx, dy };
}

Delta invert_delta(Delta d) {
    return (Delta) { -d.x, -d.y };
}

Point calc_point(Point p, Delta d) {
    return (Point) {
        (int16_t) (MAX_X + (p.x + d.x) % MAX_X) % MAX_X,
        (int16_t) (MAX_Y + (p.y + d.y) % MAX_Y) % MAX_Y,
    };
}

Point calc_tile(Point p, Delta d1) {
   Delta d2 = { d1.x * TILE_W, d1.y * TILE_H }; 
   return calc_point(p, d2);
}

bool has_flag(Sprite* self, Flag flag) {
    return (self->flags & flag) == flag;
}

bool is_moving(Sprite* self) {
    return self->d.x != 0 || self->d.y != 0;
}

bool is_aligned(Sprite* self) {
    return self->p.x % TILE_W == 0 && self->p.y % TILE_H == 0;
}

bool point_equals(Point p1, Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

bool is_overlapping(Sprite* self, Sprite* other) {
    return self != other && 
        ((!has_flag(self, F_NIL) && 
            !has_flag(other, F_NIL)) && 
                point_equals(self->p, other->p));
}

bool point_between(Point self, Point p1, Point p2) {
    if (self.x == p1.x && p1.x == p2.x) {
        return num_between(self.y, p1.y, p2.y);
    } 

    if (self.y == p1.y && p1.y == p2.y) {
        return num_between(self.x, p1.x, p2.x);
    } 
        
    return false;
}

bool can_move(Adjacent* a) {
    return has_flag(a->front, F_NIL) || 
        (has_flag(a->front, F_MOVABLE) && 
            (has_flag(a->next, F_NIL) || 
                (has_flag(a->next, F_MOVABLE) && 
                    opp_polarity(a->front, a->next))));
}

bool can_move_both(Adjacent* a, Adjacent* b) {
    return (can_move(a) && can_move(b)) &&
            !is_identical(a->front, b->next) && 
                (!point_equals(a->next_p, b->next_p) || 
                    (opp_polarity(a->front, b->front) ||
                        (has_flag(a->front, F_NIL) ||
                            has_flag(b->front, F_NIL))));
}

bool can_pull(Sprite* self, Sprite* other) {
    return has_flag(other, F_MOVABLE) && opp_polarity(self, other);
}

void move_sprite(Sprite* self, Adjacent* a, Delta d) {
    self->d = d;
    push_sprite(a->front, d);

    if (can_pull(self, a->back)) {
        push_sprite(a->back, d);
    }
}

void push_sprite(Sprite* self, Delta d) {
    if (!has_flag(self, F_NIL) && !has_flag(self, F_PLAYER_CHAR)) {
        self->d = d;
    }
}

void update_sprite(Sprite* self) {
    self->p = calc_point(self->p, self->d); 

    if (should_stop(self)) {
        self->d = (Delta) { 0, 0 };
    } 
}

void destroy_sprite(Sprite* self) {
    if (!has_flag(self, F_DESTROY)) {
        self->flags |= F_DESTROY;
        self->d = (Delta) { 0, 0 };
        self->tile += 4;
    } else {
        *self = (Sprite) { { 0, 0 }, { 0, 0 }, F_NIL, 0 };
    }
}

static bool should_stop(Sprite* self) {
    return (self->d.x != 0 && self->p.x % TILE_W == 0)
        || (self->d.y != 0 && self->p.y % TILE_H == 0);
}

static bool num_between(int16_t self, int16_t a, int16_t b) {
    if (a < b) {
        return self > a && self < b;
    } 

    if (b < a) {
        return self > b && self < a;
    }

    return false;
}

static bool opp_polarity(Sprite* self, Sprite* other) {
    return has_flag(self, F_POLARITY) ^ has_flag(other, F_POLARITY);
}

static bool is_identical(Sprite* self, Sprite* other) {
    return self == other && !has_flag(self, F_NIL);
}
