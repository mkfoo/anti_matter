#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "antimatter.h"
#include "backend.h"

typedef struct {
    int16_t vel;
    int32_t osc_phase;
    int32_t osc_period;
    int32_t env_phase;
    int32_t env_step;
    bool env_mode;
} Channel;

typedef struct {
    int16_t* buf;
    int16_t vol;
    Channel chans[3]; 
} SoundGen;

SoundGen* sg_init(void);

void sg_generate(SoundGen* self, Backend* be, uint32_t ticks);

void sg_change_vol(SoundGen* self, int16_t delta);

void sg_toggle_mute(SoundGen* self); 

void sg_quit(SoundGen* self);
