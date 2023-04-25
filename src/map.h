/*
 *
 * KIAVC hashtable implementation, which is currently just a dumb
 * wrapper around Glib's GHashTable. Initially I didn't want to also
 * have Glib as a dependency to keep things simple, but it looks like
 * it's implicitly a dependency already, since it's used under the hood
 * by one of the SDL_ttf dependencies, so it made sense to try and take
 * advantage of some of its utilities. Since the interface is more or
 * less abstracted, the actual implementation can be replaced in the
 * future, if needed.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef __KIAVC_MAP_H
#define __KIAVC_MAP_H

#include "list.h"

typedef struct kiavc_map kiavc_map;
typedef void (*kiavc_map_value_destroy)(void *value);

/* Create a new map */
kiavc_map *kiavc_map_create(kiavc_map_value_destroy value_free);
/* Destroy an existing map */
void kiavc_map_destroy(kiavc_map *map);
/* Insert an item in the map */
int kiavc_map_insert(kiavc_map *map, const char *key, void *value);
/* Lookup an item in the map */
void *kiavc_map_lookup(kiavc_map *map, const char *key);
/* Return the list of items in the map */
kiavc_list *kiavc_map_get_values(kiavc_map *map);
/* Remove an item from the map */
int kiavc_map_remove(kiavc_map *map, const char *key);

#endif
