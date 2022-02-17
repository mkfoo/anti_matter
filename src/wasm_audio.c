#include "sound.h"

#define MAX_BUF_LEN 256

static SoundGen* sg = NULL;
static float* buf = NULL; 

__attribute__((export_name("am_audio_init")))
float* am_audio_init(void);

__attribute__((export_name("am_audio_recv_msg")))
void am_audio_recv_msg(int msg);

__attribute__((export_name("am_audio_generate")))
int am_audio_generate(int smpls);

float* am_audio_init(void) {
    sg = sg_init();
    buf = calloc(MAX_BUF_LEN, sizeof(float));

    if (sg == NULL || buf == NULL) {
        return NULL;
    }

    return buf;
}

void am_audio_recv_msg(int msg) {
    sg_handle_message(sg, msg);
}

int am_audio_generate(int smpls) {
    if (smpls > MAX_BUF_LEN) {
        return -1; 
    }

    sg_generate_f32(sg, buf, smpls); 
    return 0;
}

int main(void) {
    return 0;
}
