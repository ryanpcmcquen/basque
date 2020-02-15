#include "map.h"
#include <SDL2/SDL_image.h>

SDL_Texture* load_texture(App* app, char* file)
{
    SDL_Texture* texture;
    texture = IMG_LoadTexture(app->renderer, file);

    return texture;
}

void load_music(GameState* game)
{
    int flags = MIX_INIT_OGG;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags) {
        printf("Mix_Init: Failed to init required ogg support!\n");
        printf("Mix_Init: %s\n", Mix_GetError());
        // We can still continue without audio. :^(
    } else {

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0) {
            Mix_VolumeMusic(MUSIC_VOLUME);
            game->music = Mix_LoadMUS(MUSIC_INTRO_FILE);
            if (!Mix_PlayingMusic()) {
                Mix_PlayMusic(game->music, -1);
            }
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error initializing SDL_mixer: %s\n", Mix_GetError());
            // No need to exit, we just play without sound.
            //exit(1);
        }
    }
}

void prepare_scene(App* app, GameState* game)
{
    game->player_image = load_texture(app, PLAYER_IMAGE);
    game->background_image = load_texture(app, BACKGROUND_IMAGE);
    load_music(game);
}

void sprite_blit(App* app, SDL_Texture* texture, int x, int y, int direction)
{
    SDL_Rect src;
    src.x = x;
    src.y = y;

    SDL_Rect dest;

    dest.x = x;
    dest.y = y;
    dest.h = PLAYER_SPRITE_HEIGHT;
    dest.w = PLAYER_SPRITE_WIDTH;

    SDL_Rect clip;
    clip.x = x;
    clip.y = y;
    x = abs(x % 4);
    y = abs(y % 4);
    switch (direction) {
    case NORTH: {
        clip.x = y * PLAYER_SPRITE_COLUMN_WIDTH;
        clip.y = PLAYER_SPRITE_ROW_HEIGHT * 2;
    } break;

    case EAST: {
        clip.x = x * PLAYER_SPRITE_COLUMN_WIDTH;
        clip.y = PLAYER_SPRITE_ROW_HEIGHT;
    } break;

    case SOUTH: {
        clip.x = y * PLAYER_SPRITE_COLUMN_WIDTH;
        clip.y = 0;
    } break;

    case WEST: {
        clip.x = x * PLAYER_SPRITE_COLUMN_WIDTH;
        clip.y = PLAYER_SPRITE_ROW_HEIGHT * 3;
    } break;
    }

    clip.h = PLAYER_SPRITE_HEIGHT;
    clip.w = PLAYER_SPRITE_WIDTH;

    SDL_QueryTexture(texture, NULL, NULL, &src.w, &src.h);

    SDL_RenderCopy(app->renderer, texture, &clip, &dest);
}

void present_scene(App* app)
{
    SDL_RenderPresent(app->renderer);
    SDL_RenderClear(app->renderer);
}

int is_below_bound(int coordinate, int sprite_dimension, int bound)
{
    return (coordinate + sprite_dimension) < bound;
}
int is_above_bound(int coordinate, int sprite_dimension, int bound)
{
    return (coordinate + sprite_dimension) > bound;
}

void handle_collisions(GameState* game)
{
    if (DEBUG_MODE && game->EDIT_MODE) {
        game->player.can_move.north = true;
        game->player.can_move.east = true;
        game->player.can_move.south = true;
        game->player.can_move.west = true;

    } else {
        game->player.can_move.north = false;
        game->player.can_move.east = false;
        game->player.can_move.south = false;
        game->player.can_move.west = false;

        if (is_above_bound(game->player.global.y, PLAYER_SPRITE_ROW_HEIGHT / 2, game->player.bound.north)) {
            game->player.can_move.north = true;
        }
        if (is_below_bound(game->player.global.x, PLAYER_SPRITE_WIDTH, game->player.bound.east)) {
            game->player.can_move.east = true;
        }
        if (is_below_bound(game->player.global.y, PLAYER_SPRITE_ROW_HEIGHT, game->player.bound.south)) {
            game->player.can_move.south = true;
        }
        if (is_above_bound(game->player.global.x, 0, game->player.bound.west)) {
            game->player.can_move.west = true;
        }
    }
}

