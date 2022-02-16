#include <stdint.h>
#include <stdbool.h>

typedef enum {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    POLY_AFTERTOUCH = 0xa0,
    CONTROL_CHANGE = 0xb0,
    PROGRAM_CHANGE = 0xc0,
    CHANNEL_AFTERTOUCH = 0xd0,
    PITCH_BEND = 0xe0,
    END_OF_TRACK = 0x2f,
} StatusType;

typedef struct {
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
} MidiEvent;

typedef struct {
    uint32_t delta;
    MidiEvent event;
} TrackEvent;

typedef struct {
    uint8_t status;
    uint8_t* ptr;
    uint8_t* data_end;
    uint8_t* track_end;
    uint16_t ntrks;
    uint8_t** tracks;
} MidiReader;

typedef struct {
    bool playing;
    uint16_t repeat;
    MidiReader* reader; 
    MidiEvent event;
    uint32_t count;
    uint32_t clock;
    uint32_t next;
} MidiSeq;

MidiSeq* ms_init(void);
void ms_play_track(MidiSeq* self, uint16_t track_id); 
void ms_stop(MidiSeq* self);
MidiEvent ms_advance(MidiSeq* self);
void ms_quit(MidiSeq* self);
