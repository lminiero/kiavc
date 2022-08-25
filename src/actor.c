/*
 *
 * KIAVC actor structure implementation. It's a simple abstraction that
 * mostly takes into account what the C side of the engine needs to
 * know, since all the logic related to the actual actor properties and
 * actions will sit in the Lua script instead. As such, we only keep
 * track of the images and animations we may need to render (e.g., when
 * standing still, walking or talking), plus additional information
 * related to rendering, like the direction the actor is facing, their
 * coordinates in a room, where they're going, etc.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL_image.h>

#include "actor.h"

/* Actor constructor */
kiavc_actor *kiavc_actor_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_actor *actor = SDL_calloc(1, sizeof(kiavc_actor));
	actor->res.type = KIAVC_ACTOR;
	actor->res.fade_alpha = 255;
	actor->id = SDL_strdup(id);
	actor->direction = KIAVC_DOWN;
	actor->scale = 1.0;
	actor->speed = 1;
	actor->target_x = -1;
	actor->target_y = -1;
	return actor;
}

/* Actor destructor */
void kiavc_actor_destroy(kiavc_actor *actor) {
	if(actor) {
		SDL_free(actor->id);
		if(actor->line)
			kiavc_font_text_destroy(actor->line);
		SDL_free(actor);
	}
}

/* Helper to convert a string actor state to one of the above defines */
int kiavc_actor_state(const char *state_str) {
	if(!state_str)
		return KIAVC_ACTOR_INVISIBLE;
	else if(!SDL_strcasecmp(state_str, "still"))
		return KIAVC_ACTOR_STILL;
	else if(!SDL_strcasecmp(state_str, "walking"))
		return KIAVC_ACTOR_WALKING;
	else if(!SDL_strcasecmp(state_str, "talking"))
		return KIAVC_ACTOR_TALKING;
	else if(!SDL_strcasecmp(state_str, "usehigh"))
		return KIAVC_ACTOR_USING_H;
	else if(!SDL_strcasecmp(state_str, "usemid"))
		return KIAVC_ACTOR_USING_M;
	else if(!SDL_strcasecmp(state_str, "uselow"))
		return KIAVC_ACTOR_USING_L;
	return KIAVC_ACTOR_STILL;
}

/* Helper to stringify an actor state */
const char *kiavc_actor_state_str(int state) {
	switch(state) {
		case KIAVC_ACTOR_STILL:
			return "still";
		case KIAVC_ACTOR_WALKING:
			return "walking";
		case KIAVC_ACTOR_TALKING:
			return "talking";
		case KIAVC_ACTOR_USING_H:
			return "usehigh";
		case KIAVC_ACTOR_USING_M:
			return "usemid";
		case KIAVC_ACTOR_USING_L:
			return "uselow";
		default:
			break;
	}
	return NULL;
}
