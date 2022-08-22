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

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <glib.h>

#include "bag.h"
#include "version.h"

#define KIAVC_BAG_HEADER	"KIAVCBAG"

/* Static functions only used as callbacks for a custom SDL_RWops */
static Sint64 kiavc_bag_rwops_size(SDL_RWops *rwops);
static Sint64 kiavc_bag_rwops_seek(SDL_RWops *rwops, Sint64 offset, int whence);
static size_t kiavc_bag_rwops_read(SDL_RWops *rwops, void *ptr, size_t size, size_t maxnum);
static int kiavc_bag_rwops_close(SDL_RWops *rwops);

/* Asset destruction */
static void kiavc_bag_asset_destroy(kiavc_bag_asset *asset) {
	if(asset) {
		SDL_free((char *)asset->key);
		SDL_free((char *)asset->path);
		SDL_free(asset);
	}
}

/* Asset comparator, for sorted inserts */
static int kiavc_bag_asset_compare(const kiavc_bag_asset *a, const kiavc_bag_asset *b) {
	if(!a && !b)
		return 0;
	else if(!a)
		return -1;
	else if(!b)
		return 1;
	if(!a->key && !b->key)
		return 0;
	else if(!a->key)
		return -1;
	else if(!b->key)
		return 1;
	return strcasecmp(a->key, b->key);
}

/* Create a new BAG archive from scratch */
kiavc_bag *kiavc_bag_create(void) {
	kiavc_bag *bag = SDL_calloc(1, sizeof(kiavc_bag));
	bag->major = KIAVC_VERSION_MAJOR;
	bag->minor = KIAVC_VERSION_MINOR;
	bag->patch = KIAVC_VERSION_PATCH;
	bag->map = kiavc_map_create((kiavc_map_value_destroy)&kiavc_bag_asset_destroy);
	return bag;
}

/* Destroy a BAG archive */
void kiavc_bag_destroy(kiavc_bag *bag) {
	if(bag) {
		if(bag->file)
			fclose(bag->file);
		kiavc_map_destroy(bag->map);
		kiavc_list_destroy(bag->list);
	}
}

/* Add an asset to a BAG file (internal version) */
static kiavc_bag_asset *kiavc_bag_add_asset_internal(kiavc_bag *bag, const char *key, const char *path, bool newfile) {
	if(!bag || !key || !path)
		return NULL;
	if(bag->readonly) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add asset '%s' to BAG, archive is read-only\n", key);
		return NULL;
	}
	if(kiavc_map_lookup(bag->map, key)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add asset '%s' to BAG, key already exists\n", key);
		return NULL;
	}
	size_t size = 0;
	if(newfile) {
		/* Make sure the file exists, and check how large it is */
		FILE *file = fopen(path, "rb");
		if(!file) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add asset '%s' to BAG: %s\n", key, strerror(errno));
			return NULL;
		}
		fseek(file, 0L, SEEK_END);
		size = ftell(file);
		fclose(file);
		if(size == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add asset '%s' to BAG, file is empty\n", key);
			return NULL;
		}
	}
	/* Create an asset instance */
	kiavc_bag_asset *asset = calloc(1, sizeof(kiavc_bag_asset));
	asset->key = SDL_strdup(key);
	asset->path = SDL_strdup(path);
	asset->size = size;
	/* Add to map and list */
	kiavc_map_insert(bag->map, asset->key, asset);
	bag->list = kiavc_list_insert_sorted(bag->list, asset, (kiavc_list_item_compare)&kiavc_bag_asset_compare);
	/* Reset all offsets, if needed */
	if(newfile) {
		kiavc_list *list = bag->list;
		while(list) {
			asset = (kiavc_bag_asset *)list->data;
			asset->offset = 0;
			list = list->next;
		}
	}
	/* Done */
	return asset;
}
/* Add an asset to a BAG file */
kiavc_bag_asset *kiavc_bag_add_asset(kiavc_bag *bag, const char *key, const char *path) {
	return kiavc_bag_add_asset_internal(bag, key, path, true);
}

