#include "sound.h"

#define ENV_PERIOD 32000
#define ENV_RATE 4
#define ENV_PREC 1000
#define OSC_PREC 100
#define MAX_VOL 10

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

void advance_env(Channel* self);

int16_t gen_sample(Channel* self);

void reset_env(Channel* self) {
    self->env_phase = self->env_mode * ENV_PERIOD;
}

void advance_env(Channel* self) {
    int32_t step = self->env_step * (!self->env_mode * 2 - 1);
    int32_t next = self->env_phase + step;
    bool cond = 0 <= next && next <= ENV_PERIOD;
    self->env_phase += step * cond;
}

int16_t gen_sample(Channel* self) {
    self->osc_phase += OSC_PREC;
    self->osc_phase %= self->osc_period;
    int32_t osc_v = (self->osc_phase < self->osc_period / 2) * 2 - 1;
    int32_t env_v = self->env_phase / ENV_PREC;
    return (int16_t) (osc_v * env_v * (self->vel / 4));
}

SoundGen* sg_init(void) {
    SoundGen* self = calloc(1, sizeof(SoundGen));
    assert(self != NULL);
    self->buf = calloc(BUF_LEN, sizeof(int16_t));
    assert(self->buf != NULL);
    self->vol = MAX_VOL / 2;
    self->chans[0].osc_period = PTAB[60];
    self->chans[1].osc_period = PTAB[60];
    self->chans[2].osc_period = PTAB[60];
    return self;
}

void sg_generate(SoundGen* self, Backend* be, uint32_t ticks) {
    if (self->vol > 0) {
        uint32_t smpls = ticks * SAMPLES_PER_TICK;

        if (smpls > BUF_LEN) {
            smpls = BUF_LEN;
        }

        int16_t out = 0;

        for (size_t i = 0; i < smpls; i++) {
            if (i % ENV_RATE == 0) {
                advance_env(&self->chans[0]);
                advance_env(&self->chans[1]);
                advance_env(&self->chans[2]);
            }

            out = gen_sample(&self->chans[0]);
            out += gen_sample(&self->chans[1]);
            out += gen_sample(&self->chans[2]);
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
