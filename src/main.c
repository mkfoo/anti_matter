#include "gamestate.h"

int main(void) {
    Backend* be = be_init();
    GameState* gs = gs_init();

    if (be == NULL || gs == NULL) {
        return EXIT_FAILURE;
    }

    while (gs_update(gs, be));

    gs_quit(gs);
    be_quit(be);

    return EXIT_SUCCESS;
}