/* Remove an asset from a BAG file */
int kiavc_bag_remove_asset(kiavc_bag *bag, const char *key) {
	if(!bag || !key)
		return -1;
	if(bag->readonly) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove asset '%s' from BAG, archive is read-only\n", key);
		return -1;
	}
	kiavc_bag_asset *asset = kiavc_map_lookup(bag->map, key);
	if(!asset) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove asset '%s' to BAG, no such key\n", key);
		return -1;
	}
	kiavc_map_remove(bag->map, key);
	bag->list = kiavc_list_remove(bag->list, asset);
	/* Reset all offsets */
	kiavc_list *list = bag->list;
	while(list) {
		asset = (kiavc_bag_asset *)list->data;
		asset->offset = 0;
		list = list->next;
	}
	/* Done */
	return 0;
}

/* Helper to print the contents of a BAG archive */
void kiavc_bag_list(kiavc_bag *bag) {
	if(!bag)
		return;
	SDL_Log("BAG archive v%d.%d.%d\n", bag->major, bag->minor, bag->patch);
	if(!bag->list) {
		SDL_Log("  -- No assets\n");
		return;
	}
	kiavc_list *list = bag->list;
	kiavc_bag_asset *asset = NULL;
	while(list) {
		asset = (kiavc_bag_asset *)list->data;
		SDL_Log("  -- [%010"SCNu32"][%010"SCNu32"] %s\n",
			asset->offset, asset->size, asset->key);
		list = list->next;
	}
}

/* Import a BAG archive from a file */
kiavc_bag *kiavc_bag_import(const char *filename) {
	if(!filename)
		return NULL;
	FILE *file = fopen(filename, "rb");
	if(!file) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open file '%s': %s\n",
			filename, strerror(errno));
		return NULL;
	}
	fseek(file, 0L, SEEK_END);
	long int size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	if(size < SDL_strlen(KIAVC_BAG_HEADER) + sizeof(uint32_t)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid BAG file (too small)\n");
		fclose(file);
		return NULL;
	}
	kiavc_bag *bag = kiavc_bag_create();
	/* Parse the file, starting from the header */
	char bag_header[10];
	if(fread(bag_header, sizeof(char), SDL_strlen(KIAVC_BAG_HEADER), file) != SDL_strlen(KIAVC_BAG_HEADER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG header: %s\n",
			strerror(errno));
		goto error;
	}
	bag_header[SDL_strlen(KIAVC_BAG_HEADER)] = '\0';
	if(strcasecmp(bag_header, KIAVC_BAG_HEADER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid BAG header\n");
		goto error;
	}
	uint32_t version = 0;
	if(fread(&version, sizeof(uint32_t), 1, file) != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG header version: %s\n",
			strerror(errno));
		goto error;
	}
	version = ntohl(version);
	bag->major = version/100000;
	bag->minor = (version - bag->major*10000)/100;
	bag->patch = version - bag->major*10000 - bag->minor*100;
	/* Read the list of files */
	uint32_t header_offset = 0;
	if(fread(&header_offset, sizeof(uint32_t), 1, file) != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG assets header offset: %s\n",
			strerror(errno));
		goto error;
	}
	header_offset = ntohl(header_offset);
	if(header_offset == 0) {
		/* Empty file? We're done */
		fclose(file);
		return bag;
	}
	if(fseek(file, header_offset, SEEK_SET) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't seek to BAG assets header: %s\n",
			strerror(errno));
		goto error;
	}
	uint32_t header_size = 0;
	if(fread(&header_size, sizeof(uint32_t), 1, file) != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG assets header size: %s\n",
			strerror(errno));
		goto error;
	}
	header_size = ntohl(header_size);
	if(header_size == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid BAG assets header size\n");
		goto error;
	}
	/* Iterate on all assets to add them to the BAG */
	uint16_t asset_size = 0;
	uint32_t a_offset = 0, a_size = 0;
	char *path = NULL;
	kiavc_bag_asset *asset = NULL;
	while(header_size > 0) {
		asset_size = 0;
		if(fread(&asset_size, sizeof(uint16_t), 1, file) != 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG asset header size: %s\n",
				strerror(errno));
			goto error;
		}
		if(asset_size == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid BAG asset header size\n");
			goto error;
		}
		header_size -= sizeof(uint16_t);
		asset_size = ntohs(asset_size);
		a_offset = 0;
		a_size = 0;
		if(fread(&a_offset, sizeof(uint32_t), 1, file) != 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG asset offset: %s\n",
				strerror(errno));
			goto error;
		}
		if(a_offset == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid BAG assets header size\n");
			goto error;
		}
		a_offset = ntohl(a_offset);
		header_size -= sizeof(uint32_t);
		asset_size -= sizeof(uint32_t);
		if(fread(&a_size, sizeof(uint32_t), 1, file) != 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG asset size: %s\n",
				strerror(errno));
			goto error;
		}
		if(a_size == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid BAG assets header size\n");
			goto error;
		}
		a_size = ntohl(a_size);
		header_size -= sizeof(uint32_t);
		asset_size -= sizeof(uint32_t);
		if(asset_size == 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG asset path: %s\n",
				strerror(errno));
			goto error;
		}
		path = SDL_malloc(asset_size + 1);
		if(fread(path, sizeof(char), asset_size, file) != asset_size) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't read BAG asset path: %s\n",
				strerror(errno));
			goto error;
		}
		path[asset_size] = '\0';
		asset = kiavc_bag_add_asset_internal(bag, path, path, false);
		if(asset == NULL) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error adding asset '%s' to BAG file\n",
				path);
			SDL_free(path);
			goto error;
		}
		SDL_free(path);
		asset->offset = a_offset;
		asset->size = a_size;
		header_size -= asset_size;
	}
	/* Done */
	bag->file = file;
	bag->readonly = true;
	return bag;

