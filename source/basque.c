#include "initialization.h"

int main(int argc, char* argv[]) {

    for (int argument_index = 0; argument_index < argc; argument_index++) {
        if (strcmp(argv[argument_index], "-f") == 0 || strcmp(argv[argument_index], "--fullscreen") == 0) {
            FULLSCREEN_MODE = 1;
        }
    }

    memset(&app, 0, sizeof(App));
    memset(&game, 0, sizeof(Game));

    if (init(argc, argv) != 0) {
        return 1; // Error during initialization
    }

    atexit(cleanup);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_game_loop, 0, 1);
#endif

#ifndef __EMSCRIPTEN__
    while (!game.done) {
        main_game_loop();
    }
#endif

    return 0;
}
