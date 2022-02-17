#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "antimatter.h"
#include "midi.h"

typedef enum {
    MSG_STOP,
    MSG_MUTE,
    MSG_VOL_DOWN,
    MSG_VOL_UP,
    MSG_PLAY = 1 << 7, 
    MSG_REPEAT = 1 << 8,
} AudioMessage;

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
    MidiSeq* midi;
    Channel chans[3]; 
} SoundGen;

SoundGen* sg_init(void);
void sg_handle_message(SoundGen* self, int msg);
void sg_generate_i16(SoundGen* self, uint8_t* stream, int len);
void sg_generate_f32(SoundGen* self, float* buf, int len);
void sg_quit(SoundGen* self);
