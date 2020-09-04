#include "utilities.h"

bool map_char_is_tile_info(char char_to_check)
{
    return char_to_check != ' ' && char_to_check != '\0' && char_to_check != '\n' && char_to_check != ',';
}

bool map_char_is_not_tile_info(char char_to_check)
{
    return char_to_check == ' ' || char_to_check == '\0' || char_to_check == '\n' || char_to_check == ',';
}

void read_map_layout(Game* game)
{
    game->map.layout_string = read_file(game->map.layout_file);

    if (game->map.layout_string.length < 1) {
        SDL_Log("Map layout is empty, exiting.\n");
        exit(EXIT_FAILURE);
    }

    int current_column = 0;
    game->map.rows = 0, game->map.columns = 0;

    // @Fastness:
    // See if there is a faster way to do this.
    for (size_t i = 0; i < game->map.layout_string.length; i++) {
        if (game->map.layout_string.contents[i] == '\n') {
            if (current_column > game->map.columns) {
                game->map.columns = current_column;
            }
            game->map.rows++;
            current_column = 0;
        } else {
            current_column++;
        }
    }

    // Map memory allocation:
    game->map.columns_in_row = (int*)calloc(game->map.rows, sizeof(int));
    game->map.layout = (int**)calloc(game->map.rows, sizeof(int*));
    for (int row = 0; row < game->map.rows; row++) {
        game->map.layout[row] = (int*)calloc(game->map.columns, sizeof(int));
    }
}

// TODO:
// Ask @strager if this makes sense as a macro.
#define get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter) \
    {                                                                            \
        attribute_counter = 0;                                                   \
        tmp_counter++;                                                           \
        while (tmp[tmp_counter] != ',') {                                        \
            attribute[attribute_counter] = tmp[tmp_counter];                     \
            attribute_counter++;                                                 \
            tmp_counter++;                                                       \
        }                                                                        \
    }

void read_map_attributes(Game* game)
{
    game->map.attributes_string = read_file(game->map.attributes_file);

    // @Fastness:
    // See if there is a faster way to do this.
    for (size_t i = 0; i < game->map.attributes_string.length; i++) {

        // Store our starting point, for times when we reverse
        // through the string (comments, et cetera).
        size_t original_i = i;

        // Cheap boolean so that we can skip comment lines:
        int skip_line = 0;

        char tmp[ATTRIBUTE_CHAR_LIMIT + 1] = { 0 };
        switch (game->map.attributes_string.contents[i]) {
        // case '\r': {
        //     SDL_Log("Please convert your map_*.txt files to use LF line endings ... exiting.\n");
        //     exit(EXIT_FAILURE);
        // } break;
        case ':': {
            // Start of tile.
            char tile_string[TILE_CHAR_LIMIT + 1] = { 0 };

            while (game->map.attributes_string.contents[i] != '\n') {
                if (game->map.attributes_string.contents[i] == '/') {
                    // Reset the counter!
                    i = original_i;
                    skip_line = 1;
                    break;
                }
                i--;
            }
            if (skip_line) {
                break;
            }
            // We don't need to read the newline character.
            i++;

            int tile_string_index = 0;
            while (game->map.attributes_string.contents[i] != ':') {
                tile_string[tile_string_index] = game->map.attributes_string.contents[i];
                tile_string_index++;
                i++;
            }
            tile_string[tile_string_index] = '\0';
            int tile_index = atoi(tile_string);
            if (tile_index > game->map.total_parsed_attributes) {
                game->map.total_parsed_attributes = tile_index;
            }

            // Advance twice here, because we are at the colon
            // and need to also go through the newline.
            i = i + 2;
            int attribute_index = 0;

            while (game->map.attributes_string.contents[i] != '\n') {
                switch (game->map.attributes_string.contents[i]) {
                case ' ':
                case '{':
                case '}': {
                    // Ignore.
                } break;
                default: {
                    tmp[attribute_index] = game->map.attributes_string.contents[i];
                    attribute_index++;
                } break;
                }
                i++;
            }

            tmp[attribute_index] = '\0';
            int tmp_counter = -1;

            char attribute[ATTRIBUTE_LENGTH + 1] = { 0 };

            int attribute_counter = 0;

            // X clip:
            get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter);
            int multiplier = 1;
            if (attribute[0] == '*') {
                multiplier = TILE_SPRITE_HEIGHT;
                memmove(attribute, &attribute[1], ATTRIBUTE_LENGTH);
            }
            game->map.tile_attributes[tile_index].clip.x = multiplier * atoi(attribute);

            // Y clip:
            get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter);
            multiplier = 1;
            if (attribute[0] == '*') {
                multiplier = TILE_SPRITE_HEIGHT;
                memmove(attribute, &attribute[1], ATTRIBUTE_LENGTH);
            }
            game->map.tile_attributes[tile_index].clip.y = multiplier * atoi(attribute);

            // North border:
            get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter);
            game->map.tile_attributes[tile_index].border.north = atoi(attribute);

            // East border:
            get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter);
            game->map.tile_attributes[tile_index].border.east = atoi(attribute);

            // South border:
            get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter);
            game->map.tile_attributes[tile_index].border.south = atoi(attribute);

            // West border:
            get_next_attribute_MACRO(attribute, attribute_counter, tmp, tmp_counter);
            game->map.tile_attributes[tile_index].border.west = atoi(attribute);

        } break;

        default: {
            // Nothing to do!
        } break;
        }
    }
}

