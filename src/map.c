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
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <glib.h>
#include <SDL2/SDL.h>

#include "map.h"

/* Map */
struct kiavc_map {
	/* GLib hashtable */
	GHashTable *table;
};

/* Create a new map */
kiavc_map *kiavc_map_create(kiavc_map_value_destroy value_free) {
	kiavc_map *map = SDL_malloc(sizeof(kiavc_map));
	map->table = g_hash_table_new_full(g_str_hash, g_str_equal,
		(GDestroyNotify)g_free, (GDestroyNotify)value_free);
	return map;
}

/* Destroy an existing map */
void kiavc_map_destroy(kiavc_map *map) {
	/* Destroy the hashtable */
	if(map->table)
		g_hash_table_destroy(map->table);
	SDL_free(map);
}

/* Insert an item in the map */
int kiavc_map_insert(kiavc_map *map, const char *key, void *value) {
	if(!map || !key || !value) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error inserting item in map: invalid arguments\n");
		return -1;
	}
	return g_hash_table_insert(map->table, g_strdup(key), value);
}

/* Lookup an item in the map */
void *kiavc_map_lookup(kiavc_map *map, const char *key) {
	if(!map || !key)
		return NULL;
	return g_hash_table_lookup(map->table, key);
}

/* Return the list of items in the map */
kiavc_list *kiavc_map_get_values(kiavc_map *map) {
	return g_hash_table_get_values(map->table);
}

/* Remove an item from the map */
int kiavc_map_remove(kiavc_map *map, const char *key) {
	if(!map || !key)
		return -1;
	if(!g_hash_table_remove(map->table, key))
		return -1;
	return 0;
}
