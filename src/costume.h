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

#ifndef __KIAVC_COSTUME_H
#define __KIAVC_COSTUME_H

#include <SDL2/SDL.h>

#include "animation.h"
#include "map.h"

#define KIAVC_NONE		-1
#define KIAVC_UP		0
#define KIAVC_DOWN		1
#define KIAVC_LEFT		2
#define KIAVC_RIGHT		3
/* Helper to convert a string direction to one of the above defines */
int kiavc_costume_direction(const char *direction);

/* Abstraction of an costume in the KIAVC engine */
typedef struct kiavc_costume {
	/* Unique ID of the costume */
	char *id;
	/* Set of costumes, indexed by type (e.g., still, talking, walking) */
	kiavc_map *sets;
} kiavc_costume;

/* Set of animations for a specific activity */
typedef struct kiavc_costume_set {
	/* Animations for the four directions */
	kiavc_animation *animations[4];
} kiavc_costume_set;

/* Costume constructor */
kiavc_costume *kiavc_costume_create(const char *id);
/* Get or add a costume set */
kiavc_costume_set *kiavc_costume_get_set(kiavc_costume *costume, const char *name);
/* Helper to load textures for a specific set */
void kiavc_costume_load_set(kiavc_costume_set *set, SDL_Renderer *renderer);
/* Costume destructor */
void kiavc_costume_destroy(kiavc_costume *costume);

#endif
