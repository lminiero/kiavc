/*
 *
 * Basic example of a KIAVC plugin, to show how a plugin can be involved
 * in the rendering process by adding a simple rain effect to the screen.
 *
 * Example of plugin use:
 *
 * 		loadPlugin('simplerain')
 *
 * 		startRain({ width=320, height=180 })
 * 		stopRain()
 *
 * 		startRain({ width=320, height=180, distance=10, type='regular', zplane=-100 })
 * 		stopRain()
 *
 * 		startRain({ width=320, height=180, distance=5, type='after' })
 * 		stopRain()
 *
 * 		startRain({ width=1920, height=1080, distance=30, velocity_min=5, velocity_max=10, type='last' })
 * 		stopRain()
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include <stdbool.h>
#include <time.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../../../src/plugin.h"

/* Plugin information */
#define KIAVC_SIMPLERAIN_VERSION		1
#define KIAVC_SIMPLERAIN_VERSION_STRING	"0.0.1"
#define KIAVC_SIMPLERAIN_NAME			"simplerain"
#define KIAVC_SIMPLERAIN_DESCRIPTION	"KIAVC simple rain generator plugin"
#define KIAVC_SIMPLERAIN_AUTHOR			"Lorenzo Miniero (lminiero@gmail.com)"

/* Plugin methods */
kiavc_plugin *create(void);
static int kiavc_simplerain_init(kiavc_plugin_callbacks *core);
static void kiavc_simplerain_destroy(void);
static int kiavc_simplerain_get_api_compatibility(void);
static int kiavc_simplerain_get_version(void);
static const char *kiavc_simplerain_get_version_string(void);
static const char *kiavc_simplerain_get_description(void);
static const char *kiavc_simplerain_get_name(void);
static const char *kiavc_simplerain_get_author(void);
static void kiavc_simplerain_update_world(Uint32 ticks);
static void kiavc_simplerain_render(kiavc_plugin_resource *resource, SDL_Renderer *renderer, int width, int height);

/* Plugin setup */
static kiavc_plugin kiavc_simplerain_plugin =
	KIAVC_PLUGIN_INIT (
		.init = kiavc_simplerain_init,
		.destroy = kiavc_simplerain_destroy,

		.get_api_compatibility = kiavc_simplerain_get_api_compatibility,
		.get_version = kiavc_simplerain_get_version,
		.get_version_string = kiavc_simplerain_get_version_string,
		.get_description = kiavc_simplerain_get_description,
		.get_name = kiavc_simplerain_get_name,
		.get_author = kiavc_simplerain_get_author,

		.update_world = kiavc_simplerain_update_world,
		.render = kiavc_simplerain_render,
	);

/* Plugin creator */
kiavc_plugin *create(void) {
	return &kiavc_simplerain_plugin;
}

/* Local resources */
static kiavc_plugin_callbacks *kc = NULL;
static Uint32 p_ticks = 0;
static kiavc_plugin_resource resource = { 0 };

/* A rain particle */
typedef struct kiavc_simplerain_particle {
	int x, y;
	int vel_x, vel_y;
	int color;
} kiavc_simplerain_particle;
/* List of particles */
static int particles_num = 0;
static kiavc_simplerain_particle *particles = NULL;

/* Helper function to generate a random number */
static int kiavc_simplerain_random_number(int min, int max) {
    return (rand() % (max - min + 1)) + min;
}

/* Plugins can add their own Lua functions, so we add a couple: one to
 * start showing rain on screen, and one to stop doing that instead */
static bool raining = false;
static int kiavc_simplerain_startrain(void *state) {
	lua_State *s = (lua_State *)state;
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Wrong number of arguments: %d (expected %d)\n",
			KIAVC_SIMPLERAIN_NAME, n, exp);
		return 0;
	}
	if(raining) {
		SDL_Log("[%s][Lua] Rain already started\n", KIAVC_SIMPLERAIN_NAME);
		return 0;
	}
	/* Parse the settings from the table */
	luaL_checktype(s, 1, LUA_TTABLE);
	/* width and height specify the size of the rain block */
	lua_getfield(s, 1, "width");
	int width = luaL_checknumber(s, 2);
	lua_getfield(s, 1, "height");
	int height = luaL_checknumber(s, 3);
	if(width < 10 || height < 10) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Invalid width/height parameters (%dx%d)\n",
			KIAVC_SIMPLERAIN_NAME, width, height);
		return 0;
	}
	/* distance tells us how tick the rain should be (default=10) */
	int distance = 10;
	if(lua_getfield(s, 1, "distance") != LUA_TNIL) {
		distance = lua_tointeger(s, 4);
		if(distance < 1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Invalid distance parameter (%d)\n",
				KIAVC_SIMPLERAIN_NAME, distance);
			return 0;
		}
	}
	/* type tells us when to render the plugin resource (default=regular) */
	const char *type = "regular";
	if(lua_getfield(s, 1, "type") != LUA_TNIL)
		type = luaL_checkstring(s, 5);
	if(type && !SDL_strcasecmp(type, "after"))
		resource.rendering = KIAVC_PLUGIN_RENDERING_AFTER;
	else if(type && !SDL_strcasecmp(type, "last"))
		resource.rendering = KIAVC_PLUGIN_RENDERING_LAST;
	else
		resource.rendering = KIAVC_PLUGIN_RENDERING_REGULAR;
	/* zplane tells us the priority of this resource in the group type (default=0) */
	int zplane = 0;
	if(lua_getfield(s, 1, "zplane") != LUA_TNIL)
		zplane = luaL_checknumber(s, 6);
	resource.res.zplane = zplane;
	/* velocity_min and velocity_max dictate how fast rain drops should be (default, min=1, max=2) */
	int vel_min = 1, vel_max = 2;
	if(lua_getfield(s, 1, "velocity_min") != LUA_TNIL)
		vel_min = luaL_checknumber(s, 7);
	if(vel_min < 1)
		vel_min = 1;
	if(lua_getfield(s, 1, "velocity_max") != LUA_TNIL)
		vel_max = luaL_checknumber(s, 8);
	if(vel_max < vel_min)
		vel_max = vel_min;
	/* Initialize the rain particles system */
	SDL_Log("[%s] Starting rain\n", KIAVC_SIMPLERAIN_NAME);
	particles_num = (width/distance) * (height/distance);
	particles = SDL_malloc(sizeof(kiavc_simplerain_particle) * particles_num);
	int i = 0, cols = width/distance, row = 0, col = 0, variance = distance/3;
	if(variance == 0)
		variance = 1;
	for(i=0; i<particles_num; i++) {
		col = i % cols;
		if(i > 0 && col == 0)
			row++;
		(particles + i)->x = col*distance + row + kiavc_simplerain_random_number(-variance, variance);
		(particles + i)->y = row*distance + kiavc_simplerain_random_number(-variance, variance);
		(particles + i)->vel_x = 0;
		(particles + i)->vel_y = kiavc_simplerain_random_number(vel_min, vel_max);
		(particles + i)->color = kiavc_simplerain_random_number(80, 160);
	}
	/* Done, add the resource to be rendered to the core */
	kc->add_resource(&resource);
	raining = true;
	return 0;
}
static int kiavc_simplerain_stoprain(void *state) {
	lua_State *s = (lua_State *)state;
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Wrong number of arguments: %d (expected %d)\n",
			KIAVC_SIMPLERAIN_NAME, n, exp);
		return 0;
	}
	if(raining) {
		SDL_Log("[%s] Stopping rain\n", KIAVC_SIMPLERAIN_NAME);
		kc->remove_resource(&resource);
		raining = false;
		SDL_free(particles);
		particles = NULL;
		particles_num = 0;
	}
	return 0;
}


