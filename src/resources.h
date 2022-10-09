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
	/* Resource type */
	Uint8 type;
	/* Current position */
	int x, y;
	/* Z-plane */
	int zplane;
	/* Ticks timer */
	uint32_t ticks;
	/* Whether we're fading the object in or out, and how long that should be */
	int fade_in, fade_out;
	/* Fade alpha to apply */
	Uint8 fade_alpha, fade_start, fade_range;
	/* Fade in/out ticks */
	uint32_t fade_ticks;
} kiavc_resource;

#endif
