#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "antimatter.h"

typedef enum {
    F_NIL = 1 << 0,
    F_PLAYER_CHAR = 1 << 1,
    F_MOVABLE = 1 << 2,
    F_ANIMATED = 1 << 3,
    F_POLARITY = 1 << 4,
    F_UNSTABLE = 1 << 5,
    F_DESTROY = 1 << 6,
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

typedef enum {
    ID_NIL,
    ID_ANTI,
    ID_MATTER,
    ID_BLOB_B,
    ID_BLOB_R,
    ID_BOMB_B,
    ID_BOMB_R,
} SpriteId;

typedef struct {
    Sprite* front;
    Sprite* back;
    Sprite* next;
    Point next_p;
} Adjacent;

Delta get_delta(Sprite* self, Sprite* other);
Delta invert_delta(Delta d);
Point calc_point(Point p, Delta d); 
Point calc_tile(Point p, Delta d); 
bool has_flag(Sprite* self, Flag flag);
bool is_moving(Sprite* self);
bool is_aligned(Sprite* self);
bool is_overlapping(Sprite* self, Sprite* other);
bool point_equals(Point p1, Point p2);
bool point_between(Point self, Point p1, Point p2);
bool can_move(Adjacent* a); 
bool can_move_both(Adjacent* a, Adjacent* b); 
bool can_pull(Sprite* self, Sprite* other);
void move_sprite(Sprite* self, Adjacent* a, Delta d);
void push_sprite(Sprite* self, Delta d);
void update_sprite(Sprite* self);
void destroy_sprite(Sprite* self);