int write_map_layout(GameState* game)
{

    if (file_exists(MAP_LOCK_FILE)) {
        // Do not write if there is a lock file, to avoid obliterating the map.
        // @TODO:
        // Would be nice to give the user some feedback, if there was a lock.
        // SDL_SetRenderDrawColor(app->renderer, 250, 25, 0, 150);
        return 0;
    } else {
        char* new_map_layout_file = (char*)calloc(strlen(game->map.layout_file) + 4, sizeof(char));
        snprintf(new_map_layout_file, sizeof(char) * (strlen(game->map.layout_file) + 4), "%s_%i%s", game->map.layout_file_base, game->editor.layout_file_suffix, TXT_EXTENSION);
        new_map_layout_file[strlen(new_map_layout_file)] = '\0';

        FILE* new_map_layout;
        FILE* old_map_layout;
        FILE* map_lock;
#if defined(__WIN32__) || defined(__WINRT__) || defined(_WIN64)
        fopen_s(&new_map_layout, new_map_layout_file, "wb");
        fopen_s(&old_map_layout, game->map.layout_file, "wb");
        fopen_s(&map_lock, MAP_LOCK_FILE, "wb");
#else
        new_map_layout = fopen(new_map_layout_file, "wb");
        old_map_layout = fopen(game->map.layout_file, "wb");
        map_lock = fopen(MAP_LOCK_FILE, "wb");
#endif
        // @FIX:
        // This check doesn't work.
        // if (new_map_layout == NULL || old_map_layout == NULL) {
        //     SDL_LogError(SDL_LOG_CATEGORY_ERROR, "ERROR: File failed to open.");
        //     exit(1);
        // }
        char* new_map_str = (char*)calloc(game->map.rows * game->map.columns, sizeof(char));
        int new_map_str_counter = 0;

        for (int row = 0; row < game->map.rows; row++) {

            if (game->map.columns_in_row[row] > game->map.columns) {
                break;
            }

            if (game->map.columns_in_row[row] < 1) {
                new_map_str[new_map_str_counter] = '\n';
                new_map_str_counter++;
                continue;
            }
            for (int column = 0; column < game->map.columns_in_row[row]; column++) {

                char tmp[TILE_CHAR_LIMIT] = { 0 };
                // Convert integer to string:
                if (game->map.layout[row][column] == EMPTY_COLUMN) {
                    tmp[0] = ' ';
                    tmp[1] = ' ';
                    tmp[2] = '_';
                } else {
                    snprintf(tmp, sizeof(tmp), "%3i", game->map.layout[row][column]);
                }
                int tmp_length = strlen(tmp);

                for (int tmp_counter = 0; tmp_counter < tmp_length; tmp_counter++) {
                    // Not sure I need this, but it definitely breaks everything:
                    // if (new_map_str[new_map_str_counter] == '\0') {
                    // break;
                    // } else {
                    new_map_str[new_map_str_counter] = tmp[tmp_counter];
                    new_map_str_counter++;
                    // }
                }

                // Do not write a comma for the last column:
                if ((column + 1) != game->map.columns_in_row[row]) {
                    new_map_str[new_map_str_counter] = ',';
                    new_map_str_counter++;
                }
            }

            // This is necessary!
            new_map_str[new_map_str_counter] = '\n';
            new_map_str_counter++;
        }

        // @Bug:
        // If the last three characters are not newlines, we get
        // a corrupted size vs. prev size error here.
        // This should be investigated, obviously
        // the parser has a logical error.
        int new_map_str_length = strlen(new_map_str);
        SDL_Log("%c %c %c %c \n", new_map_str[new_map_str_length - 3], new_map_str[new_map_str_length - 2], new_map_str[new_map_str_length - 1], new_map_str[new_map_str_length]);

        int chars_written = fwrite(new_map_str, sizeof(char), new_map_str_length, new_map_layout);
        if (chars_written != new_map_str_length) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Only %i chars written out of: %i\n", chars_written, new_map_str_length);
        } else {
            if (game->editor.layout_file_suffix == MAP_LAYOUT_FILE_LIMIT) {
                game->editor.layout_file_suffix = 0;
            } else {
                game->editor.layout_file_suffix++;
            }
        }
        fclose(new_map_layout);

        chars_written = fwrite(new_map_str, sizeof(char), new_map_str_length, old_map_layout);
        if (chars_written != new_map_str_length) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Only %i chars written out of: %i\n", chars_written, new_map_str_length);
        }
        fclose(old_map_layout);

        free(new_map_str);
        free(new_map_layout_file);

        // If all was successful, remove lock file:
        fclose(map_lock);
        remove(MAP_LOCK_FILE);

        return 1;
    }
}

