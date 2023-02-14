/*
 *
 * KIAVC utility to extract BAG archives.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <stdlib.h>
#include <errno.h>

#include "../bag.h"
#include "../version.h"

/* Main application */
int main(int argc, char *argv[]) {
	SDL_Log("KIAVC BAG extractor v%s\n", KIAVC_VERSION_STRING);

	if(argc != 2 && argc != 3) {
		SDL_Log("Usage: [--parse] %s target.bag\n", argv[0]);
		exit(0);
	}
	bool parseonly = false;
	char *bagfile = NULL;
	if(argc == 2) {
		bagfile = argv[1];
	} else {
		if(strcasecmp(argv[1], "--parse") && strcasecmp(argv[2], "--parse")) {
			SDL_Log("Usage: [--parse] %s target.bag\n", argv[0]);
			exit(1);
		}
		parseonly = true;
		if(!strcasecmp(argv[1], "--parse")) {
			bagfile = argv[2];
		} else {
			bagfile = argv[1];
		}
	}
	kiavc_bag *bag = kiavc_bag_import(bagfile);
	if(!bag)
		exit(1);

	SDL_Log("\n");
	kiavc_bag_list(bag);
	if(parseonly) {
		/* Don't extract the files */
		kiavc_bag_destroy(bag);
		exit(0);
	}
	/* TODO Extract files */
	SDL_Log("Extracting BAG archive files\n");
	//~ kiavc_bag_asset_export_to_file(bag, "./assets/images/fotaqrio.png", "test.png");
	kiavc_bag_destroy(bag);

	/* Done */
	exit(0);
}
