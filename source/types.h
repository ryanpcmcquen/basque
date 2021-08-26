#include "configuration.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

typedef struct {
    SDL_Renderer* renderer;
    SDL_Window* window;
} App;

typedef struct {
    int x;
    int y;
} Axes;

typedef struct {
    float x;
    float y;
} Precise_Axes;

enum directions {
    NORTH = 0,
    EAST,
    SOUTH,
    WEST
};

typedef struct {
    int north;
    int east;
    int south;
    int west;
} Directions;

typedef struct {
    int north[2];
    int east[2];
    int south[2];
    int west[2];
} Direction_Bounds;

typedef struct {
    Axes window;
    Axes global;
    Axes movement;

    Directions can_move;

    Direction_Bounds bounds;

    int direction;
} Player;

typedef struct {
    Axes clip;

    // This goes North, East, South, and West
    // (in increasing order by degree).
    Directions border;
} Tile_Data;

typedef struct {
    char* contents;
    size_t length;
} File_Data;

typedef struct {
    int rows;
    int columns;
    int* columns_in_row;

    int** layout;
    char* layout_file_base;
    char* layout_file;
    time_t layout_modified_time;
    File_Data layout_string;

    char* attributes_file;
    time_t attributes_modified_time;
    File_Data attributes_string;
    Tile_Data tile_attributes[TILE_ATTRIBUTES_LIMIT];

    int total_parsed_attributes;
} Map;

typedef struct {
    SDL_Surface text_surfaces[TILE_ATTRIBUTES_LIMIT];
    SDL_Texture* text_textures[TILE_ATTRIBUTES_LIMIT];
    int layout_file_suffix;
    int selected_tile;
} Editor;

typedef struct {
    TTF_Font* face;
    TTF_Font* outline;

    SDL_Color color;
    SDL_Color outline_color;

    SDL_Surface* outline_surface;
    SDL_Surface* surface;
    SDL_Rect rect;
} Font;

typedef struct {
    Axes scroll;

    SDL_bool done;

    Player player;

    SDL_Texture* player_image;
    SDL_Texture* background_image;

    Map map;

    Mix_Music* music;

    Font font;
    bool EDIT_MODE;

    Editor editor;
} Game;
