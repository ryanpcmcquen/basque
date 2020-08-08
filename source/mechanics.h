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

#define bounds_check_MACRO(bounds_array, coordinate, sprite_dimension, is_within_bound, can_move_direction)           \
    {                                                                                                                 \
        for (size_t bound_index = 0; bound_index < (sizeof(bounds_array) / sizeof(bounds_array[0])); bound_index++) { \
            if (is_within_bound(coordinate, sprite_dimension, bounds_array[bound_index])) {                           \
                can_move_direction = true;                                                                            \
            } else {                                                                                                  \
                can_move_direction = false;                                                                           \
                break;                                                                                                \
            }                                                                                                         \
        }                                                                                                             \
    }

void handle_collisions(GameState* game)
{
    if (DEBUG_MODE && game->EDIT_MODE) {

        game->player.can_move.north = true;
        game->player.can_move.east = true;
        game->player.can_move.south = true;
        game->player.can_move.west = true;

    } else {
        bounds_check_MACRO(game->player.bounds.north, game->player.global.y - PLAYER_INCREMENT, PLAYER_SPRITE_ROW_HEIGHT / 2, is_above_bound, game->player.can_move.north);
        bounds_check_MACRO(game->player.bounds.east, game->player.global.x + PLAYER_INCREMENT, PLAYER_SPRITE_WIDTH, is_below_bound, game->player.can_move.east);
        bounds_check_MACRO(game->player.bounds.south, game->player.global.y + PLAYER_INCREMENT, PLAYER_SPRITE_ROW_HEIGHT, is_below_bound, game->player.can_move.south);
        bounds_check_MACRO(game->player.bounds.west, game->player.global.x - PLAYER_INCREMENT, 0, is_above_bound, game->player.can_move.west);
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

        char* new_map_layout_file = (char*)calloc(strlen(game->map.layout_file) + 4 + 1, sizeof(char));
        snprintf(new_map_layout_file, sizeof(char) * (strlen(game->map.layout_file) + 4), "%s_%i%s", game->map.layout_file_base, game->editor.layout_file_suffix, TXT_EXTENSION);

        FILE* new_map_layout;
        FILE* old_map_layout;
        FILE* map_lock;
#ifdef PLATFORM_IS_WINDOWS
        fopen_s(&new_map_layout, new_map_layout_file, "wb");
        fopen_s(&old_map_layout, game->map.layout_file, "wb");
        fopen_s(&map_lock, MAP_LOCK_FILE, "wb");
#else
        new_map_layout = fopen(new_map_layout_file, "wb");
        old_map_layout = fopen(game->map.layout_file, "wb");
        map_lock = fopen(MAP_LOCK_FILE, "wb");
#endif

        char* new_map_str = (char*)calloc((game->map.rows * game->map.columns) + 1, sizeof(char));
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
                size_t tmp_length = strlen(tmp);

                for (size_t tmp_counter = 0; tmp_counter < tmp_length; tmp_counter++) {
                    new_map_str[new_map_str_counter] = tmp[tmp_counter];
                    new_map_str_counter++;
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

        size_t new_map_str_length = strlen(new_map_str);

        size_t chars_written = fwrite(new_map_str, sizeof(char), new_map_str_length, new_map_layout);
        // if (chars_written != new_map_str_length) {
        //     SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Only %zu chars written out of: %zu\n", chars_written, new_map_str_length);
        // } else {
        if (game->editor.layout_file_suffix == MAP_LAYOUT_FILE_LIMIT) {
            game->editor.layout_file_suffix = 0;
        } else {
            game->editor.layout_file_suffix++;
        }
        // }
        fclose(new_map_layout);

        chars_written = fwrite(new_map_str, sizeof(char), new_map_str_length, old_map_layout);
        // if (chars_written != new_map_str_length) {
        //     SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Only %zu chars written out of: %zu\n", chars_written, new_map_str_length);
        // }
        fclose(old_map_layout);

        free(new_map_layout_file);
        free(new_map_str);

        // If all was successful, remove the lock file:
        fclose(map_lock);
        remove(MAP_LOCK_FILE);

        return 1;
    }
}

#define move_player_MACRO(can_move_direction, window, window_increment, global, global_increment) \
    {                                                                                             \
        if (can_move_direction) {                                                                 \
            window = window + window_increment;                                                   \
            global = global + global_increment;                                                   \
        }                                                                                         \
    }

#define set_next_tile_MACRO(next_tile, next_tile_attributes, game)              \
    {                                                                           \
        if (next_tile >= 0 && next_tile <= game->map.total_parsed_attributes) { \
            next_tile_attributes = game->map.tile_attributes[next_tile];        \
        }                                                                       \
    }

void handle_input(App* app, GameState* game)
{
    game->done = SDL_FALSE;

    game->player.window.x = 0, game->player.window.y = 0, game->player.global.x = 0, game->player.global.y = 0;
    game->player.direction = SOUTH;

    int current_tile_y = 0, current_tile_x = 0, current_tile = 0;

    // TODO:
    // If the player is very close to a tile split line (maybe within 2 pixels),
    // we also need to look at the tile above and below where the player is
    // when determining collisions.
    /*
       One idea is to see if the player's global coordinate remainder when
       when divided by TILE_SPRITE_(HEIGHT/WIDTH) is 2 or less, then we
       need to also include one more tile along the perpendicular axis.
    */

    int next_tile_north = 0, next_tile_north_y = 0, next_tile_east = 0, next_tile_east_x = 0, next_tile_south = 0, next_tile_south_y = 0, next_tile_west = 0, next_tile_west_x = 0;

    int prev_time = 0;

    while (!game->done) {
        SDL_Event event;

        handle_collisions(game);

        if (game->player.window.x < SCREEN_WIDTH) {
            game->scroll.x = -(game->player.window.x * (int)INITIAL_SCALING);
        }
        if (game->player.window.y < SCREEN_HEIGHT) {
            game->scroll.y = -(game->player.window.y * (int)INITIAL_SCALING);
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                game->done = SDL_TRUE;

            } break;

            case SDL_KEYDOWN: {

                switch (event.key.keysym.sym) {

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
                        } else {
                            game->map.layout_file = MAP_LAYOUT_FILE;
                        }
                        read_map_layout(game);
                    }
                } break;

                    // case SDLK_p: {
                    //     if (SCALING < 10.0) {

                    //         SCALING++;
                    //         SDL_RenderSetScale(app->renderer, SCALING, SCALING);
                    //     }
                    // } break;

                    // case SDLK_m: {
                    //     if (SCALING > 1.0) {

                    //         SCALING--;
                    //         SDL_RenderSetScale(app->renderer, SCALING, SCALING);
                    //     }
                    // } break;
                }
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                if (DEBUG_MODE && game->EDIT_MODE) {

                    Precise_Axes mouse;
                    // @Weirdness:
                    // Player x and y is not affected by scaling, but the mouse is.
                    // Why?!
                    if (strcmp(game->map.layout_file, MAP_LIBRARY_FILE) == 0) {
                        mouse.x = (float)event.button.x / SCALING;
                        mouse.y = (float)event.button.y / SCALING;
                    } else {
                        mouse.x = ((float)event.button.x / SCALING) + (game->player.global.x - game->player.window.x);
                        mouse.y = ((float)event.button.y / SCALING) + (game->player.global.y - game->player.window.y);
                    }

                    Axes mouse_tile;
                    mouse_tile.x = (int)(mouse.x / TILE_SPRITE_WIDTH);
                    mouse_tile.y = (int)(mouse.y / TILE_SPRITE_WIDTH);
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
                            if (write_map_layout(game)) {
                                SDL_Log("Write %i successful.\n", game->editor.layout_file_suffix);
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
        }

        if (strcmp(game->map.layout_file, MAP_LAYOUT_FILE) == 0) {

            current_tile_y = (game->player.global.y + PLAYER_SPRITE_HEIGHT) / TILE_SPRITE_HEIGHT;
            current_tile_x = (game->player.global.x + (PLAYER_SPRITE_COLUMN_WIDTH / 2)) / TILE_SPRITE_WIDTH;

            next_tile_north_y = current_tile_y - 1;
            next_tile_east_x = current_tile_x + 1;
            next_tile_south_y = current_tile_y + 1;
            next_tile_west_x = current_tile_x - 1;

            if (current_tile_y < 0 || current_tile_y == 0) {
                current_tile_y = 0;
            }
            if (current_tile_x < 0 || current_tile_x == 0) {
                current_tile_x = 0;
            }

            // @Robustness:
            // Should we handle the exception case here where the current
            // tile is not a valid tile?
            if (game->map.layout[current_tile_y][current_tile_x] >= 0) {
                current_tile = game->map.layout[current_tile_y][current_tile_x];

                next_tile_north = next_tile_north_y > 0 ? game->map.layout[next_tile_north_y][current_tile_x] : 0;
                next_tile_east = next_tile_east_x > 0 ? game->map.layout[current_tile_y][next_tile_east_x] : 0;
                next_tile_south = next_tile_south_y > 0 ? game->map.layout[next_tile_south_y][current_tile_x] : 0;
                next_tile_west = next_tile_west_x > 0 ? game->map.layout[current_tile_y][next_tile_west_x] : 0;
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

        Tile_Data next_tile_north_attributes = game->map.tile_attributes[current_tile];
        set_next_tile_MACRO(next_tile_north, next_tile_north_attributes, game);

        Tile_Data next_tile_east_attributes = game->map.tile_attributes[current_tile];
        set_next_tile_MACRO(next_tile_east, next_tile_east_attributes, game);

        Tile_Data next_tile_south_attributes = game->map.tile_attributes[current_tile];
        set_next_tile_MACRO(next_tile_south, next_tile_south_attributes, game);

        Tile_Data next_tile_west_attributes = game->map.tile_attributes[current_tile];
        set_next_tile_MACRO(next_tile_west, next_tile_west_attributes, game);

        generate_map(app, game);
        if (strcmp(game->map.layout_file, MAP_LAYOUT_FILE) == 0) {
            sprite_blit(app, game->player_image, game->player.window.x, game->player.window.y, game->player.direction);
        }

        int curr_time = SDL_GetTicks();
        int time_elapsed = curr_time - prev_time;

        if (time_elapsed < MIN_FRAMETIME_MSECS) {
            // Not enough time has elapsed. Let's limit the frame rate!
            SDL_Delay(MIN_FRAMETIME_MSECS - time_elapsed);
            curr_time = SDL_GetTicks();
            time_elapsed = curr_time - prev_time;
        }
        prev_time = curr_time;

        const Uint8* current_key_states = SDL_GetKeyboardState(NULL);

        //
        // NORTH:
        //
        if (current_key_states[SDL_SCANCODE_UP]) {
            int tile_coordinate_y = current_tile_y * TILE_SPRITE_HEIGHT;
            int next_tile_north_coordinate_y = next_tile_north_y * TILE_SPRITE_WIDTH;

            array_fill_MACRO(game->player.bounds.north, 0);

            if (current_tile_attributes.border.north > 0) {
                game->player.bounds.north[0] = tile_coordinate_y + current_tile_attributes.border.north;
            }
            if (next_tile_north_attributes.border.south > 0) {
                game->player.bounds.north[1] = next_tile_north_coordinate_y + next_tile_north_attributes.border.north;
            }

            move_player_MACRO(game->player.can_move.north, game->player.window.y, -PLAYER_INCREMENT, game->player.global.y, -GLOBAL_INCREMENT);
            game->player.direction = NORTH;
        }

        //
        // EAST:
        //
        if (current_key_states[SDL_SCANCODE_RIGHT]) {
            int tile_coordinate_x = (current_tile_x + 1) * TILE_SPRITE_WIDTH;
            int next_tile_east_coordinate_x = (next_tile_east_x + 1) * TILE_SPRITE_WIDTH;

            array_fill_MACRO(game->player.bounds.east, game->map.columns * TILE_SPRITE_WIDTH);

            if (current_tile_attributes.border.east > 0) {
                game->player.bounds.east[0] = tile_coordinate_x - current_tile_attributes.border.east;
            }

            if (next_tile_east_attributes.border.east > 0) {
                game->player.bounds.east[1] = next_tile_east_coordinate_x - next_tile_east_attributes.border.east;
            }

            move_player_MACRO(game->player.can_move.east, game->player.window.x, PLAYER_INCREMENT, game->player.global.x, GLOBAL_INCREMENT);
            game->player.direction = EAST;
        }

        //
        // SOUTH:
        //
        if (current_key_states[SDL_SCANCODE_DOWN]) {
            int tile_coordinate_y = (current_tile_y + 1) * TILE_SPRITE_HEIGHT;
            int next_tile_south_coordinate_y = (next_tile_south_y + 1) * TILE_SPRITE_HEIGHT;

            array_fill_MACRO(game->player.bounds.south, game->map.rows * TILE_SPRITE_HEIGHT);

            if (current_tile_attributes.border.south > 0) {
                game->player.bounds.south[0] = tile_coordinate_y - current_tile_attributes.border.south;
            }
            if (next_tile_south_attributes.border.south > 0) {
                game->player.bounds.south[1] = next_tile_south_coordinate_y - next_tile_south_attributes.border.south;
            }

            move_player_MACRO(game->player.can_move.south, game->player.window.y, PLAYER_INCREMENT, game->player.global.y, GLOBAL_INCREMENT);
            game->player.direction = SOUTH;
        }

        //
        // WEST:
        //
        if (current_key_states[SDL_SCANCODE_LEFT]) {
            int tile_coordinate_x = current_tile_x * TILE_SPRITE_WIDTH;
            int next_tile_west_coordinate_x = next_tile_west_x * TILE_SPRITE_WIDTH;

            array_fill_MACRO(game->player.bounds.west, 0);

            if (current_tile_attributes.border.west > 0) {
                game->player.bounds.west[0] = tile_coordinate_x + current_tile_attributes.border.west;
            }
            if (next_tile_west_attributes.border.west > 0) {
                game->player.bounds.west[1] = next_tile_west_coordinate_x + next_tile_west_attributes.border.west;
            }

            move_player_MACRO(game->player.can_move.west, game->player.window.x, -PLAYER_INCREMENT, game->player.global.x, -GLOBAL_INCREMENT);
            game->player.direction = WEST;
        }

        // SDL_Log("Global x: %i\n", game->player.global.x);
        // SDL_Log("Player x: %i\n", game->player.window.x);
        // SDL_Log("Global y: %i\n", game->player.global.y);
        // SDL_Log("Player y: %i\n", game->player.window.y);

        present_scene(app);
    }
}
