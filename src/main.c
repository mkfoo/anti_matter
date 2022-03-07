#include "gamestate.h"

static Backend* be = NULL;
static GameState* gs = NULL;

#ifdef WASM_BACKEND
__attribute__((export_name("am_init")))
#endif
int am_init(int gl);

#ifdef WASM_BACKEND
__attribute__((export_name("am_update")))
#endif
int am_update(double timestamp);

void am_quit(void);

int am_init(int gl) {
    if (be == NULL && gs == NULL) {
        be = be_init(gl);
        gs = gs_init();

        if (be == NULL || gs == NULL) { 
            return -1; 
        }

        be_send_audiomsg(be, MSG_PLAY);
        be_set_render_target(be, 1);
        gs_decorate(be);
        be_set_render_target(be, 0);
        be_set_color(be, 4);

        return 0;
    }

    return -1;
}

int am_update(double timestamp) {
    if (be == NULL || gs == NULL) { 
        return 0;
    }

    return gs_update(gs, be, timestamp);
}

void am_quit(void) {
    if (gs != NULL) {
        gs_quit(gs);
        gs = NULL;
    }

    if (be != NULL) {
        be_quit(be);
        be = NULL;
    }
}

int main(void) {
    if (am_init(1)) {
        return EXIT_FAILURE;
    }

    double time = be_get_millis();

    while (am_update(time)) {
        gs_limit_fps(gs);    
        time = be_get_millis();
    }

    am_quit();
    return EXIT_SUCCESS;
}
