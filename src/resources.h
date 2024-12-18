/*
 *
 * Definition of global and inheritable resources.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
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
#define KIAVC_PLUGIN		8

/* Dynamic renderable resource (rooms and cursors are excluded since
 * there can only be a single instance of each displayed at any time) */
typedef struct kiavc_resource {
	/* Resource type */
	Uint8 type;
	/* Current position (in float to account for movement) */
	float x, y;
	/* Z-plane */
	int zplane;
	/* Ticks timer */
	uint32_t ticks;
	/* Whether we're fading the object in or out, and how long that should be */
	int fade_ms;
	/* Fade alpha to apply */
	Uint8 fade_alpha, fade_start, fade_target;
	/* Fade in/out ticks */
	uint32_t fade_ticks;
	/* Target coordinates of the resource, when it's moving */
	int target_x, target_y;
	/* Movement speed of the resource (pixels per second) */
	int speed;
	/* Movement ticks */
	uint32_t move_ticks;
} kiavc_resource;

#endif
