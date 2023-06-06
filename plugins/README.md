Plugins
=======

KIAVC now also implements a basic modular architecture, which means you can dynamically load external plugins to add additional features not available in the core. While I still don't have a clear idea of exactly what this could be used for, I implemented a few sample plugins that show how this could be leveraged to add new features, whether it is custom C-based Lua functions, or further rendering of custom resources outside of the boundaries of Lua scripting.

To build the plugins, use

	make

to build the Linux shared objects, or:

	make win32

to build the Windows DLLs. Notice you can also build the plugins from the root of the repo using `make linuxso` (Linux) or `make win32dll` (DLL).

To use a plugin with KIAVC, you first need to copy the `.so` (or `.dll`) in the same folder as the KIAVC executable and then, from Lua, use the `loadPlugin()` function to dynamically load it. For instance, to load the `helloworld` plugin, you can use the following command:

	loadPlugin('helloworld')

which will look for the `libkiavc_helloworld.so` file and dynamically load it. This dynamic loading can be done either as part of the scripts (e.g., `main.lua`), or dynamically using the KIAVC console (accessible via `F8` in the demo). Once a plugin has been loaded, it will be automatically initialized via its `init()` callback (see the "Writing your own plugin" for more information), meaning that after that its features should become immediately available.

## Sample plugins

Just for testing, I implemented a few plugins who don't have any particular purpose other than showcasing how the plugin functionality works for different requirements.

### helloworld plugin

This is a very basic plugin, that only registers a new custom Lua function, called `helloWorld()`: this function does nothing else than print a message on the KIAVC logs.

To test this plugin, load it first:

	loadPlugin('helloworld')

and then use:

	helloWorld()

which should show something like this in the logs:

	INFO: [helloworld] Hello, world!

### drawx plugin

This is another simple plugin, meant to showcase how to perform basic rendering from a plugin. Specifically, it will always just draw the two diagonals on the screen in a continuously changing blue color. To do so, it implements two new Lua functions (`showDiagonals()`, `hideDiagonals()`).

The `showDiagonals()` function, as the name suggests, is what you use to actually have those diagonals appear on the screen. It expects two arguments:

1. the type of rendering to perform ("regular", "after", "last");
2. the z-plane of the rendering.

The type is related to how the KIAVC core and plugins interact as far as rendering is concerned. If a plugin wants to render something, it will tell the core when it should be involved: then, the core will invoke the plugin `render()` function with a pointer to the renderer and info on the canvas size; at this point, the plugin will need to perform whatever rendering it wants to do itself. This means that the plugin needs to tell the core at what step of the rendering it should be involved, which can be:

* `regular`: the plugin will act as a generic resource (an actor, a room layer, an object, text on the screen, etc.), and so its z-plane will decide what will appear behind and in front of it;
* `after`: the plugin will draw things after all resources have been drawn (including mouse, dialogs, etc.), but before scaling to the actual screen size;
* last`: the plugin will draw after scaling, which means the canvas will be the one for the actual screen resolution, and not the game one.

Notice that the provided z-plane only applies to the group the type refers to: a `last` with z-plane 0 will still appear in front of everything drawn in `regular` and `after`.

As such, after the plugin has been loaded:

	loadPlugin('drawx')

a few potential examples of the plugin in use are the following. This snippet will make the diagonals appear behind the room layers, since it will add them to the generic resources and a very low z-plane:

	showDiagonals('regular', -100)

This command, instead, will move the diagonals on top of every game asset shown on screen, but below the post-scaling stuff (e.g., walkboxes, object debugging, console, etc.):

 	showDiagonals('after', 0)

Finally, this snippet acts post-scaling instead, which means the drawn diagonals will look much thinner and sharper due to the higher resolution they're drawn at:

 	showDiagonals('last', 0)

To hide the diagonals, this command can be used:

 	stopDiagonals()

### simplerain plugin

This plugin is another example of dynamic rendering, but for something more complex. Specifically, it implements a basic particle system for showing rain on the screen. To do so, it implements two new Lua functions (`startRain()`, `stopRain()`).

The `startRain()` function uses the provided arguments to create the particles to display and advertise them as a resource to render in the engine. It expects a single Lua object as argument, with a couple of mandatory properties and a few optional ones:

* `width` and `height` specify how large the rain block containing the particles should be; both properties are mandatory, and they'll typically need to match the size of the game screen;
* `distance` is an optional property to specify how distant, in game pixels, each particle should be from one another on average; the lower it is, the more the particles and so the thicker the rain; the default is `10`;
* `type` and `zplane` are optional properties that work exactly as explained for the `drawx` plugin; the defaults are `regular` and `0`.

As such, after the plugin has been loaded:

	loadPlugin('simplerain')

a few potential examples of the plugin in use are the following. This command shows some rain covering the whole game screen using the defaults:

	startRain({ width=320, height=180 })

Notice that this is a static overlay, meaning it will stay there when the room scrolls. Also notice that, when rain has been created, it needs to be stopped before you can try a different configuration:

	stopRain()

This other example changes the z-plane, which means that, since the type is still `regular`, rain will be shown behind some of the room layers:

	startRain({ width=320, height=180, zplane=-100 })

This further example shows a larger amount of rain particles after every other game resource has been drawn, which means it will cover the mouse, UI and other things too:

	startRain({ width=320, height=180, distance=5, type='after' })

Finally, this last example shows rain after scaling has been performed, which as a result will display much thinner particles:

	startRain({ width=1920, height=1080, distance=30, velocity_min=5, velocity_max=10, type='last' })

## Writing your own plugin

If you want to implement a new plugin, check the `plugin.h` header in the `src` folder of KIAVC, as that contains all the bits you need to be aware of to use the proper plugin interface. In a nutshell:

1. you need to fill in your own `kiavc_plugin` instance, and make sure that the related callbacks are implemented (some are mandatory, others are optional);
2. you need to implement a public function called `kiavc_plugin *create(void)` that returns the pointer of that instance;
3. in case you want to register new Lua functions, you use the `register_function` core callback;
4. in case you want to render stuff on the screen, you create `kiavc_plugin_resource` instances and use the `add_resource` / `remove_recourse` core callbacks.

This is a really short version of what needs to be done, and probably not that clear either, but the existing plugin implementations should provide enough information to figure out what's happening and for you to write your own.
