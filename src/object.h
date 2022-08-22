/*
 *
 * KIAVC object structure implementation. An object is basically
 * anything that we may want to be able to see and/or interact with,
 * except other actors. Objects can be used for different purposes:
 * actual objects, but also things that should appear differently in
 * a room at different times. To be able to interact with an object, a
 * set of coordinates must be provided so that the engine knows when,
 * e.g., the cursor is hovering over it; an image or animation is not
 * strictly needed, especially if the object is drawn on the background
 * already.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_OBJECT_H
#define __KIAVC_OBJECT_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#include "resources.h"
#include "room.h"
#include "actor.h"
#include "animation.h"

/* Object box coordinates */
typedef struct kiavc_object_box {
	int from_x, from_y, to_x, to_y;
} kiavc_object_box;

/* Abstraction of an object in the KIAVC engine */
typedef struct kiavc_object {
	/* Common resource info */
	kiavc_resource res;
	/* Unique ID of the object */
	char *id;
	/* Room this object is in */
	kiavc_room *room;
	/* If the object is in an inventory, the owner of the object */
	kiavc_actor *owner;
	/* Object image or animation */
	kiavc_animation *animation;
	/* Current coordinates of the object */
	int x, y;
	/* FIXME Coordinates for detecting interaction */
	kiavc_object_box hover;
	/* Whether the object is visible or not */
	bool visible;
	/* Scaling of the actor, if needed */
	float scale;
	/* Current frame in an object animation */
	int frame;
	/* Whether this object is part of the UI */
	bool ui;
	/* UI image or animation, if any */
	kiavc_animation *ui_animation;
} kiavc_object;

/* Cursor constructor */
kiavc_object *kiavc_object_create(const char *id);
/* Cursor destructor */
void kiavc_object_destroy(kiavc_object *object);

#endif
