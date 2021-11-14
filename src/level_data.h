#pragma once
#include "sprite.h"

const Sprite PROTOTYPES[5] = {
    { .flags = F_NIL, 0 },
    { .flags = F_PLAYER_CHAR | F_ANIMATED | F_UNSTABLE, 1 },
    { .flags = F_PLAYER_CHAR | F_ANIMATED | F_UNSTABLE | F_POLARITY , 5 },
    { .flags = F_MOVABLE | F_ANIMATED, 21 },
    { .flags = F_MOVABLE | F_ANIMATED | F_POLARITY, 25 },
};

const uint8_t LEVEL_DATA[MAP_W * MAP_H] = {
    46, 44, 42,  0,  0,  0,  0,  0, 49, 50, 49,  
    45,  2,  0,  0,  0,  0,  0,  0, 51,  4, 51,  
    42,  0,  0,  0,  0,  0,  0,  0, 49,  4, 49,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
    49,  3, 49,  0,  0,  0,  0,  0,  0,  0, 40,  
    51,  3, 51,  0,  0,  0,  0,  0,  0,  1, 45,  
    49, 50, 49,  0,  0,  0,  0,  0, 40, 44, 46,  
};
