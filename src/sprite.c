#include <stddef.h>
#include <stdlib.h>
#include "sprite.h"

bool should_stop(Sprite* self);
bool num_between(int16_t self, int16_t a, int16_t b);

Delta get_delta(Sprite* self, Sprite* other) {
    int16_t x = other->p.x - self->p.x; 
    int16_t y = other->p.y - self->p.y; 
    int16_t dx = 0;
    int16_t dy = 0;

    if (x != 0) {
        dx = x / abs(x);
    }

    if (y != 0) {
        dy = y / abs(y);
    }

    return (Delta) { dx, dy };
}

Delta invert_delta(Delta d) {
    return (Delta) { -d.x, -d.y };
}

Point calc_point(Point p, Delta d) {
    return (Point) {
        (MAX_X + (p.x + d.x) % MAX_X) % MAX_X,
        (MAX_Y + (p.y + d.y) % MAX_Y) % MAX_Y,
    };
}

Point calc_tile(Point p, Delta d1) {
   Delta d2 = { d1.x * TILE_W, d1.y * TILE_H }; 
   return calc_point(p, d2);
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

bool num_between(int16_t self, int16_t a, int16_t b) {
    if (a < b) {
        return self > a && self < b;
    } else if (b < a) {
        return self > b && self < a;
    }

    return false;
}

bool point_between(Point self, Point p1, Point p2) {
    if (self.x == p1.x && p1.x == p2.x) {
        return num_between(self.y, p1.y, p2.y);
    } else if (self.y == p1.y && p1.y == p2.y) {
        return num_between(self.x, p1.x, p2.x);
    } else {
        return false;
    }
}

bool can_move(Adjacent* a) {
    return a->front == NULL 
        || (a->front->flags & F_MOVABLE && a->next == NULL);
}

bool can_pull(Sprite* self, Sprite* other) {
    return other != NULL 
        && (other->flags & F_MOVABLE 
        && (self->flags & F_POLARITY) ^ (other->flags & F_POLARITY)); 
}

void move_sprite(Sprite* self, Delta d) {
    if (self != NULL) {
        self->d = d;
    }
}

void push_sprite(Sprite* self, Delta d) {
    if (self != NULL && !(self->flags & F_PLAYER_CHAR)) {
        self->d = d;
    }
}


void update_sprite(Sprite* self) {
    self->p = calc_point(self->p, self->d); 

    if (should_stop(self)) {
        self->d = (Delta) { 0, 0 };
    } 
}

bool should_stop(Sprite* self) {
    return (self->d.x != 0 && self->p.x % TILE_W == 0)
        || (self->d.y != 0 && self->p.y % TILE_H == 0);
}
