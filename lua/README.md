KIAVC scripting
===============

The Lua folder is the heart of the KIAVC scripting functionality. In fact, while the engine is written in C, the actual games implemented on top of the engine must be written in Lua instead, where the logic to use the engine functionality will reside. Specifically, you must provide at least one file called `main.lua`, which acts as the entry point for the game. A demo game example is provided in the `demo` folder (you can check the `README.md` there for more information), but notice it's just an example, not what you HAVE to do.

A typical structure for testing the engine is the following:

	./assets/
	-- fonts/ (contains TTF files)
	-- images/ (contains images and animations)
	-- music/ (contains music tracks)
	-- soundfx/ (contains sound effects)
	./lua/
	-- main.lua (main script, entry point)
	-- engine/ (contains repo scripts, loaded automatically)
	-- game/ (contains game scripts, loaded by main.lua)
	./kiavc

Of course this is just an example (especially with respect to the `assets` and `game` folders) and what I did for the demo, but what's important is that the `lua` folder is in the same folder as the `kiavc` executable, and that `main.lua` is in the `lua` folder.

As for the content of the `main.lua` file, that's entirely up to the game developer, even though there are a few things it should do (or kickstart):

* register all resources (i.e., images, animations, audio files, fonts, etc.)
* initialize engine concepts like rooms, actors, objects, etc.
* initializing the game resolution, scaling, and framerate
* optionally setting window title and/or icon (the engine will use some defaults, if missing)
* implement some relevant callbacks (e.g., for when the engine tells us about hovering on an object, or a dialog line that has been selected)
* set some defaults (e.g., room to start from, active actor, position, etc.)

Notice that, while Lua provides a `require` keyword for importing other scripts, it should not be used in KIAVC scripts, since game files may actually be packaged in BAG files, and so loaded in a custom way. As such, you should use the `kiavcRequire` keyword instead, e.g.:

	-- Let's load all the resources first
	kiavcRequire('game/resources')

Also notice that there's no need to manually load the scripts in the `engine` subfolder, as they're automatically loaded by the engine itself before loading the `main.lua` script. Those scripts provide classes for facilitating the initialization and management of file resources (images, sounds, etc.) and game concepts (rooms, actors, objects, etc.).
