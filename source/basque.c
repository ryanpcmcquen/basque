#include "initialization.h"

int main(int argc, char* argv[])
{

    for (int argument_index = 0; argument_index < argc; argument_index++) {
        if (strcmp(argv[argument_index], "-f") == 0 || strcmp(argv[argument_index], "--fullscreen") == 0) {
            FULLSCREEN_MODE = 1;
        }
    }

    memset(&app, 0, sizeof(App));
    memset(&game, 0, sizeof(Game));

    init();

    atexit(cleanup);

    return 0;
}
