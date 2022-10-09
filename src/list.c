/*
 *
 * KIAVC list implementation, which is currently just a dumb wrapper
 * arounf Glib's GHashTable. Initially I didn't want to also have Glib
 * as a dependency to keep things simple, but it looks like it's
 * implicitly a dependency already, since it's used under the hood by
 * one of the SDL_ttf dependencies, so it made sense to try and take
 * advantage of some of its utilities. Since the interface is more or
 * less abstracted, the actual implementation can be replaced in the
 * future, if needed.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL.h>

#include "list.h"

/* Create a new list */
kiavc_list *kiavc_list_create(void) {
	/* An empty list is a NULL pointer, in GLib */
	return NULL;
}

/* Destroy an existing list */
void kiavc_list_destroy(kiavc_list *list) {
	/* Destroy the GList instance */
	if(list)
		g_list_free(list);
}

/* Append an item to the list */
kiavc_list *kiavc_list_append(kiavc_list *list, void *item) {
	return g_list_append(list, item);
}

/* Sort the list */
kiavc_list *kiavc_list_sort(kiavc_list *list, kiavc_list_item_compare compare) {
	return g_list_sort(list, compare);
}

/* Insert an item in the list in a sorted way */
kiavc_list *kiavc_list_insert_sorted(kiavc_list *list, void *item, kiavc_list_item_compare compare) {
	return g_list_insert_sorted(list, item, compare);
}

/* Prepend an item to the list */
kiavc_list *kiavc_list_prepend(kiavc_list *list, void *item) {
	return g_list_prepend(list, item);
}

/* Check if an item is in the list */
bool kiavc_list_find(kiavc_list *list, void *item) {
	return g_list_find(list, item);
}

/* Return the list size */
int kiavc_list_size(kiavc_list *list) {
	return g_list_length(list);
}

/* Remove an item from the list */
kiavc_list *kiavc_list_remove(kiavc_list *list, void *item) {
	return g_list_remove(list, item);
}
