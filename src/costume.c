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
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include <SDL2/SDL_image.h>

#include "costume.h"

/* Private method to destroy a set */
static void kiavc_costume_set_destroy(kiavc_costume_set *set) {
	if(set) {
		kiavc_costume_unload_set(set, NULL);
		SDL_free(set);
	}
}

/* Costume constructor */
kiavc_costume *kiavc_costume_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_costume *costume = SDL_calloc(1, sizeof(kiavc_costume));
	costume->id = SDL_strdup(id);
	costume->sets = kiavc_map_create((kiavc_map_value_destroy)kiavc_costume_set_destroy);
	return costume;
}

/* Get or add a costume set */
kiavc_costume_set *kiavc_costume_get_set(kiavc_costume *costume, const char *name) {
	if(!costume || !name)
		return NULL;
	kiavc_costume_set *set = kiavc_map_lookup(costume->sets, name);
	if(set)
		return set;
	set = SDL_calloc(1, sizeof(kiavc_costume_set));
	kiavc_map_insert(costume->sets, name, set);
	return set;
}

/* Helper to load textures for a specific set */
void kiavc_costume_load_set(kiavc_costume_set *set, void *resource, SDL_Renderer *renderer) {
	if(!set)
		return;
	int i = 0, loaded = 0;
	for(i = KIAVC_UP; i<= KIAVC_RIGHT; i++) {
		if(set && set->animations[i] && kiavc_animation_load(set->animations[i], resource, renderer) == 0)
			loaded++;
	}
}

/* Helper to unload textures for a specific set */
void kiavc_costume_unload_set(kiavc_costume_set *set, void *resource) {
	if(!set)
		return;
	int i = 0;
	for(i = KIAVC_UP; i<= KIAVC_RIGHT; i++) {
		if(set && set->animations[i])
			kiavc_animation_unload(set->animations[i], resource);
	}
}

/* Helper to unload textures for all sets */
void kiavc_costume_unload_sets(kiavc_costume *costume, void *resource) {
	if(!costume)
		return;
	kiavc_list *sets = kiavc_map_get_values(costume->sets), *tmp = sets;
	kiavc_costume_set *set = NULL;
	int i = 0;
	while(tmp) {
		set = (kiavc_costume_set *)tmp->data;
		i = 0;
		for(i = KIAVC_UP; i<= KIAVC_RIGHT; i++) {
			if(set && set->animations[i])
				kiavc_animation_unload(set->animations[i], resource);
		}
		tmp = tmp->next;
	}
	kiavc_list_destroy(sets);
}

/* Costume destructor */
void kiavc_costume_destroy(kiavc_costume *costume) {
	if(costume) {
		SDL_free(costume->id);
		kiavc_map_destroy(costume->sets);
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
