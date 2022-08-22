/*
 *
 * KIAVC costume structure implementation. It's basically a collection
 * of different images and animations related to different states an
 * actor may be in, e.g., when standing still, walking or talking. Since
 * the same actor may need a different set of images and animations
 * depending on the state, costumes are separated from actors, meaning
 * different actors can use the same costume if needed (e.g., multiple
 * characters that look the same).
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL_image.h>

#include "costume.h"

/* Costume constructor */
kiavc_costume *kiavc_costume_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_costume *costume = SDL_calloc(1, sizeof(kiavc_costume));
	costume->id = SDL_strdup(id);
	return costume;
}

/* Helper to load all textures */
void kiavc_costume_load_all(kiavc_costume *costume, SDL_Renderer *renderer) {
	if(!costume)
		return;
	int i = 0, loaded = 0;
	for(i = KIAVC_UP; i<= KIAVC_RIGHT; i++) {
		if(costume->still[i] && kiavc_animation_load(costume->still[i], renderer) == 0)
			loaded++;
		if(costume->walking[i] && kiavc_animation_load(costume->walking[i], renderer) == 0)
			loaded++;
		if(costume->talking[i] && kiavc_animation_load(costume->talking[i], renderer) == 0)
			loaded++;
	}
}

/* Costume destructor */
void kiavc_costume_destroy(kiavc_costume *costume) {
	if(costume) {
		SDL_free(costume->id);
		SDL_free(costume);
	}
}

/* Helper to convert a string direction to one of the above defines */
int kiavc_costume_direction(const char *direction) {
	if(!direction)
		return KIAVC_NONE;
	else if(!SDL_strcasecmp(direction, "up"))
		return KIAVC_UP;
	else if(!SDL_strcasecmp(direction, "down"))
		return KIAVC_DOWN;
	else if(!SDL_strcasecmp(direction, "left"))
		return KIAVC_LEFT;
	else if(!SDL_strcasecmp(direction, "right"))
		return KIAVC_RIGHT;
	return KIAVC_NONE;
}
