#pragma once

#include <stdint.h>
#include "antimatter.h"

typedef enum {
    F_PLAYER_CHAR = 1 << 0,
    F_MOVABLE = 1 << 1,
    F_ANIMATED = 1 << 2,
    F_POLARITY = 1 << 3,
} Flag;

typedef struct {
    int16_t x;
    int16_t y;
} Point;

typedef struct {
    int8_t x;
    int8_t y;
} Delta;

typedef struct {
    Point p;
    Delta d;
    uint8_t flags;
    uint8_t tile;
} Sprite;

typedef struct {
    Sprite* front;
    Sprite* back;
    Sprite* next;
} Adjacent;

int is_moving(Sprite* self);
Delta invert_delta(Delta d);
Point calc_point(Point p, Delta d); 
Point calc_tile(Point p, Delta d); 
int cmp_point(Point p1, Point p2);
void move_sprite(Sprite* self, Delta d);
void push_sprite(Sprite* self, Delta d);
int can_move(Adjacent* a); 
int can_pull(Sprite* self, Sprite* other);
void update_sprite(Sprite* self);
