/*
 *
 * Basic example of a KIAVC plugin. Does nothing meaningful, except
 * registering a new Lua function that only prints a string on the log.
 * Example of plugin use:
 *
 * 		loadPlugin('helloworld')
 * 		helloWorld()
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include "../../../src/plugin.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* Plugin information */
#define KIAVC_HELLOWORLD_VERSION		1
#define KIAVC_HELLOWORLD_VERSION_STRING	"0.0.1"
#define KIAVC_HELLOWORLD_NAME			"helloworld"
#define KIAVC_HELLOWORLD_DESCRIPTION	"KIAVC dumb helloworld plugin"
#define KIAVC_HELLOWORLD_AUTHOR			"Lorenzo Miniero (lminiero@gmail.com)"

/* Plugin methods */
kiavc_plugin *create(void);
static int kiavc_helloworld_init(kiavc_plugin_callbacks *core);
static void kiavc_helloworld_destroy(void);
static int kiavc_helloworld_get_api_compatibility(void);
static int kiavc_helloworld_get_version(void);
static const char *kiavc_helloworld_get_version_string(void);
static const char *kiavc_helloworld_get_description(void);
static const char *kiavc_helloworld_get_name(void);
static const char *kiavc_helloworld_get_author(void);

/* Plugin setup */
static kiavc_plugin kiavc_helloworld_plugin =
	KIAVC_PLUGIN_INIT (
		.init = kiavc_helloworld_init,
		.destroy = kiavc_helloworld_destroy,

		.get_api_compatibility = kiavc_helloworld_get_api_compatibility,
		.get_version = kiavc_helloworld_get_version,
		.get_version_string = kiavc_helloworld_get_version_string,
		.get_description = kiavc_helloworld_get_description,
		.get_name = kiavc_helloworld_get_name,
		.get_author = kiavc_helloworld_get_author,
	);

/* Plugin creator */
kiavc_plugin *create(void) {
	return &kiavc_helloworld_plugin;
}

/* Local resources */
static kiavc_plugin_callbacks *kc = NULL;


/* Plugins can add their own Lua functions: we add a very simple one,
 * called helloWorld(), that just prints something on the KIAVC logs */
static int kiavc_helloworld_helloworld(void *state) {
	lua_State *s = (lua_State *)state;
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Wrong number of arguments: %d (expected %d)\n",
			KIAVC_HELLOWORLD_NAME, n, exp);
		return 0;
	}
	SDL_Log("[%s] Hello, world!\n", KIAVC_HELLOWORLD_NAME);
	return 0;
}


/* Plugin implementation */
static int kiavc_helloworld_init(kiavc_plugin_callbacks *core) {
	/* This is where you initialize your plugin */
	kc = core;
	/* As an example, we add our own Lua function to the engine */
	kc->register_function("helloWorld", kiavc_helloworld_helloworld);
	/* Done */
	SDL_Log("[%s] Plugin initialized\n", KIAVC_HELLOWORLD_NAME);
	return 0;
}

static void kiavc_helloworld_destroy(void) {
	/* This is where you get rid of your plugin resources */
	SDL_Log("[%s] Plugin initialized\n", KIAVC_HELLOWORLD_NAME);
}

static int kiavc_helloworld_get_api_compatibility(void) {
	/* Important! This is what your plugin MUST always return:
	 * don't lie here or bad things will happen */
	return KIAVC_PLUGIN_API_VERSION;
}

static int kiavc_helloworld_get_version(void) {
	/* This is where you return your plugin's numeric version */
	return KIAVC_HELLOWORLD_VERSION;
}

static const char *kiavc_helloworld_get_version_string(void) {
	/* This is where you return your plugin's string version */
	return KIAVC_HELLOWORLD_VERSION_STRING;
}

static const char *kiavc_helloworld_get_description(void) {
	/* This is where you return your plugin's brief description */
	return KIAVC_HELLOWORLD_DESCRIPTION;
}

static const char *kiavc_helloworld_get_name(void) {
	/* This is where you return your plugin's unique name */
	return KIAVC_HELLOWORLD_NAME;
}

static const char *kiavc_helloworld_get_author(void) {
	/* This is where you return your plugin's author info */
	return KIAVC_HELLOWORLD_AUTHOR;
}