error:
	kiavc_bag_destroy(bag);
	fclose(file);
	return NULL;
}

/* Export a BAG archive to a file */
int kiavc_bag_export(kiavc_bag *bag, const char *filename) {
	if(!bag || !filename)
		return -1;
	/* Create the file */
	FILE *file = fopen(filename, "wb");
	if(!file) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't create file '%s': %s\n",
			filename, strerror(errno));
		return -1;
	}
	/* Start by writing the header */
	long int offset = 0;
	size_t res = fwrite(KIAVC_BAG_HEADER, sizeof(char), SDL_strlen(KIAVC_BAG_HEADER), file);
	if(res != SDL_strlen(KIAVC_BAG_HEADER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the BAG header: %s\n",
			strerror(errno));
		fclose(file);
		return -1;
	}
	offset += SDL_strlen(KIAVC_BAG_HEADER);
	uint32_t version = bag->major * 100000 + bag->minor * 100 + bag->patch;
	version = htonl(version);
	res = fwrite(&version, sizeof(uint32_t), 1, file);
	if(res != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the BAG version: %s\n",
			strerror(errno));
		fclose(file);
		return -1;
	}
	offset += sizeof(uint32_t);
	/* Reserve some space for the header offset */
	uint32_t header_offset = offset, placeholder = 0;
	res = fwrite(&placeholder, sizeof(uint32_t), 1, file);
	if(res != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the BAG header offset placeholder: %s\n",
			strerror(errno));
		fclose(file);
		return -1;
	}
	offset += sizeof(uint32_t);
	/* Iterate on all assets, and write them to file */
	kiavc_list *list = bag->list;
	kiavc_bag_asset *asset = NULL;
	FILE *assetfile = NULL;
	char buffer[4096];
	while(list) {
		asset = (kiavc_bag_asset *)list->data;
		asset->offset = offset;
		/* Open the asset file */
		assetfile = fopen(asset->path ? asset->path : asset->key, "rb");
		SDL_Log("  -- Writing asset '%s' to file\n", asset->key);
		if(!assetfile) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error accessing asset file '%s': %s\n",
				asset->path ? asset->path : asset->key, strerror(errno));
			fclose(file);
		}
		/* Read the bytes and copy them to the BAG archive */
		size_t read = 0, added = 0;
		while((read = fread(buffer, sizeof(char), sizeof(buffer), assetfile)) > 0) {
			size_t written = 0, tot = read;
			while(tot > 0) {
				written = fwrite(buffer+read-tot, sizeof(char), tot, file);
				if(written == 0) {
					SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error writing to file: %s\n", strerror(errno));
					fclose(file);
					return -1;
				}
				tot -= written;
				added += written;
			}
		}
		asset->size = added;
		offset += asset->size;
		list = list->next;
	}
	/* Generate a header object describing the content */
	uint32_t header_size_offset = offset;
	res = fwrite(&placeholder, sizeof(uint32_t), 1, file);
	if(res != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the BAG header size placeholder: %s\n",
			strerror(errno));
		fclose(file);
		return -1;
	}
	offset += sizeof(uint32_t);
	list = bag->list;
	uint32_t asset_header_sizes = 0;
	while(list) {
		asset = (kiavc_bag_asset *)list->data;
		uint16_t offset_size = sizeof(uint32_t) + sizeof(uint32_t) + SDL_strlen(asset->key);
		asset_header_sizes += sizeof(offset_size) + offset_size;
		offset_size = htons(offset_size);
		res = fwrite(&offset_size, sizeof(uint16_t), 1, file);
		if(res != 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the offset size: %s\n",
				strerror(errno));
			fclose(file);
			return -1;
		}
		uint32_t asset_offset = htonl(asset->offset);
		res = fwrite(&asset_offset, sizeof(uint32_t), 1, file);
		if(res != 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the asset offset: %s\n",
				strerror(errno));
			fclose(file);
			return -1;
		}
		uint32_t asset_size = htonl(asset->size);
		res = fwrite(&asset_size, sizeof(uint32_t), 1, file);
		if(res != 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the asset size: %s\n",
				strerror(errno));
			fclose(file);
			return -1;
		}
		res = fwrite(asset->key, sizeof(char), SDL_strlen(asset->key), file);
		if(res != SDL_strlen(asset->key)) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the asset path: %s\n",
				strerror(errno));
			fclose(file);
			return -1;
		}
		offset += sizeof(offset_size) + offset_size;
		list = list->next;
	}
	/* Now let's seek back to the places we left empty */
	fseek(file, header_size_offset, SEEK_SET);
	uint32_t header_size = htonl(asset_header_sizes);
	res = fwrite(&header_size, sizeof(uint32_t), 1, file);
	if(res != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the header size: %s\n",
			strerror(errno));
		fclose(file);
		return -1;
	}
	fseek(file, header_offset, SEEK_SET);
	header_size_offset = htonl(header_size_offset);
	res = fwrite(&header_size_offset, sizeof(uint32_t), 1, file);
	if(res != 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't write the header offset: %s\n",
			strerror(errno));
		fclose(file);
		return -1;
	}
	/* Done */
	fclose(file);
	return 0;
}

