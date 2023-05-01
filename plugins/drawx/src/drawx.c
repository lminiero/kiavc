/*
 *
 * Basic example of a KIAVC plugin, to show how a plugin can be involved
 * in the rendering process by drawing coloured diagonals on the screen.
 * Example of plugin use:
 *
 * 		loadPlugin('drawx')
 * 		showDiagonals('regular', -100)
 * 		showDiagonals('after', 0)
 * 		showDiagonals('last', 0)
 * 		stopDiagonals()
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include "../../../src/plugin.h"

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* Plugin information */
#define KIAVC_DRAWX_VERSION			1
#define KIAVC_DRAWX_VERSION_STRING	"0.0.1"
#define KIAVC_DRAWX_NAME			"drawx"
#define KIAVC_DRAWX_DESCRIPTION		"KIAVC diagonal draw (X) plugin"
#define KIAVC_DRAWX_AUTHOR			"Lorenzo Miniero (lminiero@gmail.com)"

/* Plugin methods */
kiavc_plugin *create(void);
static int kiavc_drawx_init(kiavc_plugin_callbacks *core);
static void kiavc_drawx_destroy(void);
static int kiavc_drawx_get_api_compatibility(void);
static int kiavc_drawx_get_version(void);
static const char *kiavc_drawx_get_version_string(void);
static const char *kiavc_drawx_get_description(void);
static const char *kiavc_drawx_get_name(void);
static const char *kiavc_drawx_get_author(void);
static void kiavc_drawx_update_world(Uint32 ticks);
static void kiavc_drawx_render(kiavc_plugin_resource *resource, SDL_Renderer *renderer, int width, int height);

/* Plugin setup */
static kiavc_plugin kiavc_drawx_plugin =
	KIAVC_PLUGIN_INIT (
		.init = kiavc_drawx_init,
		.destroy = kiavc_drawx_destroy,

		.get_api_compatibility = kiavc_drawx_get_api_compatibility,
		.get_version = kiavc_drawx_get_version,
		.get_version_string = kiavc_drawx_get_version_string,
		.get_description = kiavc_drawx_get_description,
		.get_name = kiavc_drawx_get_name,
		.get_author = kiavc_drawx_get_author,

		.update_world = kiavc_drawx_update_world,
		.render = kiavc_drawx_render,
	);

/* Plugin creator */
kiavc_plugin *create(void) {
	return &kiavc_drawx_plugin;
}

/* Local resources */
static kiavc_plugin_callbacks *kc = NULL;
static Uint32 p_ticks = 0;
static int color = 255, increment = -1;
static kiavc_plugin_resource resource = { 0 };

/* Plugins can add their own Lua functions, so we add a couple: one to
 * start drawing diagonals, and one to stop doing that instead */
static bool draw = false;
static int kiavc_drawx_showdiagonals(void *state) {
	lua_State *s = (lua_State *)state;
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Wrong number of arguments: %d (expected %d)\n",
			KIAVC_DRAWX_NAME, n, exp);
		return 0;
	}
	const char *type = luaL_checkstring(s, 1);
	int zplane = luaL_checknumber(s, 2);
	if(type == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Missing rendering type\n", KIAVC_DRAWX_NAME);
		return 0;
	}
	/* This resource can be rendered at different stages, choose which */
	if(!SDL_strcasecmp(type, "after"))
		resource.rendering = KIAVC_PLUGIN_RENDERING_AFTER;
	else if(!SDL_strcasecmp(type, "last"))
		resource.rendering = KIAVC_PLUGIN_RENDERING_LAST;
	else
		resource.rendering = KIAVC_PLUGIN_RENDERING_REGULAR;
	/* This impacts the ordering of the resource in the group type */
	resource.res.zplane = zplane;
	if(draw) {
		/* We were already showing something, so let's remove the resource
		 * before we add it again, to refresh the z-plane if needed */
		SDL_Log("[%s] Updating diagonals (X)\n", KIAVC_DRAWX_NAME);
		kc->remove_resource(&resource);
	} else {
		SDL_Log("[%s] Drawing diagonals (X)\n", KIAVC_DRAWX_NAME);
		draw = true;
	}
	/* Finally, we tell the core to ping us when the resource must be rendered */
	kc->add_resource(&resource);
	return 0;
}
static int kiavc_drawx_hidediagonals(void *state) {
	lua_State *s = (lua_State *)state;
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[%s][Lua] Wrong number of arguments: %d (expected %d)\n",
			KIAVC_DRAWX_NAME, n, exp);
		return 0;
	}
	if(draw) {
		/* We need to tell the core this resource must not be rendered
		 * anymore, to ensure there won't be a render callback for it */
		SDL_Log("[%s] Hiding diagonals (X)\n", KIAVC_DRAWX_NAME);
		kc->remove_resource(&resource);
		draw = false;
	}
	return 0;
}


