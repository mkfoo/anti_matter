#pragma once

typedef struct {
    float phase;
    uint32_t prev;
    uint32_t lag;
    uint32_t ticks;
} Timer;

void t_advance(Timer* self);

float t_get_phase(Timer* self);

void t_limit_fps(Timer* self);