int next_cycle;

int time_left()
{
    int now;

    now = SDL_GetTicks();
    // SDL_Log("now: %i\n", now);
    // SDL_Log("next_cycle: %i\n", next_cycle);
    if (next_cycle <= now) {
        return 0;
    } else {
        return next_cycle - now;
    }
}

void handle_input(App* app, GameState* game)
{
    game->done = SDL_FALSE;

    game->player.window.x = 0;
    game->player.window.y = 0;
    game->player.global.x = 0;
    game->player.global.y = 0;

    game->player.direction = SOUTH;

    int current_tile_y = 0;
    int current_tile_x = 0;
    int current_tile = 0;

    next_cycle = SDL_GetTicks() + FRAME_INTERVAL_REDUCTION;

    while (!game->done) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (strcmp(game->map.layout_file, MAP_LAYOUT_FILE) == 0) {
                handle_collisions(game);

                current_tile_y = (game->player.global.y + PLAYER_SPRITE_HEIGHT) / TILE_SPRITE_HEIGHT;
                current_tile_x = (game->player.global.x + (PLAYER_SPRITE_COLUMN_WIDTH / 2)) / TILE_SPRITE_WIDTH;

                if (current_tile_y < 0) {
                    current_tile_y = 0;
                }
                if (current_tile_x < 0) {
                    current_tile_x = 0;
                }

                // @Robustness:
                // Should we handle the exception case here where the current
                // tile is not a valid tile?
                if (game->map.layout[current_tile_y][current_tile_x] >= 0) {
                    current_tile = game->map.layout[current_tile_y][current_tile_x];
                } else {
                    current_tile = 0;
                }
            } else {
                current_tile = 0;
            }
            if (current_tile < 0) {
                current_tile = 0;
            }
            Tile_Data current_tile_attributes = game->map.tile_attributes[current_tile];

            switch (event.type) {
            case SDL_QUIT: {
                game->done = SDL_TRUE;

            } break;

            case SDL_KEYDOWN: {
                // SDL_Log("Global x: %i\n", game->player.global.x);
                // SDL_Log("Player x: %i\n", game->player.window.x);
                // SDL_Log("Global y: %i\n", game->player.global.y);
                // SDL_Log("Player y: %i\n", game->player.window.y);

                switch (event.key.keysym.sym) {

                case SDLK_UP: {
                    int tile_coordinate_y = current_tile_y * TILE_SPRITE_HEIGHT;
                    game->player.bound.north = 0;
                    if (current_tile_attributes.border.north > 0) {
                        game->player.bound.north = tile_coordinate_y + current_tile_attributes.border.north;
                    }
                    if (game->player.can_move.north) {
                        game->player.window.y = game->player.window.y - PLAYER_INCREMENT;
                        game->player.global.y = game->player.global.y - GLOBAL_INCREMENT;
                    }

                    game->player.direction = NORTH;
                } break;

                case SDLK_RIGHT: {
                    int tile_coordinate_x = (current_tile_x + 1) * TILE_SPRITE_WIDTH;
                    game->player.bound.east = SCREEN_WIDTH;
                    if (current_tile_attributes.border.east > 0) {
                        game->player.bound.east = tile_coordinate_x - current_tile_attributes.border.east;
                    }

                    if (game->player.can_move.east) {
                        game->player.window.x = game->player.window.x + PLAYER_INCREMENT;
                        game->player.global.x = game->player.global.x + GLOBAL_INCREMENT;
                    }

                    game->player.direction = EAST;
                } break;

                case SDLK_DOWN: {
                    int tile_coordinate_y = (current_tile_y + 1) * TILE_SPRITE_HEIGHT;
                    game->player.bound.south = SCREEN_HEIGHT;
                    if (current_tile_attributes.border.south > 0) {
                        game->player.bound.south = tile_coordinate_y - current_tile_attributes.border.south;
                    }

                    if (game->player.can_move.south) {
                        game->player.window.y = game->player.window.y + PLAYER_INCREMENT;
                        game->player.global.y = game->player.global.y + GLOBAL_INCREMENT;
                    }

                    game->player.direction = SOUTH;
                } break;

                case SDLK_LEFT: {
                    int tile_coordinate_x = current_tile_x * TILE_SPRITE_WIDTH;
                    game->player.bound.west = 0;
                    if (current_tile_attributes.border.west > 0) {
                        game->player.bound.west = tile_coordinate_x + current_tile_attributes.border.west;
                    }
                    if (game->player.can_move.west) {
                        game->player.window.x = game->player.window.x - PLAYER_INCREMENT;
                        game->player.global.x = game->player.global.x - GLOBAL_INCREMENT;
                    }

                    game->player.direction = WEST;
                } break;

                case SDLK_q: {
                    game->done = SDL_TRUE;
                } break;

                case SDLK_e: {
                    if (DEBUG_MODE) {
                        if (game->EDIT_MODE) {
                            game->EDIT_MODE = false;
                        } else {
                            game->EDIT_MODE = true;
                        }
                    }
                } break;

                case SDLK_l: {
                    if (DEBUG_MODE && game->EDIT_MODE) {
                        if (strcmp(game->map.layout_file, MAP_LIBRARY_FILE) != 0) {
                            // Load map_library.txt:
                            game->map.layout_file = MAP_LIBRARY_FILE;
                            read_map_layout(game);
                        } else {
                            game->map.layout_file = MAP_LAYOUT_FILE;
                            read_map_layout(game);
                        }
                    }
                } break;
                case SDLK_RETURN: {

                    // @TODO:
                    // Accept input of a number, which is the id
                    // of the tile you want to place on left
                    // mouse click.

                    // @Idea:
                    // For now, we could just use enter,
                    // to display a linearly increased
                    // example of all available tiles.
                    // Doing so, would allow us to
                    // use the right mouse button
                    // to select from all tiles.

                } break;
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                if (DEBUG_MODE && game->EDIT_MODE) {

                    Axes mouse;
                    // @Weirdness:
                    // Player x and y is not affected by scaling, but the mouse is.
                    // Why?!
                    if (strcmp(game->map.layout_file, MAP_LIBRARY_FILE) == 0) {
                        mouse.x = event.button.x / SCALING;
                        mouse.y = event.button.y / SCALING;
                    } else {
                        mouse.x = (event.button.x / SCALING) + (game->player.global.x - game->player.window.x);
                        mouse.y = (event.button.y / SCALING) + (game->player.global.y - game->player.window.y);
                    }

                    // SDL_Log("Global x: %i\n", game->player.global.x);
                    // SDL_Log("Player x: %i\n", game->player.window.x);
                    // SDL_Log("Mouse x: %i\n", mouse.x);
                    // SDL_Log("Global y: %i\n", game->player.global.y);
                    // SDL_Log("Player y: %i\n", game->player.window.y);
                    // SDL_Log("Mouse y: %i\n", mouse.y);

                    Axes mouse_tile;
                    mouse_tile.x = mouse.x / TILE_SPRITE_WIDTH;
                    mouse_tile.y = mouse.y / TILE_SPRITE_WIDTH;
                    int selected_tile = EMPTY_COLUMN;

                    if (mouse_tile.y >= 0 && mouse_tile.y < game->map.rows && mouse_tile.x >= 0 && mouse_tile.x < game->map.columns) {
                        selected_tile = game->map.layout[mouse_tile.y][mouse_tile.x];
                    }
                    // @HACK:
                    // This feels kludgy, improve it.
                    if (selected_tile < 0) {
                        selected_tile = EMPTY_COLUMN;
                    }

                    switch (event.button.button) {
                    case SDL_BUTTON_LEFT: {
                        if (strcmp(game->map.layout_file, MAP_LAYOUT_FILE) == 0) {
                            // If we are trying to increase beyond the current
                            // allocated vertical space ...
                            if (mouse_tile.y >= game->map.rows) {
                                // If the user clicks way out in space, just add one row, not several:
                                mouse_tile.y = game->map.rows;
                                game->map.rows = mouse_tile.y;

                                game->map.columns_in_row[mouse_tile.y] = 0;
                            }

                            // If the row is completely empty, fill everything up to
                            // mouse_tile.x with EMPTY_COLUMN.
                            if (game->map.columns_in_row[mouse_tile.y] == 0) {
                                for (int columns_to_fill = 0; columns_to_fill < mouse_tile.x; columns_to_fill++) {
                                    game->map.layout[mouse_tile.y][columns_to_fill] = EMPTY_COLUMN;
                                }
                            }

                            game->map.layout[mouse_tile.y][mouse_tile.x] = game->editor.selected_tile;
                            // Add a newline if this is the only column in the row ...
                            // or the last column, in the row.
                            if ((mouse_tile.x + 1) >= game->map.columns_in_row[mouse_tile.y]) {
                                // While the columns in the row are less than where the mouse clicked,
                                // fill those columns with the selected tile.
                                while (game->map.columns_in_row[mouse_tile.y] < (mouse_tile.x + 1)) {
                                    game->map.layout[mouse_tile.y][game->map.columns_in_row[mouse_tile.y]] = game->editor.selected_tile;
                                    game->map.columns_in_row[mouse_tile.y] = game->map.columns_in_row[mouse_tile.y] + 1;
                                }

                                game->map.layout[mouse_tile.y][mouse_tile.x + 1] = '\n';
                                game->map.columns_in_row[mouse_tile.y] = mouse_tile.x + 1;
                            }
                            // If this row has more columns than any other column, increase
                            // the global column count.
                            if (game->map.columns_in_row[mouse_tile.y] > game->map.columns) {
                                game->map.columns = game->map.columns_in_row[mouse_tile.y];
                            }
                            // SDL_Log("Left click: %i\n", game->map.layout[mouse_tile.y][mouse_tile.x]);
                            if (write_map_layout(game)) {
                                SDL_Log("Write successful.\n");
                            }
                        }
                    } break;
                    case SDL_BUTTON_RIGHT: {
                        game->editor.selected_tile = selected_tile;
                        SDL_Log("Selected tile: %i\n", selected_tile);
                    } break;
                    }
                }
            } break;
            }

            // @TODO:
            // It is time to rework scrolling ...
            //
            // We probably want to have the player near the center of the screen
            // at all times. Perhaps, we could do segmented scrolling, where
            // scrolling happens once the player reaches some bounding
            // rectangle that represents an arbitrary percentage of
            // the screen (for example, 40%).
            if (game->player.window.x < SCREEN_WIDTH) {
                game->scroll.x = -(game->player.window.x * 2);
            }
            if (game->player.window.y < SCREEN_HEIGHT) {
                game->scroll.y = -(game->player.window.y * 2);
            }

            generate_map(app, game);
            if (strcmp(game->map.layout_file, MAP_LAYOUT_FILE) == 0) {
                sprite_blit(app, game->player_image, game->player.window.x, game->player.window.y, game->player.direction);
            }

            present_scene(app);

            // @Bug:
            // This is a bit broken now.
            // Cap frame rate:
            // SDL_Delay(time_left());
            // next_cycle += FRAME_INTERVAL_REDUCTION;

            // SDL_Log("%i\n", time_left());
            // SDL_Log("%i\n", next_cycle);

            // Allegedly this keeps the app from running at too
            // high of a frame rate. We also VSync.
            SDL_Delay(GAME_LOOP_DELAY);
        }
    }
}