/* Plugin implementation */
static int kiavc_drawx_init(kiavc_plugin_callbacks *core) {
	/* This is where you initialize your plugin */
	kc = core;
	/* We add our own Lua functions to the engine */
	kc->register_function("showDiagonals", kiavc_drawx_showdiagonals);
	kc->register_function("hideDiagonals", kiavc_drawx_hidediagonals);
	/* In this simple plugin, we use a single static resource for rendering */
	resource.plugin = &kiavc_drawx_plugin;
	/* Done */
	SDL_Log("[%s] Plugin initialized\n", KIAVC_DRAWX_NAME);
	return 0;
}

static void kiavc_drawx_destroy(void) {
	/* This is where you get rid of your plugin resources */
	SDL_Log("[%s] Plugin initialized\n", KIAVC_DRAWX_NAME);
}

static int kiavc_drawx_get_api_compatibility(void) {
	/* Important! This is what your plugin MUST always return:
	 * don't lie here or bad things will happen */
	return KIAVC_PLUGIN_API_VERSION;
}

static int kiavc_drawx_get_version(void) {
	/* This is where you return your plugin's numeric version */
	return KIAVC_DRAWX_VERSION;
}

static const char *kiavc_drawx_get_version_string(void) {
	/* This is where you return your plugin's string version */
	return KIAVC_DRAWX_VERSION_STRING;
}

static const char *kiavc_drawx_get_description(void) {
	/* This is where you return your plugin's brief description */
	return KIAVC_DRAWX_DESCRIPTION;
}

static const char *kiavc_drawx_get_name(void) {
	/* This is where you return your plugin's unique name */
	return KIAVC_DRAWX_NAME;
}

static const char *kiavc_drawx_get_author(void) {
	/* This is where you return your plugin's author info */
	return KIAVC_DRAWX_AUTHOR;
}

static void kiavc_drawx_update_world(Uint32 ticks) {
	/* This callback is invoked by the core any time there's a new tick:
	 * this is where we can update our internal state accordingly */
	if(p_ticks == 0)
		p_ticks = ticks;
	if(ticks - p_ticks >= 20) {
		if(color == 0)
			increment = 1;
		else if(color == 255)
			increment = -1;
		color += increment;
	}
}

static void kiavc_drawx_render(kiavc_plugin_resource *resource, SDL_Renderer *renderer, int width, int height) {
	/* This callback is invoked by the core any time it's our plugin's
	 * turn to render something as previously requested: this is where
	 * we can draw stuff, for instance the our simple diagonal lines */
	if(draw) {
		/* Take note of the current draw color */
		Uint8 r = 0, g = 0, b = 0, a = 0;
		SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
		/* Draw the two diagonals using the specified color */
		SDL_SetRenderDrawColor(renderer, 0, color, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLine(renderer, 0, 0, width, height);
		SDL_RenderDrawLine(renderer, width, 0, 0, height);
		/* We're done, restore the original draw color */
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}
}