/* Export a BAG asset to a file */
int kiavc_bag_asset_export_to_file(kiavc_bag *bag, const char *key, const char *filename) {
	if(!bag || !bag->file || !bag->map || !key || !filename)
		return -1;
	kiavc_bag_asset *asset = kiavc_map_lookup(bag->map, key);
	if(!asset) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No such asset '%s' in BAG\n", key);
		return -1;
	}
	if(asset->size == 0 || asset->offset < 12) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid asset '%s'\n", key);
		return -1;
	}
	/* Seek to where the asset is in the archive */
	if(fseek(bag->file, asset->offset, SEEK_SET) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error seeking to asset '%s' offset\n", key);
		return -1;
	}
	/* Create the file */
	FILE *file = fopen(filename, "wb");
	if(!file) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't create file '%s': %s\n",
			filename, strerror(errno));
		return -1;
	}
	/* Read the asset bytes and copy them to the file */
	size_t read = 0, to_read = asset->size, added = 0;
	char buffer[4096];
	while(to_read > 0 && (read = fread(buffer, sizeof(char), to_read < sizeof(buffer) ? to_read : sizeof(buffer), bag->file)) > 0) {
		size_t written = 0, tot = read;
		while(tot > 0) {
			written = fwrite(buffer+read-tot, sizeof(char), tot, file);
			if(written == 0) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error writing to file: %s\n", strerror(errno));
				fclose(file);
				return -1;
			}
			tot -= written;
			added += written;
		}
		to_read -= read;
	}
	/* Done */
	fclose(file);
	return 0;
}

