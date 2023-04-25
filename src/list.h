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
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef __KIAVC_LIST_H
#define __KIAVC_LIST_H

#include <stdbool.h>

#include <glib.h>

typedef GList kiavc_list;
typedef int (*kiavc_list_item_compare)(const void *item1, const void *item2);

/* Create a new list */
kiavc_list *kiavc_list_create(void);
/* Destroy an existing list */
void kiavc_list_destroy(kiavc_list *list);
/* Append an item to the list */
kiavc_list *kiavc_list_append(kiavc_list *list, void *item);
/* Sort the list */
kiavc_list *kiavc_list_sort(kiavc_list *list, kiavc_list_item_compare compare);
/* Insert an item in the list in a sorted way */
kiavc_list *kiavc_list_insert_sorted(kiavc_list *list, void *item, kiavc_list_item_compare compare);
/* Prepend an item to the list */
kiavc_list *kiavc_list_prepend(kiavc_list *list, void *item);
/* Check if an item is in the list */
bool kiavc_list_find(kiavc_list *list, void *item);
/* Return the list size */
int kiavc_list_size(kiavc_list *list);
/* Remove an item from the list */
kiavc_list *kiavc_list_remove(kiavc_list *list, void *item);

#endif
