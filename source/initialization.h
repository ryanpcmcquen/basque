#include "mechanics.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Game game;
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
    if (game.font.face) {
        TTF_CloseFont(game.font.face);
    }

    // Good to clean this up if we have a bad exit.
    remove(MAP_LOCK_FILE);

    free(game.map.columns_in_row);
    free(game.map.layout);
    free(game.map.layout_string.contents);
    free(game.map.attributes_string.contents);

    IMG_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

void create_outlined_font(Game* game, char* map_tile_str)
{
    game->font.surface = TTF_RenderText_Blended(game->font.face, map_tile_str, game->font.color);

    // The font outline has to set/reset with each creation call,
    // but this saves us from loading the same font twice!
    TTF_SetFontOutline(game->font.face, EDITOR_FONT_OUTLINE);
    game->font.outline_surface = TTF_RenderText_Blended(game->font.face, map_tile_str, game->font.outline_color);
    // Reset the font outline so we don't get crazy fonts.
    TTF_SetFontOutline(game->font.face, 0);

    game->font.rect.x = EDITOR_FONT_OUTLINE, game->font.rect.y = EDITOR_FONT_OUTLINE;
    game->font.rect.w = game->font.surface->w, game->font.rect.h = game->font.surface->h;

    SDL_SetSurfaceBlendMode(game->font.surface, SDL_BLENDMODE_BLEND);
    SDL_BlitSurface(game->font.surface, NULL, game->font.outline_surface, &game->font.rect);
}

void main_game_loop()
{
    handle_input(&app, &game);
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
                    SCREEN_WIDTH = display_mode.w, SCREEN_HEIGHT = display_mode.h;
                } else {
                    SCREEN_WIDTH = (int)(display_mode.w * 0.8);
                    SCREEN_HEIGHT = (int)(display_mode.h * 0.8);
                }
            }
            SDL_Log("Detected resolution: %ix%i\n", display_mode.w, display_mode.h);
        }

        // Fix for multi-montior setups:
        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

        app.window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
        // SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        if (app.window != NULL) {
            app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_PRESENTVSYNC);
            // TODO:
            // Look into ways to check for the ability to use hardware acceleration, vsync, et cetera.
            // app.renderer = SDL_CreateRenderer(app.window, -1, 0);
            // app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
            // app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if (app.renderer != NULL) {

                if (FULLSCREEN_MODE) {
                    // Other options available: SDL_WINDOW_FULLSCREEN_DESKTOP or 0
                    SDL_SetWindowFullscreen(app.window, SDL_WINDOW_FULLSCREEN);
                }

                // Comment this out because it only exists in newer SDLs, and limits the engine from
                // compiling on esoteric platforms (like the Pocket CHIP).
                // SDL_RenderSetIntegerScale(app.renderer, true);
                SDL_RenderSetScale(app.renderer, INITIAL_SCALING, INITIAL_SCALING);
                SDL_Log("%s started with %0.1fx scaling.", GAME_TITLE, INITIAL_SCALING);
                // Set initial draw color:
                SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);

                int flags = IMG_INIT_JPG | IMG_INIT_PNG;
                int img_initted = IMG_Init(flags);
                if ((img_initted & flags) != flags) {
                    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "IMG_Init: %s\n", IMG_GetError());
                }
                if (DEBUG_MODE) {

                    if (TTF_Init() == 0) {
                        game.font.face = TTF_OpenFont(EDITOR_FONT, EDITOR_FONT_SIZE);

                        // These are compound literals!
                        game.font.color = (SDL_Color) { 255, 255, 255, 210 };
                        game.font.outline_color = (SDL_Color) { 10, 10, 10, 180 };

                        char map_tile_str[TILE_CHAR_LIMIT + 1] = { 0 };
                        for (int tile_index = 0; tile_index < TILE_ATTRIBUTES_LIMIT; tile_index++) {
                            snprintf(map_tile_str, sizeof(char[TILE_CHAR_LIMIT + 1]), "%-3i", tile_index);
                            create_outlined_font(&game, map_tile_str);

                            game.editor.text_surfaces[tile_index] = *game.font.outline_surface;
                            game.editor.text_textures[tile_index] = SDL_CreateTextureFromSurface(app.renderer, &game.editor.text_surfaces[tile_index]);
                        }

                        // Set empty column label:
                        snprintf(map_tile_str, sizeof(char[TILE_CHAR_LIMIT + 1]), "%-3i", EMPTY_COLUMN);
                        create_outlined_font(&game, map_tile_str);

                        int calculated_empty_column;
                        if (EMPTY_COLUMN < 0) {
                            calculated_empty_column = TILE_ATTRIBUTES_LIMIT + EMPTY_COLUMN;
                        } else {
                            calculated_empty_column = EMPTY_COLUMN;
                        }
                        game.editor.text_surfaces[calculated_empty_column] = *game.font.outline_surface;
                        game.editor.text_textures[calculated_empty_column] = SDL_CreateTextureFromSurface(app.renderer, &game.editor.text_surfaces[calculated_empty_column]);

                        SDL_FreeSurface(game.font.surface);
                        SDL_FreeSurface(game.font.outline_surface);
                        game.editor.layout_file_suffix = 0;
                        game.editor.selected_tile = EMPTY_COLUMN;
                    } else {
                        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "TTF_Init: %s\n", TTF_GetError());
                        exit(EXIT_FAILURE);
                    }
                }

                game.map.layout_file_base = MAP_LAYOUT_FILE_BASE;
                game.map.layout_file = MAP_LAYOUT_FILE;
                game.map.layout_modified_time = 0;
                game.map.attributes_file = MAP_ATTRIBUTES_FILE;
                game.map.attributes_modified_time = 0;

                read_map_layout(&game);
                read_map_attributes(&game);

                game.player.window.x = 0, game.player.window.y = 0, game.player.global.x = 0, game.player.global.y = 0;
                game.player.direction = SOUTH;

                prepare_scene(&app, &game);
                generate_map(&app, &game);

                game.done = SDL_FALSE;

#ifdef __EMSCRIPTEN__
                emscripten_set_main_loop(main_game_loop, 0, 1);
#endif

#ifndef __EMSCRIPTEN__
                while (!game.done) {
                    main_game_loop();
                }
#endif
            }
        }
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    return 0;
}