/* Plugin implementation */
static int kiavc_simplerain_init(kiavc_plugin_callbacks *core) {
	/* This is where you initialize your plugin */
	kc = core;
    srand(time(NULL));
	/* We add our own Lua functions to the engine */
	kc->register_function("startRain", kiavc_simplerain_startrain);
	kc->register_function("stopRain", kiavc_simplerain_stoprain);
	/* In this simple plugin, we use a single static resource for rendering */
	resource.plugin = &kiavc_simplerain_plugin;
	/* Done */
	SDL_Log("[%s] Plugin initialized\n", KIAVC_SIMPLERAIN_NAME);
	return 0;
}

static void kiavc_simplerain_destroy(void) {
	/* This is where you get rid of your plugin resources */
	SDL_Log("[%s] Plugin destroyed\n", KIAVC_SIMPLERAIN_NAME);
}

static int kiavc_simplerain_get_api_compatibility(void) {
	/* Important! This is what your plugin MUST always return:
	 * don't lie here or bad things will happen */
	return KIAVC_PLUGIN_API_VERSION;
}

static int kiavc_simplerain_get_version(void) {
	/* This is where you return your plugin's numeric version */
	return KIAVC_SIMPLERAIN_VERSION;
}

static const char *kiavc_simplerain_get_version_string(void) {
	/* This is where you return your plugin's string version */
	return KIAVC_SIMPLERAIN_VERSION_STRING;
}

static const char *kiavc_simplerain_get_description(void) {
	/* This is where you return your plugin's brief description */
	return KIAVC_SIMPLERAIN_DESCRIPTION;
}

static const char *kiavc_simplerain_get_name(void) {
	/* This is where you return your plugin's unique name */
	return KIAVC_SIMPLERAIN_NAME;
}

static const char *kiavc_simplerain_get_author(void) {
	/* This is where you return your plugin's author info */
	return KIAVC_SIMPLERAIN_AUTHOR;
}

static void kiavc_simplerain_update_world(Uint32 ticks) {
	/* This callback is invoked by the core any time there's a new tick:
	 * this is where we can update our internal state accordingly */
	if(p_ticks == 0)
		p_ticks = ticks;
	if(ticks - p_ticks >= 10) {
		int i = 0;
		for(i=0; i<particles_num; i++) {
			(particles + i)->x += (particles + i)->vel_x;
			(particles + i)->y += (particles + i)->vel_y;
		}
		p_ticks = ticks;
	}
}

static void kiavc_simplerain_render(kiavc_plugin_resource *resource, SDL_Renderer *renderer, int width, int height) {
	/* This callback is invoked by the core any time it's our plugin's
	 * turn to render something as previously requested: this is where
	 * we can draw stuff, for instance the our simple rain cycle */
	if(raining) {
		/* Take note of the current draw color */
		Uint8 r = 0, g = 0, b = 0, a = 0;
		SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
		/* Draw rain particles */
		int i = 0;
		for(i=0; i<particles_num; i++) {
			SDL_SetRenderDrawColor(renderer,
				(particles + i)->color,
				(particles + i)->color,
				(particles + i)->color, 128);
			if((particles + i)->x >= width)
				(particles + i)->x -= width;
			else if((particles + i)->x <= 0)
				(particles + i)->x += width;
			if((particles + i)->y >= height)
				(particles + i)->y -= height;
			else if((particles + i)->y <= 0)
				(particles + i)->x += height;
			SDL_RenderDrawLine(renderer, (particles + i)->x, (particles + i)->y,
				(particles + i)->x, (particles + i)->y + 2);
		}
		/* We're done, restore the original draw color */
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}
}
