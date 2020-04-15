#include "mechanics.h"

GameState game;
App app;

void cleanup(void)
{

    if (app.renderer) {
        SDL_DestroyRenderer(app.renderer);
    }
    if (app.window) {
        SDL_DestroyWindow(app.window);
    }

    if (game.player_image) {
        SDL_DestroyTexture(game.player_image);
    }
    if (game.background_image) {
        SDL_DestroyTexture(game.background_image);
    }
    if (game.music) {
        Mix_FreeMusic(game.music);
    }
    if (game.font) {
        TTF_CloseFont(game.font);
    }

    // Good to clean this up if we have a bad exit.
    remove(MAP_LOCK_FILE);

    IMG_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

#define create_outlined_font(game, map_tile_str, font_outline_surface, font_surface, font_outline_color, font_color, font_rect) \
    {                                                                                                                           \
                                                                                                                                \
        font_outline_surface = TTF_RenderText_Blended(game.font_outline, map_tile_str, font_outline_color);                     \
        font_surface = TTF_RenderText_Blended(game.font, map_tile_str, font_color);                                             \
        font_rect.x = EDITOR_FONT_OUTLINE;                                                                                      \
        font_rect.y = EDITOR_FONT_OUTLINE;                                                                                      \
        font_rect.w = font_surface->w;                                                                                          \
        font_rect.h = font_surface->h;                                                                                          \
                                                                                                                                \
        SDL_SetSurfaceBlendMode(font_surface, SDL_BLENDMODE_BLEND);                                                             \
        SDL_BlitSurface(font_surface, NULL, font_outline_surface, &font_rect);                                                  \
    }

int init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0) {
        if (DETECT_RESOLUTION) {
            SDL_DisplayMode display_mode;
            if (SDL_GetDesktopDisplayMode(0, &display_mode) != 0) {
                SDL_Log("SDL_GetDesktopDisplayMode failed: %s\nWe will use the default resolution instead.", SDL_GetError());
            } else {
                if (FULLSCREEN_MODE) {
                    SCREEN_WIDTH = display_mode.w;
                    SCREEN_HEIGHT = display_mode.h;
                } else {
                    SCREEN_WIDTH = display_mode.w * 0.8;
                    SCREEN_HEIGHT = display_mode.h * 0.8;
                }
            }
            SDL_Log("Detected screen width: %i\nDetected screen height: %i\n", display_mode.w, display_mode.h);
        }

        app.window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);

        if (app.window != NULL) {
            app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if (app.renderer != NULL) {

                if (FULLSCREEN_MODE) {
                    // Other options available: SDL_WINDOW_FULLSCREEN_DESKTOP or 0
                    SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN);
                }

                SDL_RenderSetScale(app.renderer, SCALING, SCALING);
                SDL_Log("Basque started with %ix scaling.", SCALING);
                // Set initial draw color:
                SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);

                int flags = IMG_INIT_JPG | IMG_INIT_PNG;
                int img_initted = IMG_Init(flags);
                if ((img_initted & flags) != flags) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "IMG_Init: %s\n", IMG_GetError());
                }
                if (DEBUG_MODE) {

                    if (TTF_Init() == 0) {
                        game.font = TTF_OpenFont(EDITOR_FONT, EDITOR_FONT_SIZE);
                        game.font_outline = TTF_OpenFont(EDITOR_FONT, EDITOR_FONT_SIZE);
                        TTF_SetFontOutline(game.font_outline, EDITOR_FONT_OUTLINE);

                        SDL_Color font_color = { 255, 255, 255, 255 };
                        SDL_Color font_outline_color = { 10, 10, 10, 200 };
                        SDL_Surface* font_outline_surface;
                        SDL_Surface* font_surface;
                        SDL_Rect font_rect;

                        char map_tile_str[TILE_CHAR_LIMIT] = { 0 };
                        for (int tile_index = 0; tile_index < NUMBER_OF_TILES; tile_index++) {
                            snprintf(map_tile_str, sizeof(char[TILE_CHAR_LIMIT]), "%-3i", tile_index);
                            create_outlined_font(game, map_tile_str, font_outline_surface, font_surface, font_outline_color, font_color, font_rect);

                            game.editor.text_surfaces[tile_index] = font_outline_surface;
                            game.editor.text_textures[tile_index] = SDL_CreateTextureFromSurface(app.renderer, game.editor.text_surfaces[tile_index]);
                        }

                        // Set empty column label:
                        snprintf(map_tile_str, sizeof(char[TILE_CHAR_LIMIT]), "%-3i", EMPTY_COLUMN);
                        create_outlined_font(game, map_tile_str, font_outline_surface, font_surface, font_outline_color, font_color, font_rect);

                        if (EMPTY_COLUMN < 0) {
                            game.editor.text_surfaces[NUMBER_OF_TILES + EMPTY_COLUMN] = font_outline_surface;
                            game.editor.text_textures[NUMBER_OF_TILES + EMPTY_COLUMN] = SDL_CreateTextureFromSurface(app.renderer, game.editor.text_surfaces[NUMBER_OF_TILES + EMPTY_COLUMN]);
                        } else {
                            game.editor.text_surfaces[EMPTY_COLUMN] = font_outline_surface;
                            game.editor.text_textures[EMPTY_COLUMN] = SDL_CreateTextureFromSurface(app.renderer, game.editor.text_surfaces[EMPTY_COLUMN]);
                        }

                        SDL_FreeSurface(font_surface);
                        SDL_FreeSurface(font_outline_surface);
                        game.editor.layout_file_suffix = 0;
                        game.editor.selected_tile = EMPTY_COLUMN;
                    } else {
                        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_Init: %s\n", TTF_GetError());
                        exit(EXIT_FAILURE);
                    }
                }

                game.map.layout_file_base = MAP_LAYOUT_FILE_BASE;
                game.map.layout_file = MAP_LAYOUT_FILE;
                game.map.layout_modified_time = read_file_time(game.map.layout_file);
                game.map.layout_modified_time = 0;
                game.map.attributes_file = MAP_ATTRIBUTES_FILE;
                game.map.attributes_modified_time = read_file_time(game.map.attributes_file);
                game.map.attributes_modified_time = 0;

                read_map_layout(&game);
                read_map_attributes(&game);

                prepare_scene(&app, &game);
                generate_map(&app, &game);
                handle_input(&app, &game);
            }
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    return 0;
}