void draw_edit_grid(App* app, Game* game, Axes background, int map_tile)
{
    if (DEBUG_MODE && game->EDIT_MODE) {
        SDL_Rect text_clip;
        text_clip.x = 0;
        text_clip.y = 0;
        text_clip.h = TILE_SPRITE_HEIGHT;
        text_clip.w = TILE_SPRITE_WIDTH;

        SDL_Rect text_dest;
        text_dest.x = (background.x + game->scroll.x) + 2;
        text_dest.y = (background.y + game->scroll.y) + 2;
        text_dest.h = EDITOR_FONT_DEST_SIZE_H;
        text_dest.w = EDITOR_FONT_DEST_SIZE_W;

        SDL_Rect box;
        box.x = background.x + game->scroll.x;
        box.y = background.y + game->scroll.y;
        box.h = TILE_SPRITE_HEIGHT;
        box.w = TILE_SPRITE_WIDTH;

        SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderDrawRect(app->renderer, &box);

        if (map_tile < 0) {
            SDL_RenderCopy(app->renderer, game->editor.text_textures[TILE_ATTRIBUTES_LIMIT + map_tile], &text_clip, &text_dest);
        } else {
            SDL_RenderCopy(app->renderer, game->editor.text_textures[map_tile], &text_clip, &text_dest);
        }
    }
}

#define done_generating_map_MACRO()          \
    {                                        \
        if (current_row == game->map.rows) { \
            break;                           \
        }                                    \
    }

