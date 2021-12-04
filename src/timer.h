#pragma once

#include <stdint.h>

typedef struct {
    float phase;
    uint32_t start;
    uint32_t delay;
    uint32_t prev;
    uint32_t lag;
    uint32_t ticks;
} Timer;

void t_advance(Timer* self);

void t_set_delay(Timer* self, uint32_t delay);

float t_get_phase(Timer* self);

void t_limit_fps(Timer* self);

