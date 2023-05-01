/*
 *
 * KIAVC plugins interface implementation. It defines the APIs custom
 * plugins should implement and exposes to be used within the engine.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef KIAVC_PLUGINS_H
#define KIAVC_PLUGINS_H

#include <SDL2/SDL.h>

#include "resources.h"

/* Plugin API version, to detect mismatches between engine and plugins */
#define KIAVC_PLUGIN_API_VERSION	1

#define KIAVC_PLUGIN_INIT(...) {		\
		.init = NULL,					\
		.destroy = NULL,				\
		.get_api_compatibility = NULL,	\
		.get_version = NULL,			\
		.get_version_string = NULL,		\
		.get_name = NULL,				\
		.get_description = NULL,		\
		.get_author = NULL,				\
		.update_world = NULL,			\
		.render = NULL,					\
		## __VA_ARGS__ }

typedef struct kiavc_plugin_callbacks kiavc_plugin_callbacks;
typedef struct kiavc_plugin kiavc_plugin;
typedef struct kiavc_plugin_resource kiavc_plugin_resource;
typedef enum kiavc_plugin_rendering kiavc_plugin_rendering;

/* This is the plugin interface, that custom plugins should implement */
struct kiavc_plugin {
	/* Plugin initialization/constructor */
	int (* const init)(kiavc_plugin_callbacks *core);
	/* Plugin deinitialization/destructor */
	void (* const destroy)(void);

	/* Informative method to request the API version this plugin was compiled against */
	int (* const get_api_compatibility)(void);
	/*! Informative method to request the numeric version of the plugin */
	int (* const get_version)(void);
	/*! Informative method to request the string version of the plugin */
	const char *(* const get_version_string)(void);
	/*! Informative method to request the name of the plugin */
	const char *(* const get_name)(void);
	/*! Informative method to request a description of the plugin */
	const char *(* const get_description)(void);
	/*! Informative method to request the author of the plugin */
	const char *(* const get_author)(void);

	/* This method is called when the the world is updated */
	void (* const update_world)(Uint32 ticks);
	/* This method is called when the plugin is supposed to render stuff */
	void (* const render)(kiavc_plugin_resource *resource, SDL_Renderer *renderer, int width, int height);
};
/* The hook that plugins need to implement to be created */
typedef kiavc_plugin *create_p(void);

/* This is the core interfact, that plugins can use to talk to the core */
struct kiavc_plugin_callbacks {
	/* Helper function to register a new script function */
	void (* const register_function)(const char *name, int (* const function)(void *s));
	/* Helper function to add a new plugin resource to render along others */
	void (* const add_resource)(kiavc_plugin_resource *resource);
	/* Helper function to remove a previously added plugin resource */
	void (* const remove_resource)(kiavc_plugin_resource *resource);
};

/* This helps defining at what step of the rendering the resource should
 * be drawn. Notice that, in call cases, z-plane in the resource helps
 * sorting when this should be drawn in the same group. When z-plane
 * needs to be updated, remove() and add() must be called to enforce it */
enum kiavc_plugin_rendering {
	/* Never (plugin->render() will not be called) */
	KIAVC_PLUGIN_RENDERING_NONE = 0,
	/* Along other resources */
	KIAVC_PLUGIN_RENDERING_REGULAR,
	/* After everything's been drawn, but before scaling */
	KIAVC_PLUGIN_RENDERING_AFTER,
	/* After everything's been drawn, but before scaling */
	KIAVC_PLUGIN_RENDERING_LAST,
};

/* This is a plugin resource, an implementation of kiavc_resource that
 * can be tied to a specific instance of something in one of the plugins*/
struct kiavc_plugin_resource {
	/* Common resource info */
	kiavc_resource res;
	/* Plugin owning the resource */
	kiavc_plugin *plugin;
	/* When this resource should be rendered */
	kiavc_plugin_rendering rendering;
};


/* Public function to load a plugin */
kiavc_plugin *kiavc_plugin_load(kiavc_plugin_callbacks *core, const char *name);
/* Public function to unload a plugin */
void kiavc_plugin_destroy(kiavc_plugin *plugin);

#endif
