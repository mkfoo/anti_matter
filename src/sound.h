#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include "antimatter.h"
#include "backend.h"
#include "midi.h"

typedef struct {
    int16_t vel;
    int32_t osc_phase;
    int32_t osc_period;
    int32_t env_phase;
    int32_t env_step;
    int32_t rng_state;
    int32_t rng_period; 
} Channel;

typedef struct {
    int16_t vol;
    int16_t prev_vol;
    int16_t* buf;
    MidiSeq* midi;
    Channel chans[3]; 
} SoundGen;

SoundGen* sg_init(void);
void sg_play(SoundGen* self, uint16_t track_id);
void sg_stop(SoundGen* self);
bool sg_is_playing(SoundGen* self);
void sg_generate(SoundGen* self, Backend* be, uint32_t lag);
void sg_change_vol(SoundGen* self, int16_t delta);
void sg_toggle_mute(SoundGen* self); 
void sg_quit(SoundGen* self);
