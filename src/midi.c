#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "antimatter.h"
#include "midi.h"

MidiFile load_test_midi(void);

MidiFile load_test_midi(void) {
    FILE* file = fopen("./assets/test.mid", "r");
    assert(file != NULL);
    int err = fseek(file, 0, SEEK_END);
    assert(!err);
    int64_t signed_len = ftell(file);
    assert(signed_len > 0); 
    size_t len = (size_t) signed_len; 
    rewind(file);
    uint8_t* data = calloc(len, sizeof(uint8_t)); 
    assert(data != NULL);
    size_t count = fread(data, sizeof(uint8_t), len, file);
    assert(count == len);
    return (MidiFile) { data, len };
}

MidiReader* reader_init(MidiFile file);
uint8_t read_u8(MidiReader* self);
uint16_t read_u16(MidiReader* self);
uint32_t read_u32(MidiReader* self);
uint32_t read_var_len(MidiReader* self);
void scan_for_tracks(MidiReader* self);
void set_track(MidiReader* self, uint16_t track_id);
TrackEvent read_event(MidiReader* self);
MidiEvent read_cvm(MidiReader* self, uint8_t byte1);
MidiEvent read_meta(MidiReader* self);
void reader_quit(MidiReader* self);

MidiReader* reader_init(MidiFile file) {
    MidiReader* self = calloc(1, sizeof(MidiReader));
    assert(self != NULL);
    self->ptr = file.data;
    self->data_end = self->ptr + file.len;
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

uint8_t read_u8(MidiReader* self) {
    assert(self->ptr < self->data_end);
    assert(self->ptr < self->track_end);
    return *(self->ptr++);
}

uint16_t read_u16(MidiReader* self) {
    return read_u8(self) << 8 | read_u8(self);
}

uint32_t read_u32(MidiReader* self) {
    return read_u8(self) << 24 |
           read_u8(self) << 16 |
           read_u8(self) << 8 |
           read_u8(self);
}

uint32_t read_var_len(MidiReader* self) {
    uint8_t byte = read_u8(self);
    uint32_t val = byte & 0x7f;

    while (byte & 0x80) {
        val <<= 7;
        byte = read_u8(self);
        val += byte & 0x7f;
    }

    return val;
}

void scan_for_tracks(MidiReader* self) {
    for (size_t i = 0; i < self->ntrks; i++) {
        assert(read_u32(self) == 0x4d54726b);
        uint32_t t_len = read_u32(self);
        self->tracks[i] = self->ptr;
        self->ptr += t_len;
    }
}

void set_track(MidiReader* self, uint16_t track_id) {
    assert(track_id < self->ntrks);
    self->ptr = self->tracks[track_id];
    self->status = 0;

    if (track_id < self->ntrks - 1) {
        self->track_end = self->tracks[track_id + 1] - 8;
    } else {
        self->track_end = self->data_end;
    }
}

TrackEvent read_event(MidiReader* self) {
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

MidiEvent read_cvm(MidiReader* self, uint8_t byte1) {
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

MidiEvent read_meta(MidiReader* self) {
    uint8_t type = read_u8(self);
    self->ptr += read_var_len(self);
    return (MidiEvent) { type, 0, 0 };
}

void reader_quit(MidiReader* self) {
    free(self->tracks);
    free(self);
}

MidiSeq* ms_init(void) {
    MidiSeq* self = calloc(1, sizeof(MidiSeq));
    assert(self != NULL);
    MidiFile file = load_test_midi();
    self->reader = reader_init(file);
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

MidiEvent ms_advance(MidiSeq* self, size_t i) {
    uint32_t now = self->clock + (i % SAMPLES_PER_TICK == 0);

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
