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

#include "cursor.h"

/* Cursor constructor */
kiavc_cursor *kiavc_cursor_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_cursor *cursor = SDL_calloc(1, sizeof(kiavc_cursor));
	cursor->res.type = KIAVC_CURSOR;
	cursor->res.zplane = 99;	/* By default the cursor goes on top */
	cursor->id = SDL_strdup(id);
	return cursor;
}

/* Cursor destructor */
void kiavc_cursor_destroy(kiavc_cursor *cursor) {
	if(cursor) {
		SDL_free(cursor->id);
		SDL_free(cursor);
	}
}
