/*
 *
 * KIAVC asset archiving in BAG archives. A BAG archive is basically a
 * structured collection of multiple files, all saved one after the
 * other: their "path" is used as a key to access them, and a JSON
 * body at the end of the archive includes info on how to access them
 * plus some metadata.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_BAG_H
#define __KIAVC_BAG_H

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "map.h"
#include "list.h"

/* BAG archive */
typedef struct kiavc_bag {
	/* File instance, if open */
	FILE *file;
	/* Version of the archive */
	int major, minor, patch;
	/* Whether this archive can be modified (imported ones can't) */
	bool readonly;
	/* Map of assets, indexed by their path */
	kiavc_map *map;
	/* Linked list of assets, ordered by their path */
	kiavc_list *list;
} kiavc_bag;

/* Asset in a BAG file */
typedef struct kiavc_bag_asset {
	/* Key */
	const char *key;
	/* Path to the file */
	const char *path;
	/* Offset of this asset in the file */
	uint32_t offset;
	/* Size of the file */
	uint32_t size;
} kiavc_bag_asset;

/* Create a new BAG archive from scratch */
kiavc_bag *kiavc_bag_create(void);
/* Destroy a BAG archive */
void kiavc_bag_destroy(kiavc_bag *bag);
/* Add an asset to a BAG file */
kiavc_bag_asset *kiavc_bag_add_asset(kiavc_bag *bag, const char *key, const char *path);
/* Remove an asset from a BAG file */
int kiavc_bag_remove_asset(kiavc_bag *bag, const char *key);
/* Helper to print the contents of a BAG archive */
void kiavc_bag_list(kiavc_bag *bag);

/* Import a BAG archive from a file */
kiavc_bag *kiavc_bag_import(const char *filename);
/* Export a BAG archive to a file */
int kiavc_bag_export(kiavc_bag *bag, const char *filename);
/* Export a BAG asset to a file */
int kiavc_bag_asset_export_to_file(kiavc_bag *bag, const char *key, const char *filename);
/* Return a SDL_RWops instance associated with an asset in a BAG file */
SDL_RWops *kiavc_bag_asset_export_rw(kiavc_bag *bag, const char *key);

#endif
