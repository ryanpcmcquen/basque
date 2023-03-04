#include "types.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN64) || defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(__WINRT__)
#define PLATFORM_IS_WINDOWS
#endif

int file_exists(char* path)
{
    struct stat filestat;
    return (stat(path, &filestat) == 0);
}

time_t read_file_time(char* path)
{
    struct stat filestat;
    stat(path, &filestat);

    return filestat.st_mtime;
}

File_Data read_file(char* path)
{
    SDL_Log("Reading: %s\n", path);

    SDL_RWops* file_to_read_ops = SDL_RWFromFile(path, "rt");

    if (file_to_read_ops != NULL) {
        SDL_RWseek(file_to_read_ops, 0, RW_SEEK_END);
        size_t file_size = SDL_RWtell(file_to_read_ops);
        SDL_RWseek(file_to_read_ops, 0, RW_SEEK_SET);

        char* file_contents = (char*)malloc(file_size + 1);

        if (file_contents != NULL) {
            size_t read_size = SDL_RWread(file_to_read_ops, file_contents, 1, file_size);
            assert(read_size <= file_size);

            file_contents[read_size] = 0x00;
            SDL_RWclose(file_to_read_ops);
        }

        return (File_Data) {
            file_contents,
            file_size
        };
    }

    return (File_Data) {
        (char*)calloc(1, sizeof(char)),
        1
    };
}

#define array_fill_MACRO(arr, val)                                                            \
    {                                                                                         \
        for (size_t arr_index = 0; arr_index < (sizeof(arr) / sizeof(arr[0])); arr_index++) { \
            arr[arr_index] = val;                                                             \
        }                                                                                     \
    }
