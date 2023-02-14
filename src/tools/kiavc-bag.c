/*
 *
 * KIAVC utility to create BAG archives.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "../bag.h"
#include "../version.h"

/* BAG instance we'll write to */
static kiavc_bag *bag = NULL;

static int add_asset(char *path) {
	/* Check if it's a file or a folder */
	struct stat s;
	if(stat(path, &s) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't access path '%s'\n", path);
		return -1;
	}
	if(S_ISDIR(s.st_mode)) {
		/* It's a folder, iterate on its files */
		DIR *dir = opendir(path);
		if(!dir) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't access folder '%s'\n", path);
			return -1;
		}
		struct dirent *ent = NULL;
		char fullpath[1024];
		while((ent = readdir(dir))) {
			if(strlen(ent->d_name) < 4)
				continue;
			fullpath[0] = '\0';
			SDL_snprintf(fullpath, sizeof(fullpath)-1, "%s/%s", path, ent->d_name);
			if(add_asset(fullpath) < 0)
				return -1;
		}
		return 0;
	}
	/* It's a file */
	SDL_Log("  -- Adding asset: %s\n", path);
	return (kiavc_bag_add_asset(bag, path, path) ? 0 : -1);
};

/* Main application */
int main(int argc, char *argv[]) {
	SDL_Log("KIAVC BAG creator v%s\n", KIAVC_VERSION_STRING);

	if(argc < 3) {
		SDL_Log("Usage: %s target.bag file1 [file2 [file3 ... ]]\n", argv[0]);
		exit(0);
	}
	char *bagfile = argv[1], *assetfile = NULL;
	int i = 0, assets = argc - 2;

	bag = kiavc_bag_create();
	for(i=0; i<assets; i++) {
		assetfile = argv[2+i];
		if(add_asset(assetfile) < 0) {
			/* Give up */
			kiavc_bag_destroy(bag);
			exit(1);
		}
	}

	/* Save to file */
	if(kiavc_bag_export(bag, bagfile) < 0) {
		kiavc_bag_destroy(bag);
		exit(1);
	}
	SDL_Log("\n");
	kiavc_bag_list(bag);
	kiavc_bag_destroy(bag);

	/* Show the size of the file */
	FILE *file = fopen(bagfile, "rb");
	if(!file) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open file '%s': %s\n",
			bagfile, strerror(errno));
		exit(1);
	}
	fseek(file, 0L, SEEK_END);
	long int size = ftell(file);
	fseek(file, 0L, SEEK_SET);
	SDL_Log("\n");
	SDL_Log("BAG archive %s is %ld bytes\n", bagfile, size);
	fclose(file);

	/* Done */
	exit(0);
}
