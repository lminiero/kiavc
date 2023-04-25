/*
 *
 * KIAVC room structure implementation. It's a simple abstraction that
 * mostly takes into account what the C side of the engine needs to
 * know, since all the logic related to the actual room properties will
 * sit in the Lua script instead. As such, we only keep track of the
 * images we may need to render (e.g., background and overlay), plus
 * where the room visual data should be clipped when rendering.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef __KIAVC_ROOM_H
#define __KIAVC_ROOM_H

#include <SDL2/SDL.h>

#include "resources.h"
#include "list.h"
#include "animation.h"
#include "pathfinding.h"

/* Abstraction of a room in the KIAVC engine */
typedef struct kiavc_room {
	/* Common resource info */
	kiavc_resource res;
	/* Unique ID of the room */
	char *id;
	/* Background image */
	kiavc_animation *background;
	/* Room layers, if any */
	kiavc_list *layers;
	/* Pathfinding context */
	kiavc_pathfinding_context *pathfinding;
	/* List of actors in this room */
	kiavc_list *actors;
	/* List of objects in this room */
	kiavc_list *objects;
} kiavc_room;

/* Abstraction of a room layer (background or foreground) */
typedef struct kiavc_room_layer {
	/* Common resource info */
	kiavc_resource res;
	/* Unique ID of the room layer */
	char *id;
	/* Layer image, if any */
	kiavc_animation *background;
} kiavc_room_layer;

/* Room constructor */
kiavc_room *kiavc_room_create(const char *id);
/* Add a layer to a room */
kiavc_room_layer *kiavc_room_add_layer(kiavc_room *room, const char *id, int zplane);
/* Remove a layer from a room */
int kiavc_room_remove_layer(kiavc_room *room, const char *id);
/* Add a walkbox to a room */
int kiavc_room_add_walkbox(kiavc_room *room, kiavc_pathfinding_walkbox *walkbox);
/* Enable a walkbox in a room */
int kiavc_room_enable_walkbox(kiavc_room *room, const char *name);
/* Disable a walkbox in a room */
int kiavc_room_disable_walkbox(kiavc_room *room, const char *name);
/* Room destructor */
void kiavc_room_destroy(kiavc_room *room);

#endif
