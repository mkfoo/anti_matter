#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef struct {
    uint8_t* data;
    size_t len;
} Buf;

static uint16_t get_u16(Buf* b, size_t offs);
static uint32_t get_u32(Buf* b, size_t offs);
static Buf new_buf(size_t len);
static Buf load_file(const char* path);
static Buf get_pixel_data(Buf b);
static void write_file(const char* path, const char* name, Buf b);

static uint16_t get_u16(Buf* b, size_t offs) {
    assert(offs < b->len);
    uint8_t* ptr = b->data + offs;
    return ptr[1] << 8 | ptr[0];
}

static uint32_t get_u32(Buf* b, size_t offs) {
    assert(offs < b->len);
    uint8_t* ptr = b->data + offs;
    return ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0];
}

static Buf new_buf(size_t len) {
    uint8_t* data = calloc(len, sizeof(uint8_t));
    assert(data != NULL);
    return (Buf) { data, len };
}

static Buf load_file(const char* path) {
    FILE* file = fopen(path, "r");
    assert(file != NULL);
    int err = fseek(file, 0, SEEK_END);
    assert(!err);
    int64_t signed_len = ftell(file);
    assert(signed_len > 0);
    Buf b = new_buf((size_t) signed_len);
    rewind(file);
    size_t count = fread(b.data, sizeof(uint8_t), b.len, file);
    assert(count == b.len);
    assert(!fclose(file));
    return b;
}

static Buf get_pixel_data(Buf in) {
    assert(get_u32(&in, 0xe) == 40); 
    assert(get_u16(&in, 0x1c) == 8); 
    uint32_t width = get_u32(&in, 0x12);
    uint32_t height = get_u32(&in, 0x16);
    uint32_t data_size = get_u32(&in, 0x22);
    assert(width == 160 && height == 184);
    assert(width * height == data_size);
    Buf out = new_buf(data_size);
    uint8_t* src = in.data + in.len;
    uint8_t* dst = out.data;

    for (size_t r = 0; r < height; r++) {
        src -= width;
        memcpy(dst, src, width);
        dst += width;
    }

    return out;
}

static void write_file(const char* path, const char* name, Buf b) {
    FILE* file = fopen(path, "w");
    assert(file != NULL);
    fprintf(file, "static const uint8_t %s[%lu] = {\n", name, b.len);

    for (size_t i = 0; i < b.len; i++) {
        fprintf(file, "%3u,", b.data[i]);

        if (i % 16 == 15) {
            fprintf(file, "\n"); 
        }
    }
            
    fprintf(file, "};\n"); 
    assert(!fclose(file));
}

int main(void) {
    Buf bmp = load_file("../../assets/graphics.bmp");
    Buf mid = load_file("../../assets/music.mid");
    Buf pixels = get_pixel_data(bmp);
    write_file("../../src/texture_data.h", "TEXTURE_DATA", pixels);
    write_file("../../src/midi_data.h", "MIDI_DATA", mid);
}
