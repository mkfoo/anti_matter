#include "antimatter.h"
#include "backend.h"
#include "timer.h"

void t_advance(Timer* self) {
    uint32_t curr = be_get_millis();
    uint32_t lag = curr - self->prev + self->lag;
    uint32_t ticks = lag / MS_PER_TICK;
    self->phase = fmodf(self->phase + ANIM_SPEED * (float) ticks, 1.0f);
    self->prev = curr;
    self->lag = lag % MS_PER_TICK;
    self->ticks = ticks;
}

void t_set_delay(Timer* self, uint32_t delay) {
    self->start = self->prev;
    self->delay = delay * 1000;
}

float t_get_phase(Timer* self) {
    if (self->delay == 0) {
        return self->phase;
    }

    if (self->prev < (self->start + self->delay)) {
        return (float) (self->prev - self->start) / (float) self->delay;
    }

    return 1.0f;
}

void t_limit_fps(Timer* self) {
    uint32_t next = self->prev + MS_PER_FRAME;
    uint32_t now = be_get_millis();

    if (next > now) {
        be_delay(next - now);
    }
}
