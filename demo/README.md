Demo game
=========

This folder contains a simple (incomplete) demo game, whose only purpose is to showcase the engine functionality and show how to use the different features, e.g., in terms of adding rooms, actors, using images, sounds, etc., and how to properly script rooms so that you can wire a simple game.

Notice that by default this folder only contains the `game.kvc` file (where the name of the "game" is saved), the `main.lua` script, the additional scripts in the `game` folder, and an `assets` folder containing files used by the game. To actually start the game, you'll first need to copy or link the `kiavc` (or `kiavc.exe`) executable in this `demo` folder (as you'll need the KIAVC engine to run this game), and then also copy the `engine` folder from the root `lua` folder to the `lua` folder here (as that contains the helper scripts that facilitate interaction with the engine core).

Once you've done that, just launch the engine:

	./kiavc

and the game should start.

## Demo details

The demo is quite basic, as it's made of only two rooms: a city street and some outskirts. The UI is simplified at the moment, and you just use the mouse to navigate around: right click is for looking at things (but it will also move your character if nothing is selected), while left click is used for default actions. The default action for each object is actually defined in the scripts. The game also supports some keyboard input, but only for tweaking some settings, i.e.:

* `1` to `6` allows you to scale the game window;
* `f` allows you to go in and out of fullscreen;
* `.` allows you to skip text when actors are talking;
* `F8` enables the debugging console, from where you can write commands to the KIAVC Lua engine;
* `F9` enables and disables the debugging of objects, which allows you to check the objects interaction area;
* `F10` enables and disables scanlines (which is in the engine on an experimental basis);
* `F11` enables and disables the debugging of walkboxes, which allows you to check where the actor can walk, and the path the engine has calculated any time you click to go somewhere;
* `F12` captures a screenshot;
* `Esc` ends a cutscene, if active, and quits the game otherwise (or, if you're using the debugging console, turns it off).

The game starts in the street, and a cutscene moves the main character automatically up to a certain point, moves them around a bit, and then has them say something: this is meant to showcase how the cutscene mode works, and how you can use a dedicated script for implementing a scenario like this.

The street background is large enough that it will scroll when you move in either direction, and has room layers to implement a parallax effect in the background. When you move on the far left, you'll be sent back automatically until a specific action is performed: this is again something added to showcase one of the engine features, that is "walkbox triggers", that allow you to start a specific script any time the character walks on a specific walkbox. I added this to implement the typical "bouncer scenario" (you're not allowed to go further until a puzzle is solved") but I'm sure there's plenty of other things that can be done with it. The street's walkboxes (which are used for pathfinding) also have automated scaling of size and speed the character when you walk: you can see this going back and forth on the street you came from.

The street has a few objects acting as props, meaning you can interact with them but not pick them up. Some are part of the background (which means we don't provide additional images for them), while the fire uses an animation to be displayed instead. Most importantly, the street also hosts another actor, an NPC you can talk to and interact with: since I'm not an artist, the same exact "costume" is used for this NPC as the main character, but I tried to play around this a bit :grin:  There's also a dialog puzzle in the game, when you talk to the NPC, showcasing how you can choose dialog lines to display, and how as a game developer you're notified about the choice the player made, so that you can do something with it.

Another interesting feature of the street room is that it automatically starts a couple of scripts to add some "color": specifically, it shows some text automatically around a door, and it makes the NPC move around and say things automatically while you play on and independently of you. Both were, again, mostly added to showcase the functionality, which can be useful in different cases.

The demo also has a couple of other rooms. One you can reach going to the left after you solve the "puzzle", and shows some stairs reaching to a temple. When you join it, the game logic is configured to walk you automatically for a bit: the first time you enter the room, some text appears as well. Apart from this, the room is basically empty, as I only added it to test and experiment with vertical scrolling, as opposed to the horizontal scrolling you can see in the street room. The only object you see is a "door" to go back to the street.

The outskirts, instead, is another small room you reach going on the far right of the streets: unlike street and stairs, there's no scrolling, since its background fits the screen. Its only interesting feature is an object you can pick up, and then use with the other objects (and the NPC). The inventory management is very barebones at the moment, and in fact the object will simply appear in the inventory on top when you pick it up: in the future, I plan to make this more flexible, in order to allow game developers to show the inventory any way they want (e.g., always on the bottom, or on demand clicking on a button, or appearing when you hover on top, etc.). Until then, it's ugly but functional for the demo!

Finally, a simple localization is implemented too, and two languages are available: English (default) and Italian. To test the localization features, press `F8` to open the debugging console, and type:

	lang = 'it'

This should have the demo start showing all text (descriptions, dialogues, etc.) in Italian rather than English (`'en'`).

The demo is incomplete since there's no purpose at all, so don't bash your head too hard trying to figure out puzzles that are not there :joy:

## Credits

I'm not an artist, so I relied on the kindness of strangers for the fonts, images and animations you see in the game. More precisely, I mostly used some free-to-use assets made available on [itch.io](https://itch.io/game-assets/free), including (but not limited to):

* https://brokencellgames.itch.io/detective-point-click-character-pack
* https://ansimuz.itch.io/cyberpunk-street-environment
* https://dusan-pavkovic-warlord.itch.io/skadovsk
* https://twitter.com/PixelArtJourney/status/1560261114516029446
* https://cainos.itch.io/pixel-art-icon-pack-rpg
* https://managore.itch.io/m5x7
* https://managore.itch.io/m3x6
* https://fontenddev.com/fonts/notepen/

In case I'm using anything in the demo improperly, please let me know and I'll fix it right away. This is simply meant as a demonstration of the engine, and I most definitely don't want to end up using resources I'm not allowed to.

The music you hear, instead, was indeed written by me: it actually comes from the soundtrack for a never completed game from more than 20 years ago, called "Wonder World", for which I contributed some music track written for the purpose. If you want to listen to some more tunes from that game, they're available as a mix on [SoundCloud](https://soundcloud.com/lminiero/wonder-world-ost).
