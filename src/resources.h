/*
 *
 * Definition of globan an inheritable resources.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_RESOURCES_H
#define __KIAVC_RESOURCES_H

#include <SDL2/SDL.h>

#define KIAVC_ROOM			1
#define KIAVC_ROOM_LAYER	2
#define KIAVC_ACTOR			3
#define KIAVC_OBJECT		4
#define KIAVC_FONT_TEXT		5
#define KIAVC_CURSOR		6
#define KIAVC_DIALOG		7

/* Dynamic renderable resource (rooms and cursors are excluded since
 * there can only be a single instance of each displayed at any time) */
typedef struct kiavc_resource {
	/* All renderable resources will have a type as their first attribute */
	Uint8 type;
	/* All resources will have a z-plane value as their second attribute */
	int zplane;
	/* All resources will have a ticks timer as their third attribute */
	uint32_t ticks;
} kiavc_resource;

#endif
