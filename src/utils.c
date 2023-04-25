/*
 *
 * KIAVC generic utilities.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include "utils.h"

/* Helper to create a surface to use in the engine */
SDL_Surface *kiavc_create_surface(int w, int h) {
	if(w < 1 || h < 1)
		return NULL;
	Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#else
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#endif
	SDL_Surface *surface = SDL_CreateRGBSurface(0, w, h, 32, rmask, gmask, bmask, amask);
	if(!surface)
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating %dx%d surface: %s\n", w, h, SDL_GetError());
	return surface;
}