/* Return a SDL_RWops instance associated with an asset in a BAG file */
SDL_RWops *kiavc_bag_asset_export_rw(kiavc_bag *bag, const char *key) {
	if(!bag || !bag->file || !bag->map || !key)
		return NULL;
	kiavc_bag_asset *asset = kiavc_map_lookup(bag->map, key);
	if(!asset) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No such asset '%s' in BAG\n", key);
		return NULL;
	}
	if(asset->size == 0 || asset->offset < 12) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid asset '%s'\n", key);
		return NULL;
	}
	if(fseek(bag->file, asset->offset, SEEK_SET) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error seeking to asset '%s' offset\n", key);
		return NULL;
	}
	SDL_RWops *rwops = SDL_AllocRW();
	if(rwops) {
		/* Set the callbacks */
		rwops->size = kiavc_bag_rwops_size;
		rwops->seek = kiavc_bag_rwops_seek;
		rwops->read = kiavc_bag_rwops_read;
		rwops->write = NULL;
		rwops->close = kiavc_bag_rwops_close;
		/* We abuse the mem pointers for our needs */
		rwops->hidden.mem.base = (Uint8 *)bag;
		rwops->hidden.mem.here = (Uint8 *)asset;
		rwops->hidden.mem.stop = GUINT_TO_POINTER(0);
		rwops->type = SDL_RWOPS_UNKNOWN;
	}
	/* Done */
	return rwops;
}

/* Static functions only used as callbacks for a custom SDL_RWops */
static Sint64 kiavc_bag_rwops_size(SDL_RWops *rwops) {
	if(!rwops)
		return SDL_SetError("Invalid SDL_RWops instance");
	kiavc_bag_asset *asset = (kiavc_bag_asset *)rwops->hidden.mem.here;
	return (Sint64)asset->size;
}
static Sint64 kiavc_bag_rwops_seek(SDL_RWops *rwops, Sint64 offset, int whence) {
	if(!rwops)
		return SDL_SetError("Invalid SDL_RWops instance");
	kiavc_bag *bag = (kiavc_bag *)rwops->hidden.mem.base;
	kiavc_bag_asset *asset = (kiavc_bag_asset *)rwops->hidden.mem.here;
	uint32_t current = GPOINTER_TO_UINT(rwops->hidden.mem.stop);

	/* Check how we should seek */
	uint32_t newpos = 0;
	switch(whence) {
		case RW_SEEK_SET:
			newpos = asset->offset + offset;
			break;
		case RW_SEEK_CUR:
			newpos = asset->offset + current + offset;
			break;
		case RW_SEEK_END:
			newpos = asset->offset + asset->size + offset;
			break;
		default:
			return SDL_SetError("Unknown value for 'whence'");
	}
	fseek(bag->file, newpos, SEEK_SET);
	rwops->hidden.mem.stop = GUINT_TO_POINTER(newpos - asset->offset);

	/* Done */
	return (Sint64)(newpos - asset->offset);
}
static size_t kiavc_bag_rwops_read(SDL_RWops *rwops, void *ptr, size_t size, size_t maxnum) {
	if(!rwops)
		return 0;
	kiavc_bag *bag = (kiavc_bag *)rwops->hidden.mem.base;
	uint32_t current = GPOINTER_TO_UINT(rwops->hidden.mem.stop);

	/* Read the asset bytes and copy them to the buffer */
	size_t read = 0, to_read = maxnum * size, written = 0;
	while(to_read > 0 && (read = fread(ptr+written, sizeof(char), to_read < maxnum ? to_read : maxnum, bag->file)) > 0) {
		written += read;
		to_read -= read;
	}
	current += written;
	rwops->hidden.mem.stop = GUINT_TO_POINTER(current);

	/* Done */
	return (written/size);
}
static int kiavc_bag_rwops_close(SDL_RWops *rwops) {
	if(rwops)
		SDL_FreeRW(rwops);
	return 0;
}
