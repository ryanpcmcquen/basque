#include "initialization.h"

int main(int argc, char* argv[])
{
    // Fake out the 'unused' parameters so that
    // Clang does not complain. We do
    // actually need these for
    // Windows since SDL_main
    // uses them.
    (void)argc;
    (void)argv;

    memset(&app, 0, sizeof(App));
    memset(&game, 0, sizeof(GameState));

    init();

    atexit(cleanup);

    return 0;
}
