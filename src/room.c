/*
 *
 * KIAVC room structure implementation. It's a simple abstraction that
 * mostly takes into account what the C side of the engine needs to
 * know, since all the logic related to the actual room properties will
 * sit in the Lua script instead. As such, we only keep track of the
 * images we may need to render (e.g., background and overlay), plus
 * where the room visual data should be clipped when rendering.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL_image.h>

#include "room.h"

/* Helper static function to free a room layer */
static void kiavc_room_layer_destroy(kiavc_room_layer *layer) {
	if(layer) {
		SDL_free(layer->id);
		SDL_free(layer);
	}
}

/* Room constructor */
kiavc_room *kiavc_room_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_room *room = SDL_calloc(1, sizeof(kiavc_room));
	room->res.type = KIAVC_ROOM;
	room->res.zplane = -50;		/* By default a room is blitted way back */
	room->id = SDL_strdup(id);
	return room;
}

/* Add a layer to a room */
kiavc_room_layer *kiavc_room_add_layer(kiavc_room *room, const char *id, int zplane) {
	if(!room || !id)
		return NULL;
	/* Make sure such a layer doesn't exist already */
	kiavc_list *list = room->layers;
	while(list) {
		kiavc_room_layer *layer = (kiavc_room_layer *)list->data;
		if(layer->id && !SDL_strcasecmp(layer->id, id)) {
			/* Layer exists */
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Layer '%s' exists already\n", id);
			return NULL;
		}
		list = list->next;
	}
	kiavc_room_layer *layer = SDL_calloc(1, sizeof(kiavc_room_layer));
	layer->res.type = KIAVC_ROOM_LAYER;
	layer->res.zplane = zplane;
	layer->id = SDL_strdup(id);
	room->layers = kiavc_list_append(room->layers, layer);
	return layer;
}

/* Remove a layer from a room */
int kiavc_room_remove_layer(kiavc_room *room, const char *id) {
	if(!room || !id)
		return -1;
	kiavc_list *list = room->layers;
	while(list) {
		kiavc_room_layer *layer = (kiavc_room_layer *)list->data;
		if(layer->id && !SDL_strcasecmp(layer->id, id)) {
			/* Found */
			room->layers = kiavc_list_remove(room->layers, layer);
			kiavc_room_layer_destroy(layer);
			return 0;
		}
	}
	/* If we got here, we couldn't find it */
	return -2;
}

/* Add a walkbox to a room */
int kiavc_room_add_walkbox(kiavc_room *room, kiavc_pathfinding_walkbox *walkbox) {
	if(!room || !walkbox)
		return -1;
	if(!room->pathfinding)
		room->pathfinding = kiavc_pathfinding_context_create();
	room->pathfinding->walkboxes = kiavc_list_append(room->pathfinding->walkboxes, walkbox);
	return 0;
}

/* Enable a walkbox in a room */
int kiavc_room_enable_walkbox(kiavc_room *room, const char *name) {
	if(!room || !room->pathfinding || !name)
		return -1;
	kiavc_list *list = room->pathfinding->walkboxes;
	while(list) {
		kiavc_pathfinding_walkbox *walkbox = (kiavc_pathfinding_walkbox *)list->data;
		if(walkbox->name && !SDL_strcasecmp(walkbox->name, name)) {
			/* Found */
			walkbox->disabled = false;
			kiavc_pathfinding_context_recalculate(room->pathfinding);
			return 0;
		}
		list = list->next;
	}
	/* If we got here, we couldn't find it */
	return -2;
}

/* Disable a walkbox in a room */
int kiavc_room_disable_walkbox(kiavc_room *room, const char *name) {
	if(!room || !room->pathfinding || !name)
		return -1;
	kiavc_list *list = room->pathfinding->walkboxes;
	while(list) {
		kiavc_pathfinding_walkbox *walkbox = (kiavc_pathfinding_walkbox *)list->data;
		if(walkbox->name && !SDL_strcasecmp(walkbox->name, name)) {
			/* Found */
			walkbox->disabled = true;
			kiavc_pathfinding_context_recalculate(room->pathfinding);
			return 0;
		}
		list = list->next;
	}
	/* If we got here, we couldn't find it */
	return -2;
}

/* Room destructor */
void kiavc_room_destroy(kiavc_room *room) {
	if(room) {
		SDL_free(room->id);
		g_list_free_full(room->layers, (GDestroyNotify)kiavc_room_layer_destroy);
		kiavc_pathfinding_context_destroy(room->pathfinding);
		SDL_free(room);
	}
}
