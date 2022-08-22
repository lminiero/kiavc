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

#include "object.h"

/* Cursor constructor */
kiavc_object *kiavc_object_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_object *object = SDL_calloc(1, sizeof(kiavc_object));
	object->res.type = KIAVC_OBJECT;
	object->id = SDL_strdup(id);
	/* FIXME */
	object->scale = 1.0;
	object->hover.from_x = -1;
	object->hover.from_y = -1;
	object->hover.to_x = -1;
	object->hover.to_y = -1;
	return object;
}

/* Cursor destructor */
void kiavc_object_destroy(kiavc_object *object) {
	if(object) {
		SDL_free(object->id);
		SDL_free(object);
	}
}
