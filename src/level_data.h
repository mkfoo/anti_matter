#pragma once
#include "sprite.h"

static const Sprite PROTOTYPES[7] = {
    { .flags = F_NIL, 0 },
    { .flags = F_PLAYER_CHAR | F_ANIMATED | F_UNSTABLE, 1 },
    { .flags = F_PLAYER_CHAR | F_ANIMATED | F_UNSTABLE | F_POLARITY, 9 },
    { .flags = F_MOVABLE | F_ANIMATED, 17 },
    { .flags = F_MOVABLE | F_ANIMATED | F_POLARITY, 25 },
    { .flags = F_MOVABLE | F_ANIMATED | F_UNSTABLE, 33 },
    { .flags = F_MOVABLE | F_ANIMATED | F_UNSTABLE | F_POLARITY, 37 },
};

static const int32_t LEVEL_ENERGY[MAX_LEVEL] = {
    2256, 1200, 1680, 1728, 5008, 2608, 8000
};

static const uint8_t LEVEL_DATA[MAP_W * MAP_H * MAX_LEVEL] = {
    50, 54, 54, 54, 54, 54, 54, 54, 54, 54, 51,  
    55,  2,  0,  0,  0,  0,  0,  0,  0,  0, 55,  
    55,  0,  0, 50, 54, 88, 54, 51,  0,  0, 55,  
    55,  0,  0, 59, 60, 87, 60, 59,  0,  4, 55,  
    55,  0,  0, 53, 54, 89, 54, 89, 54, 54, 58,  
    55,  0,  0,  0,  0,  0,  0,  0,  0,  0, 55,  
    57, 54, 54, 88, 54, 88, 54, 51,  0,  0, 55,  
    55,  0,  0, 59, 60, 87, 60, 59,  0,  0, 55,  
    55,  0,  0, 53, 54, 89, 54, 52,  0,  0, 55,  
    55,  3,  0,  0,  0,  0,  0,  0,  0,  1, 55,  
    53, 54, 54, 54, 54, 54, 54, 54, 54, 54, 52,  

    54, 54, 54, 54, 54, 51,  0, 59, 60, 60, 60,  
     0,  0,  0,  0,  0, 55,  0,  0,  0,  0,  0,  
    60, 60, 60, 59,  0, 53, 54, 54,  0, 54, 54,  
     0,  0,  0,  0,  0,  0,  0,  0,  3,  0,  0,  
     0,  0,  4,  0,  0,  0,  0,  0,  3,  0,  0,  
     0,  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  
    54, 54,  0, 54, 54, 51,  0, 59, 60, 60, 60,  
     0,  0,  0,  0,  0, 55,  0,  0,  0,  0,  0,  
    60, 60, 60, 59,  0, 53, 54, 54, 54, 54, 54,  
     1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  

     0,  0,  0,  0, 50, 88, 51,  0,  0,  0,  0,  
     0,  2,  0,  0,  6,  4,  6,  0,  0,  5,  0,  
     0,  0,  0,  0, 53, 89, 52,  0,  0,  0,  0,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
    50,  5, 51,  0,  0,  0,  0,  0, 50,  5, 51,  
    57,  3, 58,  0,  0,  0,  0,  0, 57,  3, 58,  
    53,  5, 52,  0,  0,  0,  0,  0, 53,  5, 52,  
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
     0,  0,  0,  0, 50, 88, 51,  0,  0,  0,  0,  
     0,  6,  0,  0,  6,  4,  6,  0,  0,  1,  0,  
     0,  0,  0,  0, 53, 89, 52,  0,  0,  0,  0,  
    
    54, 88, 54, 54, 54, 54, 54, 88, 54, 54, 54,   
    54, 89, 54, 54, 88, 54, 54, 89, 54, 54, 88,   
    54, 88, 54, 54, 89, 54, 54, 88, 54, 54, 89,   
    54, 89, 54, 54, 54, 54, 54, 89, 54, 54, 54,   
     2,  5,  6,  4,  6,  6,  4,  3,  6,  3,  4,  
     5,  4,  3,  6,  3,  6,  3,  5,  3,  4,  5,  
     4,  3,  5,  3,  4,  5,  6,  4,  5,  6,  1,  
    54, 54, 54, 54, 88, 54, 54, 54, 54, 54, 88,  
    54, 88, 54, 54, 89, 54, 54, 88, 54, 54, 89,   
    54, 89, 54, 54, 88, 54, 54, 89, 54, 54, 88,  
    54, 54, 54, 54, 89, 54, 54, 54, 54, 54, 89,  

     2,  0,  0, 55,  0,  0,  0, 55,  0,  0,  0,  
     0, 59,  0,  0,  0,  4,  0, 55,  0,  3,  0,  
     0,  0,  1, 55,  0,  0,  0, 55,  0,  0,  0,  
    54,  0, 54, 56, 54,  0, 54, 56, 54,  0, 54,  
     0,  0,  0, 55,  0,  0,  0, 55,  0,  0,  0,  
     0,  4,  0,  0,  0,  3,  0,  0,  0,  4,  0,  
     0,  0,  0, 55,  0,  0,  0, 55,  0,  0,  0,  
    54, 54, 54, 56, 54,  0, 54, 56, 54,  0, 54,  
     0,  0,  0, 55,  0,  0,  0, 55,  0,  0,  0,  
     0,  3,  0,  0,  0,  4,  0,  0,  0,  3,  0,  
     0,  0,  0, 55,  0,  0,  0, 55,  0,  0,  0,  

     0, 55, 55,  0,  0,  0,  0,  0, 55, 55,  0,  
    54, 56, 52,  0,  0,  0,  0,  0, 53, 56, 54,  
    54, 52,  0,  0,  0,  4,  0,  0,  0, 53, 54,  
     0,  0,  0,  0,  6,  5,  6,  0,  0,  0,  0,  
     0,  0,  0,  6,  5,  3,  5,  6,  0,  0,  0,  
     0,  1,  4,  5,  3,  6,  3,  5,  4,  2,  0,  
     0,  0,  0,  6,  5,  3,  5,  6,  0,  0,  0,  
     0,  0,  0,  0,  6,  5,  6,  0,  0,  0,  0,  
    54, 51,  0,  0,  0,  4,  0,  0,  0, 50, 54,  
    54, 56, 51,  0,  0,  0,  0,  0, 50, 56, 54,  
     0, 55, 55,  0,  0,  0,  0,  0, 55, 55,  0,  

     6,  6,  6,  6,  6,  0,  5,  5,  5,  5,  5,  
     6,  0,  0,  0,  0, 59,  0,  0,  0,  0,  5,  
     6,  0,  2,  0,  0, 61,  0,  0,  0,  0,  5,  
     6,  0,  0,  0,  3, 61,  4,  0,  0,  0,  5,  
     6,  0,  0,  3,  3, 61,  4,  4,  0,  0,  5,  
     0, 59, 60, 60, 60, 87, 60, 60, 60, 59,  0,  
     5,  0,  0,  4,  4, 61,  3,  3,  0,  0,  6,  
     5,  0,  0,  0,  4, 61,  3,  0,  0,  0,  6,  
     5,  0,  0,  0,  0, 61,  0,  0,  1,  0,  6,  
     5,  0,  0,  0,  0, 59,  0,  0,  0,  0,  6,  
     5,  5,  5,  5,  5,  0,  6,  6,  6,  6,  6,  
};
