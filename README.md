KIAVC
=====

KIAVC is an open source engine to create point and click adventure games, developed by [Lorenzo Miniero](https://github.com/lminiero/). Officially it stands for "KIAVC Is an Adventure Videogame Creator", but in practice I just called it like that because it's a Neapolitan word that's pretty much equivalent to what [SCUMM](https://en.wikipedia.org/wiki/SCUMM) means in English :grin:  The reference to SCUMM isn't a coincidence, as this whole effort is basically a love letter to Ron Gilbert and all the people who wrote (and are writing) the adventure games we love.

Notice that this was born as a pet project, and an opportunity for me to study and learn more about the development of a game engine. As such, it should be considered experimental, rather than something you can really rely upon. If you just want to write a game and sell it, there are far better and more robust options out there.

If you want to learn more about the engine internals or its capabilities, please check the [blog](https://kiavc.wordpress.com), where I try to more or less regularly post content.

## Dependencies

To compile KIAVC, you'll need the following dependencies (development versions):

* [SDL2](https://github.com/libsdl-org/SDL)
* [SDL2_image](https://github.com/libsdl-org/SDL_image)
* [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)
* [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf)
* [GLib2](https://docs.gtk.org/glib/)
* [Lua](https://www.lua.org/download.html)

On Linux, most of these dependencies are commonly available in distros. This is how you can install them on Fedora, for instance:

	dnf install make gcc pkg-config SDL2-devel SDL2_image-devel \
		SDL2_mixer-devel SDL2_ttf-devel glib2-devel lua-devel

On Ubuntu, instead, you can install them like this:

	apt-get -y install make gcc pkg-config libsdl2-dev libsdl2-image-dev \
		libsdl2-mixer-dev libsdl2-ttf-dev liblua5.4-dev

> **Note**: Depending on the version of Ubuntu, Lua 5.4 may or may not be available: in case it isn't, 5.3 is a viable option as well. Besides, notice that, depending on which Lua version is installed and which one is considered the default in the `alternatives`, `pkg-config` may not be able to find the library using the `lua` name: in that case, just edit the `Makefile` and edit both the `DEPS` and `DEPS_LIBS` variables so that they reference what `pkg-config` knows about instead (e.g., `lua53`).

## Compiling KIAVC

Once you have installed all the dependencies, get the code:

	git clone https://github.com/lminiero/kiavc.git
	cd kiavc

Then just type:

	make

to start compiling the code.

Notice that the default target is `linux`. If you want to cross-compile to a Windows executable instead, type:

	make win32

This uses `i686-w64-mingw32-gcc` to cross-compile from Windows, so you'll need the above mentioned dependencies in MingW. At the time of writing, there's no specific Makefile target to build directly from Windows instead: contributions in that sense would be welcome!

I'm not sure if the existing Makefile can be used on MacOS too, but it might. In case you're aware of ways to cross-compile a MacOS executable from Linux as well, that would be appreciated too.

## Launching KIAVC

To launch the engine, just type:

	./kiavc

This will have the engine automatically try and look for a `game.kvc` (containing the name of the game) and a `main.lua` script in the `lua` folder, to initialize the actual game. In case those files are not available, the engine will exit with an error.

The `demo` folder contains a demo you can use to test the engine, so follow the instructions there to try it out.

## Packaging game files

Notice that, by default, the engine expects the files to be available on the disk in subfolders (e.g., `game.kvc`, `lua` and `assets`). In case you want to package the game files in an archive instead, you can use the `kiavc-bag` tool, which will create a BAG file that you can pass to the engine.

To create a BAG file, type something like this:

	./kiavc-bag assets.bag ./game.kvc ./lua ./assets

The above command will take all files in the `lua` and `assets` subfolders, and package them in the `assets.bag` file. Notice that the tool only works with files in subfolders, and not with files in arbitrary positions. Relative paths should be used as well, or this will cause problems when used elsewhere.

Once an asset file is ready, it can be used by the engine by passing it as a command line argument:

	./kiavc assets.bag

This will tell the engine to load all files from the archive, rather than from disk.

## Documentation

Sadly, no documentation is available at the moment: the README in the `lua` folder contains some information on how to start working on a script, though. Detailed articles on the engine internals (which also includes some examples) are often posted on [this blog](https://kiavc.wordpress.com) as well. Besides, a sample `main.lua` (plus some assets) is available as a reference in the `demo` folder too too, to showcase the engine functionality in a more practical way: it's probably buggy and incomplete (it's all WIP, after all), but it should give a good starting point.
