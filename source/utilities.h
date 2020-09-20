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
    FILE* file_to_read;
#ifdef PLATFORM_IS_WINDOWS
    fopen_s(&file_to_read, path, "rb");
#else
    file_to_read = fopen(path, "rb");
#endif
    if (file_to_read != NULL) {
        fseek(file_to_read, 0, SEEK_END);
        size_t file_size = ftell(file_to_read);
        fseek(file_to_read, 0, SEEK_SET);

        char* file_contents = (char*)malloc(file_size + 1);

        if (file_contents != NULL) {
            size_t read_size = fread(file_contents, 1, file_size, file_to_read);
            assert(read_size <= file_size);
            file_contents[read_size] = '\0';
            fclose(file_to_read);
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
