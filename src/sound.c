#include <stdio.h>
#include <assert.h>
#include "sound.h"

#define ENV_PERIOD 32000
#define ENV_RATE 4
#define ENV_PREC 1000
#define OSC_PREC 100
#define MAX_VOL 10
#define RNG_SEED 0x1bac

const int32_t PTAB[128] = {
    539397, 509123, 480548, 453577, 428120, 404091, 381411, 360004, 339799, 320727, 302726, 285736,
    269698, 254561, 240274, 226788, 214060, 202046, 190706, 180002, 169899, 160364, 151363, 142868,
    134849, 127281, 120137, 113394, 107030, 101023, 95353, 90001, 84950, 80182, 75682, 71434,
    67425, 63640, 60068, 56697, 53515, 50511, 47676, 45001, 42475, 40091, 37841, 35717, 33712,
    31820, 30034, 28349, 26757, 25256, 23838, 22500, 21237, 20045, 18920, 17858, 16856, 15910,
    15017, 14174, 13379, 12628, 11919, 11250, 10619, 10023, 9460, 8929, 8428, 7955, 7509, 7087,
    6689, 6314, 5960, 5625, 5309, 5011, 4730, 4465, 4214, 3978, 3754, 3544, 3345, 3157, 2980, 2813,
    2655, 2506, 2365, 2232, 2107, 1989, 1877, 1772, 1672, 1578, 1490, 1406, 1327, 1253, 1183, 1116,
    1054, 994, 939, 886, 836, 789, 745, 703, 664, 626, 591, 558, 527, 497, 469, 443, 418, 395, 372, 352,
};

void reset_env(Channel* self);
void advance_env(Channel* self, size_t i);
void advance_rng(Channel* self, size_t i);
int16_t gen_sample(Channel* self);
void handle_midi_event(SoundGen* self, MidiEvent e);
void handle_cc(SoundGen* self, uint8_t chn, uint8_t cc, uint8_t val);

void reset_env(Channel* self) {
    self->env_phase = (self->env_step < 0) * ENV_PERIOD;
}

void advance_env(Channel* self, size_t i) {
    int32_t next = self->env_phase + self->env_step;
    bool cond = (0 <= next && next <= ENV_PERIOD) && (i % ENV_RATE == 0);
    self->env_phase += self->env_step * cond;
}

void advance_rng(Channel* self, size_t i) {
    if (self->rng_period && !(i % self->rng_period)) {
        int32_t rs = self->rng_state;
        self->rng_state = (rs >> 1) ^ ((rs & 1) << 13) ^ ((rs & 1) << 16);
    }
}

int16_t gen_sample(Channel* self) {
    self->osc_phase += OSC_PREC;
    self->osc_phase %= self->osc_period;
    bool cond = self->rng_period == 0;
    int32_t osc_v = (self->osc_phase < self->osc_period / 2) * 2 - 1;
    int32_t rng_v = (self->rng_state & 1) * 2 - 1;
    int32_t env_v = self->env_phase / ENV_PREC;
    int16_t out_v = osc_v * cond + rng_v * !cond;
    return (int16_t) (out_v * env_v * (self->vel / 4));
}

void handle_midi_event(SoundGen* self, MidiEvent e) {
    uint8_t status = e.status & 0xf0;
    uint8_t chn = (e.status & 0x0f) % 3;

    switch (status) {
        case NOTE_OFF:
            self->chans[chn].vel = 0;
            break;
        case NOTE_ON:
            if (e.data2) {
                reset_env(&self->chans[chn]);
                self->chans[chn].osc_period = PTAB[e.data1];
            }
            self->chans[chn].vel = e.data2;
            break;
        case CONTROL_CHANGE:
            handle_cc(self, chn, e.data1, e.data2);
            break;
        default:
            break;
    }
}

void handle_cc(SoundGen* self, uint8_t chn, uint8_t cc, uint8_t val) {
    switch (cc) {
        case 80:
            self->chans[chn].env_step = 64 - (int32_t) val;
            break;
        case 81:
            self->chans[chn].rng_period = val * 4;
            break;
        default:
            break;
    }
}

SoundGen* sg_init(void) {
    SoundGen* self = calloc(1, sizeof(SoundGen));
    assert(self != NULL);
    self->vol = MAX_VOL / 2;
    self->buf = calloc(BUF_LEN, sizeof(int16_t));
    assert(self->buf != NULL);
    self->midi = ms_init();
    self->chans[0].osc_period = PTAB[60];
    self->chans[1].osc_period = PTAB[60];
    self->chans[2].osc_period = PTAB[60];
    self->chans[0].rng_state = RNG_SEED;
    self->chans[1].rng_state = RNG_SEED;
    self->chans[2].rng_state = RNG_SEED;
    return self;
}

void sg_play(SoundGen* self, uint16_t track_id) {
    if (!self->midi->playing) {
        ms_play_track(self->midi, track_id);
    }
}

void sg_stop(SoundGen* self) {
    ms_stop(self->midi);
    self->chans[0].vel = 0;
    self->chans[1].vel = 0;
    self->chans[2].vel = 0;
}

void sg_generate(SoundGen* self, Backend* be, uint32_t lag) {
    if (self->vol > 0) {
        size_t smpls = ceil(SAMPLE_RATE / 1000.0f * (float) lag);
        if (smpls > BUF_LEN) smpls = BUF_LEN;
        MidiEvent event;
        int16_t out;

        for (size_t i = 0; i < smpls; i++) {
            do {
                event = ms_advance(self->midi);
                handle_midi_event(self, event);
            } while (event.status);

            out = 0;

            for (size_t c = 0; c < 3; c++) {
                advance_env(&self->chans[c], i);
                advance_rng(&self->chans[c], i);
                out += gen_sample(&self->chans[c]);
            }

            self->buf[i] = out * self->vol;
        }

        uint32_t len = smpls * sizeof(int16_t);
        be_queue_audio(be, self->buf, len);
    }
}

void sg_change_vol(SoundGen* self, int16_t delta) {
    int16_t new_vol = self->vol + delta;

    if (new_vol > MAX_VOL) {
        new_vol = MAX_VOL;
    } else if (new_vol < 0) {
        new_vol = 0;
    }

    self->vol = new_vol;
}

void sg_toggle_mute(SoundGen* self) {
    if (self->vol != 0) {
        self->vol = 0;
    } else {
        self->vol = MAX_VOL / 2;
    }
}

void sg_quit(SoundGen* self) {
    free(self->buf);
    free(self);
}
