/*
 *
 * KIAVC cursor implementation. We might want to use different cursor
 * in the engine at different times, and so this code helps us keep
 * separate instances we can address independently. We assume a cursor
 * to always be animated, and so it uses an animation property: in case
 * of static cursors, an animation with a single frame can be used
 * as a workaround. We also keep track of where the cursor is at any
 * given time, and whether it should be displayed or not.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_CURSOR_H
#define __KIAVC_CURSOR_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#include "resources.h"
#include "animation.h"

/* Abstraction of a cursor in the KIAVC engine */
typedef struct kiavc_cursor {
	/* Common resource info */
	kiavc_resource res;
	/* Unique ID of the cursor */
	char *id;
	/* Cursor animation */
	kiavc_animation *animation;
	/* Current coordinates of the cursor */
	int x, y;
	/* Current frame in a cursor animation */
	int frame;
} kiavc_cursor;

/* Cursor constructor */
kiavc_cursor *kiavc_cursor_create(const char *id);
/* Cursor destructor */
void kiavc_cursor_destroy(kiavc_cursor *cursor);

#endif
