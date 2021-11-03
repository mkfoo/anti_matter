#pragma once
#include "sprite.h"

const Sprite PROTOTYPES[5] = {
    { .flags = 0, 0 },
    { .flags = F_PLAYER_CHAR | F_ANIMATED, 1 },
    { .flags = F_PLAYER_CHAR | F_ANIMATED | F_POLARITY, 5 },
    { .flags = F_MOVABLE, 37 },
    { .flags = F_MOVABLE | F_POLARITY, 38 },
};

typedef enum {
    ID_NULL,
    ID_ANTI,
    ID_MATTER,
} SpriteId;

const uint8_t LEVEL_DATA[MAP_W * MAP_H] = {
    46, 44, 42,  0,  0,  0,  0,  0, 49, 50, 49,  
    45,  2,  0,  0,  0,  0,  0,  0,  0,  4, 51,  
    42,  0,  0,  0,  0,  0,  0,  0,  0,  0, 49,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
    49,  0,  0,  0,  0,  0,  0,  0,  0,  0, 40,  
    51,  3,  0,  0,  0,  0,  0,  0,  0,  1, 45,  
    49, 50, 49,  0,  0,  0,  0,  0, 40, 44, 46,  
};