void generate_map(App* app, Game* game)
{
    Axes background;
    Axes initial_background;

    if (strcmp(game->map.layout_file, MAP_LIBRARY_FILE) == 0) {
        initial_background.x = game->player.global.x - game->player.window.x;
        initial_background.y = game->player.global.y - game->player.window.y;
    } else {
        initial_background.x = 0;
        initial_background.y = 0;
    }

    background.x = initial_background.x;
    background.y = initial_background.y;

    char last_char = ',';
    int current_row = 0, current_column = 0;

    if (DEBUG_MODE && read_file_time(game->map.layout_file) > game->map.layout_modified_time) {
        game->map.layout_modified_time = read_file_time(game->map.layout_file);
        read_map_layout(game);
    }

    // Initially the time may not be set, so let's not
    // be wasteful and read the attributes twice.
    if (game->map.attributes_modified_time == 0) {
        game->map.attributes_modified_time = read_file_time(game->map.attributes_file);
    }
    if (DEBUG_MODE && read_file_time(game->map.attributes_file) > game->map.attributes_modified_time) {
        game->map.attributes_modified_time = read_file_time(game->map.attributes_file);
        SDL_Log("Reloading map attributes ...\n");
        read_map_attributes(game);
    }

    // Set rectangle color for map grid:
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 0);

    for (size_t i = 0; i < game->map.layout_string.length; i++) {

        // Stop if we are at the end of rows.
        done_generating_map_MACRO();

        switch (game->map.layout_string.contents[i]) {
        case BLANK_TILE: {
            // @Robustness:
            // Research ways to have two render pipes,
            // so we are not always switching back
            // and forth between desired colors.
            SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 40);
            draw_edit_grid(app, game, background, EMPTY_COLUMN);
            SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 0);

            // This represents a blank column:
            background.x += TILE_SPRITE_WIDTH;
            game->map.layout[current_row][current_column] = EMPTY_COLUMN;
            current_column++;
            last_char = game->map.layout_string.contents[i];
        } break;
        case ',':
        case ' ': {
            last_char = game->map.layout_string.contents[i];
        } break;
        case '\n': {

            // Record row data to be used for the map editor:
            game->map.columns_in_row[current_row] = current_column;

            background.x = initial_background.x;

            current_column = 0;

            // Y axis increases!
            background.y += TILE_SPRITE_HEIGHT;
            current_row++;

            // Stop if we are at the end of rows.
            done_generating_map_MACRO();

            last_char = game->map.layout_string.contents[i];
        } break;
        default: {
            // Stop if we are at the end of rows.
            done_generating_map_MACRO();

            // Actual tiles!
            // We make sure the last character is not
            // tile info (an integer), because if it
            // is, we should have already processed
            // it and recorded it to our layout
            // data.
            if (map_char_is_not_tile_info(last_char)) {
                // We skip processing unless the last char is fluff.
                last_char = game->map.layout_string.contents[i];

                int map_tile;

                // @Robustness: this feels kinda sloppy, it can probably be improved later.
                if (map_char_is_tile_info(game->map.layout_string.contents[i + 1])) {
                    // Initialize this array with zeros, so we can
                    // guarantee weird characters do not end
                    // up in the map layout files.
                    char map_str_group[TILE_CHAR_LIMIT + 1] = { 0 };

                    size_t chars_to_copy = TILE_CHAR_LIMIT - ((map_char_is_tile_info(game->map.layout_string.contents[i + 2])) ? 0 : 1);
                    memcpy(map_str_group, &game->map.layout_string.contents[i], chars_to_copy);

                    map_tile = atoi(map_str_group);
                } else {
                    // This converts a char to an integer:
                    map_tile = game->map.layout_string.contents[i] - '0';
                }

                Axes tile;
                tile.x = game->map.tile_attributes[map_tile].clip.x;
                tile.y = game->map.tile_attributes[map_tile].clip.y;

                SDL_Rect src;
                src.x = background.x, src.y = background.y;

                SDL_Rect clip;

                clip.x = tile.x, clip.y = tile.y;
                clip.h = TILE_SPRITE_HEIGHT, clip.w = TILE_SPRITE_WIDTH;

                SDL_Rect dest;

                dest.x = background.x + game->scroll.x;
                dest.y = background.y + game->scroll.y;
                dest.h = TILE_SPRITE_HEIGHT, dest.w = TILE_SPRITE_WIDTH;

                SDL_QueryTexture(game->background_image, NULL, NULL, &src.w, &src.h);
                SDL_RenderCopy(app->renderer, game->background_image, &clip, &dest);

                SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 40);
                draw_edit_grid(app, game, background, map_tile);
                SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 0);

                background.x += TILE_SPRITE_WIDTH;
                game->map.layout[current_row][current_column] = map_tile;
                current_column++;

            } else {
                break;
            }
        } break;
        }
    }

    // Reset positioning when the library is loaded so it appears in the upper left of the viewport.
    background.x = initial_background.x;
    background.y = initial_background.y;
}
