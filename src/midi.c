#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "antimatter.h"
#include "midi.h"
#include "midi_data.h"

static MidiReader* reader_init(void);
static uint8_t read_u8(MidiReader* self);
static uint16_t read_u16(MidiReader* self);
static uint32_t read_u32(MidiReader* self);
static uint32_t read_var_len(MidiReader* self);
static void scan_for_tracks(MidiReader* self);
static void set_track(MidiReader* self, uint16_t track_id);
static TrackEvent read_event(MidiReader* self);
static MidiEvent read_cvm(MidiReader* self, uint8_t byte1);
static MidiEvent read_meta(MidiReader* self);
static void reader_quit(MidiReader* self);

static MidiReader* reader_init(void) {
    MidiReader* self = calloc(1, sizeof(MidiReader));
    assert(self != NULL);
    self->ptr = (uint8_t*) MIDI_DATA;
    size_t len = sizeof MIDI_DATA;
    self->data_end = self->ptr + len;
    self->track_end = self->data_end;
    assert(read_u32(self) == 0x4d546864);
    assert(read_u32(self) == 6);
    assert(read_u16(self));
    self->ntrks = read_u16(self);
    assert(self->ntrks);
    assert(read_u16(self));
    self->tracks = calloc(self->ntrks, sizeof(uint8_t**));
    assert(self->tracks != NULL);
    scan_for_tracks(self);
    return self;
}

static uint8_t read_u8(MidiReader* self) {
    assert(self->ptr < self->data_end);
    assert(self->ptr < self->track_end);
    return *(self->ptr++);
}

static uint16_t read_u16(MidiReader* self) {
    return read_u8(self) << 8 | read_u8(self);
}

static uint32_t read_u32(MidiReader* self) {
    return read_u8(self) << 24 |
           read_u8(self) << 16 |
           read_u8(self) << 8 |
           read_u8(self);
}

static uint32_t read_var_len(MidiReader* self) {
    uint8_t byte = read_u8(self);
    uint32_t val = byte & 0x7f;

    while (byte & 0x80) {
        val <<= 7;
        byte = read_u8(self);
        val += byte & 0x7f;
    }

    return val;
}

static void scan_for_tracks(MidiReader* self) {
    for (size_t i = 0; i < self->ntrks; i++) {
        assert(read_u32(self) == 0x4d54726b);
        uint32_t t_len = read_u32(self);
        self->tracks[i] = self->ptr;
        self->ptr += t_len;
    }
}

static void set_track(MidiReader* self, uint16_t track_id) {
    assert(track_id < self->ntrks);
    self->ptr = self->tracks[track_id];
    self->status = 0;

    if (track_id < self->ntrks - 1) {
        self->track_end = self->tracks[track_id + 1] - 8;
    } else {
        self->track_end = self->data_end;
    }
}

static TrackEvent read_event(MidiReader* self) {
    uint32_t delta = read_var_len(self);
    uint8_t byte0 = read_u8(self);
    MidiEvent event;

    if (byte0 == 0xff) {
        self->status = 0;
        event = read_meta(self);
    } else if (byte0 & 0x80) {
        self->status = byte0;
        event = read_cvm(self, read_u8(self));
    } else {
        event = read_cvm(self, byte0);
    }

    return  (TrackEvent) { delta, event }; 
}

static MidiEvent read_cvm(MidiReader* self, uint8_t byte1) {
    uint8_t data1 = byte1 & 0x7f;
    uint8_t data2 = 0;

    switch (self->status & 0xf0) {
        case NOTE_OFF:
        case NOTE_ON:
        case POLY_AFTERTOUCH:
        case CONTROL_CHANGE:
        case PITCH_BEND:
            data2 = read_u8(self) & 0x7f;
            break;
        case PROGRAM_CHANGE:
        case CHANNEL_AFTERTOUCH:
            break;
        default:
            printf("Unrecognized MIDI event %x\n", self->status);
            abort();
    }

    return (MidiEvent) { self->status, data1, data2 };
}

static MidiEvent read_meta(MidiReader* self) {
    uint8_t type = read_u8(self);
    self->ptr += read_var_len(self);
    return (MidiEvent) { type, 0, 0 };
}

static void reader_quit(MidiReader* self) {
    free(self->tracks);
    free(self);
}

MidiSeq* ms_init(void) {
    MidiSeq* self = calloc(1, sizeof(MidiSeq));
    assert(self != NULL);
    self->reader = reader_init();
    self->playing = false;
    return self;
}

void ms_play_track(MidiSeq* self, uint16_t track_id) {
    set_track(self->reader, track_id); 
    TrackEvent te = read_event(self->reader);
    self->clock = 0;
    self->next = te.delta;
    self->event = te.event;
    self->playing = te.event.status != END_OF_TRACK;
}

void ms_stop(MidiSeq* self) {
    self->playing = false;
}

MidiEvent ms_advance(MidiSeq* self) {
    self->count++;
    self->count %= SAMPLES_PER_TICK;
    uint32_t now = self->clock + (self->count == 0);

    if (self->playing && now >= self->next) {
        TrackEvent te = read_event(self->reader);
        MidiEvent ret = self->event;
        self->clock = self->next;
        self->next += te.delta;
        self->event = te.event;
        self->playing = te.event.status != END_OF_TRACK;
        return ret;
    }

    self->clock = now;
    return (MidiEvent) { 0, 0, 0 };
}

void ms_quit(MidiSeq* self) {
    reader_quit(self->reader);
    free(self);
}
