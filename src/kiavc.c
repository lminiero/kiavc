/*
 *
 * KIAVC Is an Adventure Videogame Creator (KIAVC).
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"
#include "version.h"

/* Main application */
int main(int argc, char *argv[]) {
	SDL_Log("KIAVC Is an Adventure Videogame Creator (KIAVC) v%s\n", KIAVC_VERSION_STRING);

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

	/* Check if we need to read assets from a BAG file or from disk */
	const char *bagfile = NULL;
	if(argc > 1)
		bagfile = argv[1];

	/* Initialize the game engine */
	if(kiavc_engine_init(bagfile) < 0) {
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
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	SDL_Log("Bye!\n");
	exit(0);

	/* If we got here, something went wrong */
error:
	kiavc_engine_destroy();
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	exit(1);
}
