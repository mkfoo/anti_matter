#include "sound.h"

#define ENV_PERIOD 32000
#define ENV_RATE 4
#define ENV_PREC 1000
#define OSC_PREC 100
#define MAX_VOL 10
#define RNG_SEED 0x1bac

static const int32_t PTAB[128] = {
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

static void reset_env(Channel* self);
static void advance_env(Channel* self, size_t i);
static void advance_rng(Channel* self, size_t i);
static int16_t gen_sample(Channel* self);
static void handle_midi_event(SoundGen* self, MidiEvent e);
static void handle_cc(SoundGen* self, uint8_t chn, uint8_t cc, uint8_t val);
static void play(SoundGen* self, uint16_t track_id);
static void stop(SoundGen* self);
static void change_vol(SoundGen* self, int16_t delta);
static void toggle_mute(SoundGen* self); 

static void reset_env(Channel* self) {
    self->env_phase = (self->env_step < 0) * ENV_PERIOD;
}

static void advance_env(Channel* self, size_t i) {
    int32_t next = self->env_phase + self->env_step;
    bool cond = (0 <= next && next <= ENV_PERIOD) && (i % ENV_RATE == 0);
    self->env_phase += self->env_step * cond;
}

static void advance_rng(Channel* self, size_t i) {
    if (self->rng_period && !(i % self->rng_period)) {
        int32_t rs = self->rng_state;
        self->rng_state = (rs >> 1) ^ ((rs & 1) << 13) ^ ((rs & 1) << 16);
    }
}

static int16_t gen_sample(Channel* self) {
    self->osc_phase += OSC_PREC;
    self->osc_phase %= self->osc_period;
    bool cond = self->rng_period == 0;
    int32_t osc_v = (self->osc_phase < self->osc_period / 2) * 2 - 1;
    int32_t rng_v = (self->rng_state & 1) * 2 - 1;
    int32_t env_v = self->env_phase / ENV_PREC;
    int16_t out_v = osc_v * cond + rng_v * !cond;
    return (int16_t) (out_v * env_v * (self->vel / 4));
}

static void handle_midi_event(SoundGen* self, MidiEvent e) {
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
        case PITCH_BEND:
            self->chans[chn].osc_period += -8192 + ((e.data2 << 7) | e.data1);
            break;
        default:
            break;
    }
}

static void handle_cc(SoundGen* self, uint8_t chn, uint8_t cc, uint8_t val) {
    switch (cc) {
        case 80:
            self->chans[chn].env_step = -64 + val;
            break;
        case 81:
            self->chans[chn].rng_period = val;
            break;
        default:
            break;
    }
}

static void play(SoundGen* self, uint16_t track_id) {
    if (!self->midi->playing) {
        ms_play_track(self->midi, track_id);

        for (size_t c = 0; c < 3; c++) {
            self->chans[c].rng_period = 0;
        }
    }
}

static void stop(SoundGen* self) {
    if (self->midi->playing) {
        ms_stop(self->midi);
    }

    for (size_t c = 0; c < 3; c++) {
        self->chans[c].vel = 0;
    }
}

static void change_vol(SoundGen* self, int16_t delta) {
    int16_t new_vol = self->vol + delta;

    if (new_vol > MAX_VOL) {
        new_vol = MAX_VOL;
    } else if (new_vol < 0) {
        new_vol = 0;
    }

    self->vol = new_vol;
}

static void toggle_mute(SoundGen* self) {
    if (self->vol != 0) {
        self->prev_vol = self->vol;
        self->vol = 0;
    } else {
        self->vol = self->prev_vol;
    }
}

SoundGen* sg_init(void) {
    SoundGen* self = calloc(1, sizeof(SoundGen));
    LOG_ERR(self == NULL, "alloc failure")

    self->vol = MAX_VOL / 2;
    self->prev_vol = MAX_VOL / 2;

    self->midi = ms_init();
    LOG_ERR(self->midi == NULL, "sequencer error")
    
    for (size_t c = 0; c < 3; c++) {
        self->chans[c].osc_period = PTAB[60];
        self->chans[c].rng_state = RNG_SEED;
    }

    return self;
}

void sg_handle_message(SoundGen* self, int msg) {
    switch (msg) {
        case MSG_STOP:
            stop(self);
            break;
        case MSG_MUTE:
            toggle_mute(self);
            break;
        case MSG_VOL_UP:
            change_vol(self, 1);
            break;
        case MSG_VOL_DOWN:
            change_vol(self, -1);
            break;
        default:
            if (msg & MSG_PLAY) {
                play(self, msg & 0x7f);
            }

            if (msg & MSG_REPEAT) {
                self->midi->repeat = msg & 0x7f; 
            }
    }
}

void sg_generate_i16(SoundGen* self, uint8_t* stream, int len) {
    int16_t* buf = (int16_t*) stream; 
    size_t smpls = (size_t) len / sizeof(int16_t);
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

        buf[i] = out * self->vol;
    }
}

void sg_generate_f32(SoundGen* self, float* buf, int len) {
    size_t smpls = (size_t) len;
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

        float fout = (float) (out * self->vol);
        buf[i] = fout / 32767.0f;
    }
}

void sg_quit(SoundGen* self) {
    ms_quit(self->midi);
    free(self);
}
