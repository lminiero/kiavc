/*
 *
 * KIAVC Is an Adventure Videogame Creator (KIAVC).
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"
#include "logger.h"
#include "version.h"

/* Main application */
int main(int argc, char *argv[]) {
	/* Check if we need to read assets from a BAG file or from disk */
	kiavc_bag *bag = NULL;
	const char *bagfile = NULL;
	if(argc > 1)
		bagfile = argv[1];
	/* If we need to open a BAG file, let's import it now */
	if(bagfile) {
		bag = kiavc_bag_import(bagfile);
		if(!bag) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error importing BAG file\n");
			return -1;
		}
	}
	/* Read the game.kvc file from the bag (or the disk, if there's no bag) */
	const char *path = "./game.kvc";
	SDL_RWops *rwops = NULL;
	if(bag) {
		rwops = kiavc_bag_asset_export_rw(bag, path);
	} else {
		rwops = SDL_RWFromFile(path, "rb");
	}
	if(rwops == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error opening asset file '%s'\n", path);
		kiavc_bag_destroy(bag);
		return -1;
	}
	SDL_RWseek(rwops, 0, RW_SEEK_END);
	Sint64 size = SDL_RWtell(rwops);
	SDL_RWseek(rwops, 0, RW_SEEK_SET);
	if(size < 1 || size > 64) {
		SDL_RWclose(rwops);
		kiavc_bag_destroy(bag);
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Invalid asset file size '%"SCNi64"'\n", size);
		return -1;
	}
	char app[65];
	size_t read = 0, to_read = size, written = 0;
	while(to_read > 0 && (read = SDL_RWread(rwops, app+written, sizeof(char), to_read < size ? to_read : size)) > 0) {
		written += read;
		to_read -= read;
	}
	SDL_RWclose(rwops);
	*(app + size) = '\0';
	char *cr = strchr(app, '\r');
	if(cr)
		*cr = '\0';
	char *lf = strchr(app, '\n');
	if(lf)
		*lf = '\0';

	/* Initialize the logger: by providing the app name, we'll create
	 * a log file in the related path; we can also choose whether to
	 * enable (default) or disable logging on the terminal too */
	kiavc_logger_init(app, true);
	SDL_Log("KIAVC Is an Adventure Videogame Creator (KIAVC) v%s\n", KIAVC_VERSION_STRING);
	SDL_Log("  -- %s\n", app);
	if(bagfile) {
		SDL_Log("Imported BAG file '%s'\n", bagfile);
	} else {
		SDL_Log("No BAG file, loading assets from disk\n");
	}

	/* Initialize SDL backends */
	if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL2: %s\n", SDL_GetError());
		goto error;
	}
	if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL2_image: %s\n", IMG_GetError());
		goto error;
	}
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL2_mixer: %s\n", Mix_GetError());
		goto error;
	}
	if(TTF_Init() == -1) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL2_ttf: %s\n", TTF_GetError());
		goto error;
	}

	/* Initialize the game engine */
	if(kiavc_engine_init(app, bag) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error initializing game engine\n");
		goto error;
	}

	/* Game engine loop */
	while(true) {
		/* Handle the user input */
		if(kiavc_engine_handle_input() < 0)
			break;
		/* Update world */
		if(kiavc_engine_update_world() < 0)
			break;
		/* Render */
		if(kiavc_engine_render() < 0)
			break;
	}

	/* Done */
	kiavc_engine_destroy();
	kiavc_bag_destroy(bag);
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	SDL_Log("Bye!\n");
	kiavc_logger_destroy();
	exit(0);

	/* If we got here, something went wrong */
error:
	kiavc_engine_destroy();
	kiavc_bag_destroy(bag);
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	kiavc_logger_destroy();
	exit(1);
}
