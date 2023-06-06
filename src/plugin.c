/*
 *
 * KIAVC plugins interface implementation. It defines the APIs custom
 * plugins should implement and exposes to be used within the engine.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include <dlfcn.h>

#include <glib.h>

#include "plugin.h"
#include "logger.h"

/* Public function to load a plugin */
kiavc_plugin *kiavc_plugin_load(kiavc_plugin_callbacks *core, const char *name) {
	if(!name) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Invalid plugin name\n");
		return NULL;
	}
	char pluginpath[1024] = {0};
#ifdef _WIN32
	g_snprintf(pluginpath, sizeof(pluginpath)-1, "./libkiavc_%s.dll", name);
#else
	g_snprintf(pluginpath, sizeof(pluginpath)-1, "./libkiavc_%s.so", name);
#endif
	void *p = dlopen(pluginpath, RTLD_NOW | RTLD_GLOBAL);
	if(!p) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open '%s': %s\n",
			pluginpath, dlerror());
		return NULL;
	}
	create_p *create = (create_p *)dlsym(p, "create");
	const char *dlsym_error = dlerror();
	if(!create) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize '%s': %s\n",
			name, dlsym_error);
		dlclose(p);
		return NULL;
	}
	kiavc_plugin *plugin = create();
	if(!plugin) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize '%s': error invoking 'create'\n", name);
		dlclose(p);
		return NULL;
	}
	/* Are all the mandatory methods and callbacks implemented? */
	if(!plugin->init || !plugin->destroy ||
			!plugin->get_api_compatibility ||
			!plugin->get_version || !plugin->get_version_string ||
			!plugin->get_name || !plugin->get_description) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize '%s': missing some mandatory methods/callbacks\n", name);
		dlclose(p);
		return NULL;
	}
	if(plugin->get_api_compatibility() < KIAVC_PLUGIN_API_VERSION) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "The '%s' plugin was compiled against an older version of the API (%d < %d)\n",
			name, plugin->get_api_compatibility(), KIAVC_PLUGIN_API_VERSION);
		dlclose(p);
		return NULL;
	}
	if(plugin->init(core) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize '%s': error invoking 'init'\n", name);
		dlclose(p);
		return NULL;
	}
	SDL_Log("Loaded plugin '%s'\n", plugin->get_name());
	SDL_Log("  -- Description: %s\n", plugin->get_description());
	SDL_Log("  -- Version: %s (%d)\n", plugin->get_version_string(), plugin->get_version());
	SDL_Log("  -- Author: %s\n", plugin->get_author());
	/* Done */
	return plugin;
}

/* Public function to unload a plugin */
void kiavc_plugin_destroy(kiavc_plugin *plugin) {
	if(plugin)
		plugin->destroy();
	/* FIXME Should we dlclose it too? */
}

