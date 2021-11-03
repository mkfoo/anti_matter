#include <stddef.h>
#include "sprite.h"

int should_stop(Sprite* self);

int is_moving(Sprite* self) {
    return self->d.x != 0 || self->d.y != 0;
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

int cmp_point(Point p1, Point p2) {
    return p1.x == p2.x && p1.y == p2.y;
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

int can_move(Adjacent* a) {
    return a->front == NULL 
        || (a->front->flags & F_MOVABLE && a->next == NULL);
}

int can_pull(Sprite* self, Sprite* other) {
    return other != NULL 
        && (other->flags & F_MOVABLE 
        && (self->flags & F_POLARITY) ^ (other->flags & F_POLARITY)); 
}

void update_sprite(Sprite* self) {
    self->p = calc_point(self->p, self->d); 

    if (should_stop(self)) {
        self->d = (Delta) { 0, 0 };
    } 
}

int should_stop(Sprite* self) {
    return (self->d.x != 0 && self->p.x % TILE_W == 0)
        || (self->d.y != 0 && self->p.y % TILE_H == 0);
}
