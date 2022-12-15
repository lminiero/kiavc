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

#ifndef __KIAVC_ACTOR_H
#define __KIAVC_ACTOR_H

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "resources.h"
#include "costume.h"
#include "room.h"
#include "font.h"

#define KIAVC_ACTOR_INVISIBLE	0
#define KIAVC_ACTOR_STILL		1
#define KIAVC_ACTOR_WALKING		2
#define KIAVC_ACTOR_TALKING		3
#define KIAVC_ACTOR_USING_H		4
#define KIAVC_ACTOR_USING_M		5
#define KIAVC_ACTOR_USING_L		6
/* Helper to convert a string actor state to one of the above defines */
int kiavc_actor_state(const char *state_str);
/* Helper to stringify an actor state */
const char *kiavc_actor_state_str(int state);

/* Abstraction of an actor in the KIAVC engine */
typedef struct kiavc_actor {
	/* Common resource info */
	kiavc_resource res;
	/* Unique ID of the actor */
	char *id;
	/* Current costume of the actor */
	kiavc_costume *costume;
	/* Room this actor is in */
	kiavc_room *room;
	/* Walkbox this actor is in */
	kiavc_pathfinding_walkbox *walkbox;
	/* Whether the actor is visible */
	bool visible;
	/* Walking path for an actor */
	kiavc_list *path, *step;
	/* Current state of the actor */
	int state;
	/* Current direction of the actor */
	int direction;
	/* Scaling of the actor, if needed */
	float scale;
	/* Current frame in an actor animation */
	int frame;
	/* Line this actor is saying, if any */
	kiavc_font_text *line;
} kiavc_actor;

/* Actor constructor */
kiavc_actor *kiavc_actor_create(const char *id);
/* Actor destructor */
void kiavc_actor_destroy(kiavc_actor *actor);

#endif
