/*
 *
 * KIAVC scripting interface, specifically using Lua. The interfaces
 * with the engine are abstracted so that in theory different scripting
 * languages can be used in the future, even though they're mostly
 * tailored to be used with Lua. Functions implemented in the engine
 * are exposed here as Lua functions, and bubbled up to the engine via
 * callbacks that just contain the provided data.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <SDL2/SDL.h>

#include "engine.h"
#include "scripts.h"
#include "version.h"

/* Lua state */
static lua_State *lua_state = NULL;

/* Callbacks to the main application */
static const kiavc_scripts_callbacks *kiavc_cb = NULL;

/* Methods that we expose to the Lua script */
/* Load a script from the assets */
static int kiavc_lua_method_kiavcrequire(lua_State *s);
/* Return version as major/minor/patch */
static int kiavc_lua_method_getversion(lua_State *s);
/* Return version string */
static int kiavc_lua_method_getversionstring(lua_State *s);
/* Logging, to use the same SDL-based logging as the rest of the application */
static int kiavc_lua_method_kiavclog(lua_State *s);
/* Error logging, to use the same SDL-based logging as the rest of the application */
static int kiavc_lua_method_kiavcerror(lua_State *s);
/* Warning logging, to use the same SDL-based logging as the rest of the application */
static int kiavc_lua_method_kiavcwarn(lua_State *s);
/* Set resolution and scaling */
static int kiavc_lua_method_setresolution(lua_State *s);
/* Set window title */
static int kiavc_lua_method_settitle(lua_State *s);
/* Set window icon */
static int kiavc_lua_method_seticon(lua_State *s);
/* Set whether we should grab the mouse and confine it to the window */
static int kiavc_lua_method_grabmouse(lua_State *s);
/* Check whether we're grabbing the mouse and confining it to the window */
static int kiavc_lua_method_isgrabbingmouse(lua_State *s);
/* Set fullscreen mode */
static int kiavc_lua_method_setfullscreen(lua_State *s);
/* Check the fullscreen mode */
static int kiavc_lua_method_getfullscreen(lua_State *s);
/* Set scanlines mode */
static int kiavc_lua_method_setscanlines(lua_State *s);
/* Check the scanlines mode */
static int kiavc_lua_method_getscanlines(lua_State *s);
/* Set whether to debug objects or not */
static int kiavc_lua_method_debugobjects(lua_State *s);
/* Check whether objects debugging is on or not */
static int kiavc_lua_method_isdebuggingobjects(lua_State *s);
/* Set whether to debug walkboxes or not */
static int kiavc_lua_method_debugwalkboxes(lua_State *s);
/* Check whether walkboxes debugging is on or not */
static int kiavc_lua_method_isdebuggingwalkboxes(lua_State *s);
/* Save a screenshot */
static int kiavc_lua_method_savescreenshot(lua_State *s);
/* Enable the console and specify which font to use */
static int kiavc_lua_method_enableconsole(lua_State *s);
/* Show the console */
static int kiavc_lua_method_showconsole(lua_State *s);
/* Hide the console */
static int kiavc_lua_method_hideconsole(lua_State *s);
/* Disable the console */
static int kiavc_lua_method_disableconsole(lua_State *s);
/* Check if the console is enabled */
static int kiavc_lua_method_isconsoleenabled(lua_State *s);
/* Check if the console is visible */
static int kiavc_lua_method_isconsolevisible(lua_State *s);
/* Enable input from the user */
static int kiavc_lua_method_enableinput(lua_State *s);
/* Disable input from the user */
static int kiavc_lua_method_disableinput(lua_State *s);
/* Check if input from the user is enabled */
static int kiavc_lua_method_isinputenabled(lua_State *s);
/* Start a cutscene */
static int kiavc_lua_method_startcutscene(lua_State *s);
/* Stop a cutscene */
static int kiavc_lua_method_stopcutscene(lua_State *s);
/* Fade in */
static int kiavc_lua_method_fadein(lua_State *s);
/* Fade out */
static int kiavc_lua_method_fadeout(lua_State *s);
/* Start a new dialog session */
static int kiavc_lua_method_startdialog(lua_State *s);
/* Add a line to a dialog session */
static int kiavc_lua_method_adddialogline(lua_State *s);
/* Stop a dialog session */
static int kiavc_lua_method_stopdialog(lua_State *s);
/* Register a new animation in the engine */
static int kiavc_lua_method_registeranimation(lua_State *s);
/* Register a new font in the engine */
static int kiavc_lua_method_registerfont(lua_State *s);
/* Register a new cursor in the engine */
static int kiavc_lua_method_registercursor(lua_State *s);
/* Set the current animation for a cursor */
static int kiavc_lua_method_setcursoranimation(lua_State *s);
/* Set the main cursor */
static int kiavc_lua_method_setmaincursor(lua_State *s);
/* Set the hotspot cursor */
static int kiavc_lua_method_sethotspotcursor(lua_State *s);
/* Show a cursor */
static int kiavc_lua_method_showcursor(lua_State *s);
/* Hide a cursor */
static int kiavc_lua_method_hidecursor(lua_State *s);
/* Show a cursor text */
static int kiavc_lua_method_showcursortext(lua_State *s);
/* Hide cursor text */
static int kiavc_lua_method_hidecursortext(lua_State *s);
/* Register an new audio track in the engine */
static int kiavc_lua_method_registeraudio(lua_State *s);
/* Play an audio track in the engine */
static int kiavc_lua_method_playaudio(lua_State *s);
/* Pause an audio track in the engine */
static int kiavc_lua_method_pauseaudio(lua_State *s);
/* Resume an audio track in the engine */
static int kiavc_lua_method_resumeaudio(lua_State *s);
/* Stop an audio track in the engine */
static int kiavc_lua_method_stopaudio(lua_State *s);
/* Register a new room in the engine */
static int kiavc_lua_method_registerroom(lua_State *s);
/* Set the current background for a room */
static int kiavc_lua_method_setroombackground(lua_State *s);
/* Add a room layer */
static int kiavc_lua_method_addroomlayer(lua_State *s);
/* Remove a room layer */
static int kiavc_lua_method_removeroomlayer(lua_State *s);
/* Add a room walkbox */
static int kiavc_lua_method_addroomwalkbox(lua_State *s);
/* Enable a room walkbox */
static int kiavc_lua_method_enableroomwalkbox(lua_State *s);
/* Disable a room walkbox */
static int kiavc_lua_method_disableroomwalkbox(lua_State *s);
/* Recalculate walkboxes in a room */
static int kiavc_lua_method_recalculateroomwalkboxes(lua_State *s);
/* Set the active room */
static int kiavc_lua_method_showroom(lua_State *s);
/* Register a new actor in the engine */
static int kiavc_lua_method_registeractor(lua_State *s);
/* Set the current costume for an actor */
static int kiavc_lua_method_setactorcostume(lua_State *s);
/* Move an actor to a specific room */
static int kiavc_lua_method_moveactorto(lua_State *s);
/* Show an actor in the room they're in */
static int kiavc_lua_method_showactor(lua_State *s);
/* Follow an actor in the room they're in */
static int kiavc_lua_method_followactor(lua_State *s);
/* Hide an actor in the room they're in */
static int kiavc_lua_method_hideactor(lua_State *s);
/* Fade an actor in */
static int kiavc_lua_method_fadeactorin(lua_State *s);
/* Fade an actor out */
static int kiavc_lua_method_fadeactorout(lua_State *s);
/* Fade an actor to a specific alpha */
static int kiavc_lua_method_fadeactorto(lua_State *s);
/* Set the alpha for the actor */
static int kiavc_lua_method_setactoralpha(lua_State *s);
/* Set the z-plane for the actor */
static int kiavc_lua_method_setactorplane(lua_State *s);
/* Set the movement speed for the actor */
static int kiavc_lua_method_setactorspeed(lua_State *s);
/* Scale an actor */
static int kiavc_lua_method_scaleactor(lua_State *s);
/* Walk an actor to some coordinates */
static int kiavc_lua_method_walkactorto(lua_State *s);
/* Have an actor say something */
static int kiavc_lua_method_sayactor(lua_State *s);
/* Change the actor's direction */
static int kiavc_lua_method_setactordirection(lua_State *s);
/* Set the currently controlled actor */
static int kiavc_lua_method_controlledactor(lua_State *s);
/* Skip the text any actor is saying */
static int kiavc_lua_method_skipactorstext(lua_State *s);
/* Set a specific state for an actor */
static int kiavc_lua_method_setactorstate(lua_State *s);
/* Register a new custome in the engine */
static int kiavc_lua_method_registercostume(lua_State *s);
/* Set the current animation for a costume */
static int kiavc_lua_method_setcostumeanimation(lua_State *s);
/* Register a new object in the engine */
static int kiavc_lua_method_registerobject(lua_State *s);
/* Set the current animation for an object */
static int kiavc_lua_method_setobjectanimation(lua_State *s);
/* Mark whether this object is interactable */
static int kiavc_lua_method_setobjectinteractable(lua_State *s);
/* Mark whether this object iss part of the UI */
static int kiavc_lua_method_setobjectui(lua_State *s);
/* Set the UI position for this object */
static int kiavc_lua_method_setobjectuiposition(lua_State *s);
/* Set the current animation for an object, when part of the UI */
static int kiavc_lua_method_setobjectuianimation(lua_State *s);
/* Set the parent for an object (start relative positioning), when part of the UI */
static int kiavc_lua_method_setobjectparent(lua_State *s);
/* Remove the parent for an object (stop relative positioning), when part of the UI */
static int kiavc_lua_method_removeobjectparent(lua_State *s);
/* Move an object to a specific room */
static int kiavc_lua_method_moveobjectto(lua_State *s);
/* Float an object at some coordinates at a certain speed */
static int kiavc_lua_method_floatobjectto(lua_State *s);
/* Specify the hover coordinates for an object */
static int kiavc_lua_method_setobjecthover(lua_State *s);
/* Show an object in the room they're in */
static int kiavc_lua_method_showobject(lua_State *s);
/* Hide an object in the room they're in */
static int kiavc_lua_method_hideobject(lua_State *s);
/* Fade an object in */
static int kiavc_lua_method_fadeobjectin(lua_State *s);
/* Fade an object out */
static int kiavc_lua_method_fadeobjectout(lua_State *s);
/* Fade an object to a specific alpha */
static int kiavc_lua_method_fadeobjectto(lua_State *s);
/* Set the alpha for the object */
static int kiavc_lua_method_setobjectalpha(lua_State *s);
/* Set the z-plane for the object */
static int kiavc_lua_method_setobjectplane(lua_State *s);
/* Set the state for the object */
static int kiavc_lua_method_setobjectstate(lua_State *s);
/* Scale an object */
static int kiavc_lua_method_scaleobject(lua_State *s);
/* Add an object to an actor's inventory */
static int kiavc_lua_method_addobjecttoinventory(lua_State *s);
/* Remove an object from an actor's inventory */
static int kiavc_lua_method_removeobjectfrominventory(lua_State *s);
/* Show some text at some coordinates for some time */
static int kiavc_lua_method_showtext(lua_State *s);
/* Float some text at some coordinates at a certain speed */
static int kiavc_lua_method_floattextto(lua_State *s);
/* Fade rendered text in */
static int kiavc_lua_method_fadetextin(lua_State *s);
/* Fade rendered text out */
static int kiavc_lua_method_fadetextout(lua_State *s);
/* Fade rendered text to a specific alpha */
static int kiavc_lua_method_fadetextto(lua_State *s);
/* Set the alpha for the rendered text */
static int kiavc_lua_method_settextalpha(lua_State *s);
/* Remove rendered text, if an ID had been provided */
static int kiavc_lua_method_removetext(lua_State *s);
/* Quit */
static int kiavc_lua_method_quit(lua_State *s);

/* Helper to print the content of the Lua stack for debugging purposes
 * Adapted from https://stackoverflow.com/a/59097940 */
static void kiavc_scripts_dumpstack(lua_State *s) {
	int top = lua_gettop(s), i = 1;
	for(i=1; i<=top; i++) {
	switch(lua_type(s, i)) {
		case LUA_TNUMBER:
			SDL_Log("[%d] %s: %g\n", i, luaL_typename(s, i), lua_tonumber(s, i));
			break;
		case LUA_TSTRING:
			SDL_Log("[%d] %s: %s\n", i, luaL_typename(s, i), lua_tostring(s, i));
			break;
		case LUA_TBOOLEAN:
			SDL_Log("[%d] %s: %s\n", i, luaL_typename(s, i), (lua_toboolean(s, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			SDL_Log("[%d] %s: nil\n", i, luaL_typename(s, i));
			break;
		default:
			SDL_Log("[%d] %s: %p\n", i, luaL_typename(s,i), lua_topointer(s, i));
			break;
		}
	}
}

/* Helper function to read scripts from files to strings */
static char *kiavc_scripts_open_file(const char *path) {
	if(!path)
		return NULL;
	SDL_RWops *rwops = kiavc_engine_open_file(path);
	if(!rwops)
		return NULL;
	SDL_RWseek(rwops, 0, RW_SEEK_END);
	Sint64 size = SDL_RWtell(rwops);
	SDL_RWseek(rwops, 0, RW_SEEK_SET);
	if(size < 1) {
		SDL_RWclose(rwops);
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Invalid Lua script size '%"SCNi64"'\n", size);
		return NULL;
	}
	char *script = SDL_malloc(size+1);
	size_t read = 0, to_read = size, written = 0;
	while(to_read > 0 && (read = SDL_RWread(rwops, script+written, sizeof(char), to_read < size ? to_read : size)) > 0) {
		written += read;
		to_read -= read;
	}
	SDL_RWclose(rwops);
	*(script + size) = '\0';
	return script;
}

/* Initialize Lua and load the main script */
int kiavc_scripts_load(const char *path, const kiavc_scripts_callbacks *callbacks) {
	if(!path || !callbacks)
		return -1;
	/* Take note of the callback hooks */
	kiavc_cb = callbacks;
	/* Initialize Lua */
	lua_state = luaL_newstate();
	luaL_openlibs(lua_state);
	/* Register our functions */
	lua_register(lua_state, "kiavcRequire", kiavc_lua_method_kiavcrequire);
	lua_register(lua_state, "getVersion", kiavc_lua_method_getversion);
	lua_register(lua_state, "getVersionString", kiavc_lua_method_getversionstring);
	lua_register(lua_state, "kiavcLog", kiavc_lua_method_kiavclog);
	lua_register(lua_state, "kiavcError", kiavc_lua_method_kiavcerror);
	lua_register(lua_state, "kiavcWarn", kiavc_lua_method_kiavcwarn);
	lua_register(lua_state, "setResolution", kiavc_lua_method_setresolution);
	lua_register(lua_state, "setTitle", kiavc_lua_method_settitle);
	lua_register(lua_state, "setIcon", kiavc_lua_method_seticon);
	lua_register(lua_state, "grabMouse", kiavc_lua_method_grabmouse);
	lua_register(lua_state, "isGrabbingMouse", kiavc_lua_method_isgrabbingmouse);
	lua_register(lua_state, "setFullscreen", kiavc_lua_method_setfullscreen);
	lua_register(lua_state, "getFullscreen", kiavc_lua_method_getfullscreen);
	lua_register(lua_state, "setScanlines", kiavc_lua_method_setscanlines);
	lua_register(lua_state, "getScanlines", kiavc_lua_method_getscanlines);
	lua_register(lua_state, "debugObjects", kiavc_lua_method_debugobjects);
	lua_register(lua_state, "isDebuggingObjects", kiavc_lua_method_isdebuggingobjects);
	lua_register(lua_state, "debugWalkboxes", kiavc_lua_method_debugwalkboxes);
	lua_register(lua_state, "isDebuggingWalkboxes", kiavc_lua_method_isdebuggingwalkboxes);
	lua_register(lua_state, "saveScreenshot", kiavc_lua_method_savescreenshot);
	lua_register(lua_state, "enableConsole", kiavc_lua_method_enableconsole);
	lua_register(lua_state, "showConsole", kiavc_lua_method_showconsole);
	lua_register(lua_state, "hideConsole", kiavc_lua_method_hideconsole);
	lua_register(lua_state, "disableConsole", kiavc_lua_method_disableconsole);
	lua_register(lua_state, "isConsoleEnabled", kiavc_lua_method_isconsoleenabled);
	lua_register(lua_state, "isConsoleVisible", kiavc_lua_method_isconsolevisible);
	lua_register(lua_state, "enableInput", kiavc_lua_method_enableinput);
	lua_register(lua_state, "disableInput", kiavc_lua_method_disableinput);
	lua_register(lua_state, "isInputEnabled", kiavc_lua_method_isinputenabled);
	lua_register(lua_state, "startCutscene", kiavc_lua_method_startcutscene);
	lua_register(lua_state, "stopCutscene", kiavc_lua_method_stopcutscene);
	lua_register(lua_state, "fadeIn", kiavc_lua_method_fadein);
	lua_register(lua_state, "fadeOut", kiavc_lua_method_fadeout);
	lua_register(lua_state, "startDialog", kiavc_lua_method_startdialog);
	lua_register(lua_state, "addDialogLine", kiavc_lua_method_adddialogline);
	lua_register(lua_state, "stopDialog", kiavc_lua_method_stopdialog);
	lua_register(lua_state, "registerAnimation", kiavc_lua_method_registeranimation);
	lua_register(lua_state, "registerFont", kiavc_lua_method_registerfont);
	lua_register(lua_state, "registerCursor", kiavc_lua_method_registercursor);
	lua_register(lua_state, "setCursorAnimation", kiavc_lua_method_setcursoranimation);
	lua_register(lua_state, "setMainCursor", kiavc_lua_method_setmaincursor);
	lua_register(lua_state, "setHotspotCursor", kiavc_lua_method_sethotspotcursor);
	lua_register(lua_state, "showCursor", kiavc_lua_method_showcursor);
	lua_register(lua_state, "hideCursor", kiavc_lua_method_hidecursor);
	lua_register(lua_state, "showCursorText", kiavc_lua_method_showcursortext);
	lua_register(lua_state, "hideCursorText", kiavc_lua_method_hidecursortext);
	lua_register(lua_state, "registerAudio", kiavc_lua_method_registeraudio);
	lua_register(lua_state, "playAudio", kiavc_lua_method_playaudio);
	lua_register(lua_state, "pauseAudio", kiavc_lua_method_pauseaudio);
	lua_register(lua_state, "resumeAudio", kiavc_lua_method_resumeaudio);
	lua_register(lua_state, "stopAudio", kiavc_lua_method_stopaudio);
	lua_register(lua_state, "registerRoom", kiavc_lua_method_registerroom);
	lua_register(lua_state, "setRoomBackground", kiavc_lua_method_setroombackground);
	lua_register(lua_state, "addRoomLayer", kiavc_lua_method_addroomlayer);
	lua_register(lua_state, "removeRoomLayer", kiavc_lua_method_removeroomlayer);
	lua_register(lua_state, "addRoomWalkbox", kiavc_lua_method_addroomwalkbox);
	lua_register(lua_state, "enableRoomWalkbox", kiavc_lua_method_enableroomwalkbox);
	lua_register(lua_state, "disableRoomWalkbox", kiavc_lua_method_disableroomwalkbox);
	lua_register(lua_state, "recalculateRoomWalkboxes", kiavc_lua_method_recalculateroomwalkboxes);
	lua_register(lua_state, "showRoom", kiavc_lua_method_showroom);
	lua_register(lua_state, "registerActor", kiavc_lua_method_registeractor);
	lua_register(lua_state, "setActorCostume", kiavc_lua_method_setactorcostume);
	lua_register(lua_state, "moveActorTo", kiavc_lua_method_moveactorto);
	lua_register(lua_state, "showActor", kiavc_lua_method_showactor);
	lua_register(lua_state, "followActor", kiavc_lua_method_followactor);
	lua_register(lua_state, "hideActor", kiavc_lua_method_hideactor);
	lua_register(lua_state, "fadeActorIn", kiavc_lua_method_fadeactorin);
	lua_register(lua_state, "fadeActorOut", kiavc_lua_method_fadeactorout);
	lua_register(lua_state, "fadeActorTo", kiavc_lua_method_fadeactorto);
	lua_register(lua_state, "setActorAlpha", kiavc_lua_method_setactoralpha);
	lua_register(lua_state, "setActorPlane", kiavc_lua_method_setactorplane);
	lua_register(lua_state, "setActorSpeed", kiavc_lua_method_setactorspeed);
	lua_register(lua_state, "scaleActor", kiavc_lua_method_scaleactor);
	lua_register(lua_state, "walkActorTo", kiavc_lua_method_walkactorto);
	lua_register(lua_state, "sayActor", kiavc_lua_method_sayactor);
	lua_register(lua_state, "setActorDirection", kiavc_lua_method_setactordirection);
	lua_register(lua_state, "controlledActor", kiavc_lua_method_controlledactor);
	lua_register(lua_state, "skipActorsText", kiavc_lua_method_skipactorstext);
	lua_register(lua_state, "setActorState", kiavc_lua_method_setactorstate);
	lua_register(lua_state, "registerCostume", kiavc_lua_method_registercostume);
	lua_register(lua_state, "setCostumeAnimation", kiavc_lua_method_setcostumeanimation);
	lua_register(lua_state, "registerObject", kiavc_lua_method_registerobject);
	lua_register(lua_state, "setObjectAnimation", kiavc_lua_method_setobjectanimation);
	lua_register(lua_state, "setObjectInteractable", kiavc_lua_method_setobjectinteractable);
	lua_register(lua_state, "setObjectUi", kiavc_lua_method_setobjectui);
	lua_register(lua_state, "setObjectUiPosition", kiavc_lua_method_setobjectuiposition);
	lua_register(lua_state, "setObjectUiAnimation", kiavc_lua_method_setobjectuianimation);
	lua_register(lua_state, "setObjectParent", kiavc_lua_method_setobjectparent);
	lua_register(lua_state, "removeObjectParent", kiavc_lua_method_removeobjectparent);
	lua_register(lua_state, "moveObjectTo", kiavc_lua_method_moveobjectto);
	lua_register(lua_state, "floatObjectTo", kiavc_lua_method_floatobjectto);
	lua_register(lua_state, "setObjectHover", kiavc_lua_method_setobjecthover);
	lua_register(lua_state, "showObject", kiavc_lua_method_showobject);
	lua_register(lua_state, "hideObject", kiavc_lua_method_hideobject);
	lua_register(lua_state, "fadeObjectIn", kiavc_lua_method_fadeobjectin);
	lua_register(lua_state, "fadeObjectOut", kiavc_lua_method_fadeobjectout);
	lua_register(lua_state, "fadeObjectTo", kiavc_lua_method_fadeobjectto);
	lua_register(lua_state, "setObjectAlpha", kiavc_lua_method_setobjectalpha);
	lua_register(lua_state, "setObjectPlane", kiavc_lua_method_setobjectplane);
	lua_register(lua_state, "setObjectState", kiavc_lua_method_setobjectstate);
	lua_register(lua_state, "scaleObject", kiavc_lua_method_scaleobject);
	lua_register(lua_state, "addObjectToInventory", kiavc_lua_method_addobjecttoinventory);
	lua_register(lua_state, "removeObjectFromInventory", kiavc_lua_method_removeobjectfrominventory);
	lua_register(lua_state, "showText", kiavc_lua_method_showtext);
	lua_register(lua_state, "floatTextTo", kiavc_lua_method_floattextto);
	lua_register(lua_state, "fadeTextIn", kiavc_lua_method_fadetextin);
	lua_register(lua_state, "fadeTextOut", kiavc_lua_method_fadetextout);
	lua_register(lua_state, "fadeTextTo", kiavc_lua_method_fadetextto);
	lua_register(lua_state, "setTextAlpha", kiavc_lua_method_settextalpha);
	lua_register(lua_state, "removeText", kiavc_lua_method_removetext);
	lua_register(lua_state, "quit", kiavc_lua_method_quit);
	/* Set the scripts folder */
	lua_getglobal(lua_state, "package");
	lua_getfield(lua_state, -1, "path");
	const char *cur_path = lua_tostring(lua_state, -1);
	char new_path[1024];
	new_path[0] = '\0';
	snprintf(new_path, sizeof(new_path)-1, "%s;%s/?.lua", cur_path, "./lua");
	lua_pop(lua_state, 1);
	lua_pushstring(lua_state, new_path);
	lua_setfield(lua_state, -2, "path");
	lua_pop(lua_state, 1);
	/* Let's load the engine script first */
	char *script = kiavc_scripts_open_file("./lua/engine/kiavc.lua");
	if(!script) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open engine Lua script\n");
		return -1;
	}
	int err = luaL_dostring(lua_state, script);
	SDL_free(script);
	if(err) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error loading engine Lua script: %s\n",
			lua_tostring(lua_state, -1));
		return -1;
	}
	/* Now load the provided script */
	script = kiavc_scripts_open_file(path);
	if(!script) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open Lua script '%s'\n", path);
		return -1;
	}
	err = luaL_dostring(lua_state, script);
	SDL_free(script);
	if(err) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error loading Lua script '%s': %s\n",
			path, lua_tostring(lua_state, -1));
		return -1;
	}
	/* We're done for now */
	return 0;
}

/* Run the provided script command */
void kiavc_scripts_run_command(const char *fmt, ...) {
	char command[1024];
	va_list args;
	va_start(args, fmt);
	SDL_vsnprintf(command, sizeof(command)-1, fmt, args);
	va_end(args);
	lua_getglobal(lua_state, "runCommand");
	lua_pushstring(lua_state, command);
	if(lua_pcall(lua_state, 1, 0, 0) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error running function `updateWorld': %s",
			lua_tostring(lua_state, -1));
	}
}

/* Update the world in the script */
int kiavc_scripts_update_world(Uint32 ticks) {
	/* We invoke the updateWorld() function in the Lua script */
	lua_getglobal(lua_state, "updateWorld");
	lua_pushnumber(lua_state, ticks);
	if(lua_pcall(lua_state, 1, 0, 0) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error running function `updateWorld': %s",
			lua_tostring(lua_state, -1));
		return -1;
	}
	return 0;
}

/* Close the script engine */
void kiavc_scripts_unload(void) {
	/* FIXME */
	lua_close(lua_state);
}

/*
 * Methods that we expose to the Lua script
 */

/* Load a script from the assets */
static int kiavc_lua_method_kiavcrequire(lua_State *s) {
	/* This method allows the Lua script to import another Lua file */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *required = luaL_checkstring(s, 1);
	if(required == NULL) {
		/* Ignore */
		return 0;
	}
	char path[256];
	path[0] = '\0';
	SDL_snprintf(path, sizeof(path)-1, "./lua/%s.lua", required);
	char *script = kiavc_scripts_open_file(path);
	if(!script) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Couldn't open Lua script '%s'\n", path);
		return 0;
	}
	int err = luaL_dostring(lua_state, script);
	SDL_free(script);
	if(err) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error loading Lua script '%s': %s\n",
			path, lua_tostring(lua_state, -1));
	}
	SDL_Log("Loaded script '%s'\n", path);
	return 0;
}

/* Return version as major/minor/patch */
static int kiavc_lua_method_getversion(lua_State *s) {
	/* This method allows the Lua script to retrieve info on the engine version */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	lua_pushnumber(s, KIAVC_VERSION_MAJOR);
	lua_pushnumber(s, KIAVC_VERSION_MINOR);
	lua_pushnumber(s, KIAVC_VERSION_PATCH);
	return 3;
}

/* Return version string */
static int kiavc_lua_method_getversionstring(lua_State *s) {
	/* This method allows the Lua script to retrieve the engine version as a string */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	lua_pushstring(s, KIAVC_VERSION_STRING);
	return 1;
}

/* Logging, to use the same SDL-based logging as the rest of the application */
static int kiavc_lua_method_kiavclog(lua_State *s) {
	/* This method allows the Lua script to use the Janus internal logger */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *text = luaL_checkstring(s, 1);
	if(text == NULL) {
		/* Ignore */
		return 0;
	}
	SDL_Log("[Lua] %s\n", text);
	return 0;
}

/* Error logging, to use the same SDL-based logging as the rest of the application */
static int kiavc_lua_method_kiavcerror(lua_State *s) {
	/* This method allows the Lua script to use the Janus internal logger */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *text = luaL_checkstring(s, 1);
	if(text == NULL) {
		/* Ignore */
		return 0;
	}
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] %s\n", text);
	return 0;
}

/* Warning logging, to use the same SDL-based logging as the rest of the application */
static int kiavc_lua_method_kiavcwarn(lua_State *s) {
	/* This method allows the Lua script to use the Janus internal logger */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *text = luaL_checkstring(s, 1);
	if(text == NULL) {
		/* Ignore */
		return 0;
	}
	SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[Lua] %s\n", text);
	return 0;
}

/* Set resolution and scaling */
static int kiavc_lua_method_setresolution(lua_State *s) {
	/* This method allows the Lua script to set the window resolution and scaling */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "width");
	int width = luaL_checknumber(s, 2);
	lua_getfield(s, 1, "height");
	int height = luaL_checknumber(s, 3);
	lua_getfield(s, 1, "fps");
	int fps = luaL_checknumber(s, 4);
	lua_getfield(s, 1, "scale");
	int scale = luaL_checknumber(s, 5);
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_resolution(width, height, fps, scale);
	return 0;
}

/* Set window title */
static int kiavc_lua_method_settitle(lua_State *s) {
	/* This method allows the Lua script to set the window title */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *title = luaL_checkstring(s, 1);
	if(title == NULL) {
		/* Ignore */
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_title(title);
	return 0;
}

/* Set window icon */
static int kiavc_lua_method_seticon(lua_State *s) {
	/* This method allows the Lua script to set the window icon */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *path = luaL_checkstring(s, 1);
	if(path == NULL) {
		/* Ignore */
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_icon(path);
	return 0;
}

/* Set whether we should grab the mouse and confine it to the window */
static int kiavc_lua_method_grabmouse(lua_State *s) {
	/* This method allows the Lua script to enable or disable mouse grabbing */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	bool grab = lua_toboolean(s, 1);
	/* Invoke the application callback to enforce this */
	kiavc_cb->grab_mouse(grab);
	return 0;
}

/* Check whether we're grabbing the mouse and confining it to the window */
static int kiavc_lua_method_isgrabbingmouse(lua_State *s) {
	/* This method allows the Lua script check if fullscreen is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool grabbing = kiavc_cb->is_grabbing_mouse();
	/* Pass the response back to the stack */
	lua_pushboolean(s, grabbing);
	return 1;
}

/* Set fullscreen mode */
static int kiavc_lua_method_setfullscreen(lua_State *s) {
	/* This method allows the Lua script to enable or disable the fullscreen mode */
	int n = lua_gettop(s), exp = 1, exp2 = 2;
	if(n != exp && n != exp2) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d or %d)\n", n, exp, exp2);
		return 0;
	}
	bool fullscreen = lua_toboolean(s, 1);
	bool desktop = false;
	if(n == 2)
		desktop = lua_toboolean(s, 2);
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_fullscreen(fullscreen, desktop);
	return 0;
}

/* Get fullscreen mode */
static int kiavc_lua_method_getfullscreen(lua_State *s) {
	/* This method allows the Lua script check if fullscreen is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool fullscreen = kiavc_cb->get_fullscreen();
	/* Pass the response back to the stack */
	lua_pushboolean(s, fullscreen);
	return 1;
}

/* Set scanlines mode */
static int kiavc_lua_method_setscanlines(lua_State *s) {
	/* This method allows the Lua script to show or hide the scanlines */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	int alpha = lua_tointeger(s, 1);
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_scanlines(alpha);
	return 0;
}

/* Get scanlines mode */
static int kiavc_lua_method_getscanlines(lua_State *s) {
	/* This method allows the Lua script check if scanlines is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	int alpha = kiavc_cb->get_scanlines();
	/* Pass the response back to the stack */
	lua_pushinteger(s, alpha);
	return 1;
}

/* Set whether to debug objects or not */
static int kiavc_lua_method_debugobjects(lua_State *s) {
	/* This method allows the Lua script to debug objects */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	bool debug = lua_toboolean(s, 1);
	/* Invoke the application callback to enforce this */
	kiavc_cb->debug_objects(debug);
	return 0;
}

/* Check whether objects debugging is on or not */
static int kiavc_lua_method_isdebuggingobjects(lua_State *s) {
	/* This method allows the Lua script check if objects debugging is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool debug = kiavc_cb->is_debugging_objects();
	/* Pass the response back to the stack */
	lua_pushboolean(s, debug);
	return 1;
}

/* Set whether to debug walkboxes or not */
static int kiavc_lua_method_debugwalkboxes(lua_State *s) {
	/* This method allows the Lua script to debug walkboxes */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	bool debug = lua_toboolean(s, 1);
	/* Invoke the application callback to enforce this */
	kiavc_cb->debug_walkboxes(debug);
	return 0;
}

/* Check whether walkboxes debugging is on or not */
static int kiavc_lua_method_isdebuggingwalkboxes(lua_State *s) {
	/* This method allows the Lua script check if walkboxes debugging is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool debug = kiavc_cb->is_debugging_walkboxes();
	/* Pass the response back to the stack */
	lua_pushboolean(s, debug);
	return 1;
}

/* Save a screenshot */
static int kiavc_lua_method_savescreenshot(lua_State *s) {
	/* This method allows the Lua script to save a screenshot */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *path = luaL_checkstring(s, 1);
	if(path == NULL) {
		/* Ignore */
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->save_screenshot(path);
	return 0;
}

/* Enable the console and specify which font to use */
static int kiavc_lua_method_enableconsole(lua_State *s) {
	/* This method allows the Lua script to enable the scripting console */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *font = luaL_checkstring(s, 1);
	if(font == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing font ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->enable_console(font);
	return 0;
}

/* Show the console */
static int kiavc_lua_method_showconsole(lua_State *s) {
	/* This method allows the Lua script to show the scripting console */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_console();
	return 0;
}

/* Hide the console */
static int kiavc_lua_method_hideconsole(lua_State *s) {
	/* This method allows the Lua script to hide the scripting console */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->hide_console();
	return 0;
}

/* Disable the console */
static int kiavc_lua_method_disableconsole(lua_State *s) {
	/* This method allows the Lua script to disable the scripting console */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->disable_console();
	return 0;
}

/* Check if the console is enabled */
static int kiavc_lua_method_isconsoleenabled(lua_State *s) {
	/* This method allows the Lua script check if the console is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool enabled = kiavc_cb->is_console_enabled();
	/* Pass the response back to the stack */
	lua_pushboolean(s, enabled);
	return 1;
}

/* Check if the console is visible */
static int kiavc_lua_method_isconsolevisible(lua_State *s) {
	/* This method allows the Lua script check if the console is visible */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool visible = kiavc_cb->is_console_visible();
	/* Pass the response back to the stack */
	lua_pushboolean(s, visible);
	return 1;
}

/* Enable input from the user */
static int kiavc_lua_method_enableinput(lua_State *s) {
	/* This method allows the Lua script to disable input from the user temporarily */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->enable_input();
	return 0;
}

/* Check if input from the user is enabled */
static int kiavc_lua_method_isinputenabled(lua_State *s) {
	/* This method allows the Lua script check if input from the user is enabled */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to query this */
	bool enabled = kiavc_cb->is_input_enabled();
	/* Pass the response back to the stack */
	lua_pushboolean(s, enabled);
	return 1;
}

/* Disable input from the user */
static int kiavc_lua_method_disableinput(lua_State *s) {
	/* This method allows the Lua script to disable input from the user temporarily */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->disable_input();
	return 0;
}

/* Start a cutscene */
static int kiavc_lua_method_startcutscene(lua_State *s) {
	/* This method allows the Lua script to start a cutscene */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->start_cutscene();
	return 0;
}

/* Stop a cutscene */
static int kiavc_lua_method_stopcutscene(lua_State *s) {
	/* This method allows the Lua script to stop a cutscene */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->stop_cutscene();
	return 0;
}

/* Fade in */
static int kiavc_lua_method_fadein(lua_State *s) {
	/* This method allows the Lua script to fade in */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	int ms = luaL_checknumber(s, 1);
	kiavc_cb->fade_in(ms);
	return 0;
}

/* Fade out */
static int kiavc_lua_method_fadeout(lua_State *s) {
	/* This method allows the Lua script to fade out */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	int ms = luaL_checknumber(s, 1);
	kiavc_cb->fade_out(ms);
	return 0;
}

/* Start a new dialog session */
static int kiavc_lua_method_startdialog(lua_State *s) {
	/* This method allows the Lua script to start a new dialog session */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "font");
	const char *font = luaL_checkstring(s, 3);
	lua_getfield(s, 1, "color");
	luaL_checktype(s, 4, LUA_TTABLE);
	SDL_Color color = { 0 };
	lua_getfield(s, 4, "r");
	color.r = luaL_checknumber(s, 5);
	lua_getfield(s, 4, "g");
	color.g = luaL_checknumber(s, 6);
	lua_getfield(s, 4, "b");
	color.b = luaL_checknumber(s, 7);
	int or = -1, og = -1, ob = -1, idx = 8;
	if(lua_getfield(s, 1, "outline") != LUA_TNIL) {
		int tidx = idx;
		luaL_checktype(s, tidx, LUA_TTABLE);
		lua_getfield(s, tidx, "r");
		or = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "g");
		og = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "b");
		ob = luaL_checknumber(s, ++idx);
	}
	SDL_Color outline = { .r = or, .g = og, .b = ob };
	lua_getfield(s, 1, "selected");
	int tidx = ++idx;
	luaL_checktype(s, tidx, LUA_TTABLE);
	SDL_Color s_color = { 0 };
	lua_getfield(s, tidx, "r");
	s_color.r = luaL_checknumber(s, ++idx);
	lua_getfield(s, tidx, "g");
	s_color.g = luaL_checknumber(s, ++idx);
	lua_getfield(s, tidx, "b");
	s_color.b = luaL_checknumber(s, ++idx);
	int sor = -1, sog = -1, sob = -1;
	idx++;
	if(lua_getfield(s, 1, "selectedOutline") != LUA_TNIL) {
		tidx = idx;
		luaL_checktype(s, tidx, LUA_TTABLE);
		lua_getfield(s, tidx, "r");
		sor = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "g");
		sog = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "b");
		sob = luaL_checknumber(s, ++idx);
	}
	SDL_Color s_outline = { .r = sor, .g = sog, .b = sob };
	if(id == NULL || font == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing dialog of font ID\n");
		return 0;
	}
	lua_getfield(s, 1, "background");
	tidx = ++idx;
	luaL_checktype(s, tidx, LUA_TTABLE);
	SDL_Color background = { 0 };
	lua_getfield(s, tidx, "r");
	background.r = luaL_checknumber(s, ++idx);
	lua_getfield(s, tidx, "g");
	background.g = luaL_checknumber(s, ++idx);
	lua_getfield(s, tidx, "b");
	background.b = luaL_checknumber(s, ++idx);
	idx++;
	background.a = 255;
	if(lua_getfield(s, tidx, "a") != LUA_TNIL)
		background.a = luaL_checknumber(s, idx);
	lua_getfield(s, 1, "area");
	tidx = ++idx;
	luaL_checktype(s, tidx, LUA_TTABLE);
	SDL_Rect area = { 0 };
	lua_getfield(s, tidx, "x1");
	area.x = luaL_checknumber(s, ++idx);
	lua_getfield(s, tidx, "y1");
	area.y = luaL_checknumber(s, ++idx);
	lua_getfield(s, tidx, "x2");
	area.w = luaL_checknumber(s, ++idx) - area.x;
	lua_getfield(s, tidx, "y2");
	area.h = luaL_checknumber(s, ++idx) - area.y;
	idx++;
	bool autohide = false;
	if(lua_getfield(s, 1, "autohide") != LUA_TNIL)
		autohide = lua_toboolean(s, idx);
	/* Invoke the application callback to enforce this */
	kiavc_cb->start_dialog(id, font, &color, (or != -1 && og != -1 && ob != -1 ? &outline : NULL),
		&s_color, (sor != -1 && sog != -1 && sob != -1 ? &s_outline : NULL), &background, &area, autohide);
	return 0;
}

/* Add a line to a dialog session */
static int kiavc_lua_method_adddialogline(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "text");
	const char *text = luaL_checkstring(s, 3);
	lua_getfield(s, 1, "name");
	const char *name = luaL_checkstring(s, 4);
	if(id == NULL || text == NULL || name == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing dialog ID, text or line name\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->add_dialog_line(id, name, text);
	return 0;
}

/* Stop a dialog session */
static int kiavc_lua_method_stopdialog(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing cursor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->stop_dialog(id);
	return 0;
}

/* Register a new animation in the engine */
static int kiavc_lua_method_registeranimation(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new animation */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "path");
	const char *path = luaL_checkstring(s, 3);
	lua_getfield(s, 1, "frames");
	int frames = luaL_checknumber(s, 4);
	int tr = -1, tg = -1, tb = -1, idx = 4;
	int tidx = ++idx;
	if(lua_getfield(s, 1, "transparency") != LUA_TNIL) {
		luaL_checktype(s, tidx, LUA_TTABLE);
		lua_getfield(s, tidx, "r");
		tr = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "g");
		tg = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "b");
		tb = luaL_checknumber(s, ++idx);
	}
	int ms = 100;
	if(lua_getfield(s, 1, "ms") != LUA_TNIL)
		ms = luaL_checknumber(s, ++idx);
	if(id == NULL || path == NULL || frames < 1 || ms < 1) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing animation ID or path, or invalid number of frames/timing\n");
		return 0;
	}
	if(tr < 0 || tb < 0 || tg < 0) {
		/* Invoke the application callback to enforce this */
		kiavc_cb->register_animation(id, path, frames, ms, NULL);
	} else {
		/* RGB for color keying was passed as well */
		SDL_Color color = { .r = tr, .g = tg, .b = tb };
		/* Invoke the application callback to enforce this */
		kiavc_cb->register_animation(id, path, frames, ms, &color);
	}
	return 0;
}

/* Register a new font in the engine */
static int kiavc_lua_method_registerfont(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new font */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "path");
	const char *path = luaL_checkstring(s, 3);
	lua_getfield(s, 1, "size");
	int size = luaL_checknumber(s, 4);
	if(id == NULL || path == NULL || size < 1) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing font ID or path, or invalid size\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_font(id, path, size);
	return 0;
}

/* Register a new cursor in the engine */
static int kiavc_lua_method_registercursor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing cursor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_cursor(id);
	return 0;
}

/* Set the current animation for a cursor */
static int kiavc_lua_method_setcursoranimation(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *anim = luaL_checkstring(s, 2);
	if(id == NULL || anim == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing cursor or animation ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_cursor_animation(id, anim);
	return 0;
}

/* Set the main cursor */
static int kiavc_lua_method_setmaincursor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing cursor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_main_cursor(id);
	return 0;
}

/* Set the hotspot cursor */
static int kiavc_lua_method_sethotspotcursor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing cursor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_hotspot_cursor(id);
	return 0;
}

/* Show a cursor */
static int kiavc_lua_method_showcursor(lua_State *s) {
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_cursor();
	return 0;
}

/* Hide a cursor */
static int kiavc_lua_method_hidecursor(lua_State *s) {
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->hide_cursor();
	return 0;
}

/* Show a cursor text */
static int kiavc_lua_method_showcursortext(lua_State *s) {
	/* This method allows the Lua script to show custom text on the cursor as it moves */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "font");
	const char *font = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "text");
	const char *text = luaL_checkstring(s, 3);
	lua_getfield(s, 1, "color");
	luaL_checktype(s, 4, LUA_TTABLE);
	SDL_Color color = { 0 };
	lua_getfield(s, 4, "r");
	color.r = luaL_checknumber(s, 5);
	lua_getfield(s, 4, "g");
	color.g = luaL_checknumber(s, 6);
	lua_getfield(s, 4, "b");
	color.b = luaL_checknumber(s, 7);
	int or = -1, og = -1, ob = -1, idx = 7;
	if(lua_getfield(s, 1, "outline") != LUA_TNIL) {
		int tidx = ++idx;
		luaL_checktype(s, tidx, LUA_TTABLE);
		lua_getfield(s, tidx, "r");
		or = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "g");
		og = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "b");
		ob = luaL_checknumber(s, ++idx);
	}
	SDL_Color outline = { .r = or, .g = og, .b = ob };
	if(font == NULL || text == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing cursor font or text\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_cursor_text(font, text, &color, (or != -1 && og != -1 && ob != -1 ? &outline : NULL));
	return 0;
}

/* Hide a cursor text */
static int kiavc_lua_method_hidecursortext(lua_State *s) {
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->hide_cursor_text();
	return 0;
}

/* Register a new audio track in the engine */
static int kiavc_lua_method_registeraudio(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new audio */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "path");
	const char *path = luaL_checkstring(s, 3);
	if(id == NULL || path == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing audio ID or path\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_audio(id, path);
	return 0;
}

/* Play an audio track in the engine */
static int kiavc_lua_method_playaudio(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int fade_ms = luaL_checknumber(s, 2);
	bool loop = lua_toboolean(s, 3);
	if(id == NULL || fade_ms < 0) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing audio ID or invalid fade-in value\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->play_audio(id, fade_ms, loop);
	return 0;
}

/* Pause an audio track in the engine */
static int kiavc_lua_method_pauseaudio(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing audio track ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->pause_audio(id);
	return 0;
}

/* Resume an audio track in the engine */
static int kiavc_lua_method_resumeaudio(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing audio track ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->resume_audio(id);
	return 0;
}

/* Stop an audio track in the engine */
static int kiavc_lua_method_stopaudio(lua_State *s) {
	int n = lua_gettop(s), exp = 1, exp2 = 2;
	if(n != exp && n != exp2) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d or %d)\n", n, exp, exp2);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int fade_ms = (n == 2 ? luaL_checknumber(s, 2) : 0);
	if(id == NULL || fade_ms < 0) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing audio track ID or invalid fade-out value\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->stop_audio(id, fade_ms);
	return 0;
}

/* Register a new room in the engine */
static int kiavc_lua_method_registerroom(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new room */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_room(id);
	return 0;
}

/* Set the current background for a room */
static int kiavc_lua_method_setroombackground(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *bg = luaL_checkstring(s, 2);
	if(id == NULL || bg == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room or image ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_room_background(id, bg);
	return 0;
}

/* Add a room layer */
static int kiavc_lua_method_addroomlayer(lua_State *s) {
	int n = lua_gettop(s), exp = 4;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *name = luaL_checkstring(s, 2);
	const char *bg = luaL_checkstring(s, 3);
	int zplane = luaL_checkinteger(s, 4);
	if(id == NULL || name == NULL || bg == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room or layer ID, or layer image\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->add_room_layer(id, name, bg, zplane);
	return 0;
}

/* Remove a room layer */
static int kiavc_lua_method_removeroomlayer(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *name = luaL_checkstring(s, 2);
	if(id == NULL || name == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room or layer ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->remove_room_layer(id, name);
	return 0;
}

/* Add a room walkbox */
static int kiavc_lua_method_addroomwalkbox(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n > exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room ID\n");
		return 0;
	}
	luaL_checktype(s, 2, LUA_TTABLE);
	lua_getfield(s, 2, "x1");
	int from_x = luaL_checknumber(s, 3);
	lua_getfield(s, 2, "y1");
	int from_y = luaL_checknumber(s, 4);
	lua_getfield(s, 2, "x2");
	int to_x = luaL_checknumber(s, 5);
	lua_getfield(s, 2, "y2");
	int to_y = luaL_checknumber(s, 6);
	/* name and disabled are both optional */
	const char *name = NULL;
	bool disabled = false;
	float scale = 1.0, speed = 1.0;
	int idx = 7;
	if(lua_getfield(s, 2, "name") != LUA_TNIL)
		name = luaL_checkstring(s, idx);
	idx++;
	if(lua_getfield(s, 2, "disabled") != LUA_TNIL)
		disabled = lua_toboolean(s, idx);
	idx++;
	if(lua_getfield(s, 2, "scale") != LUA_TNIL)
		scale = luaL_checknumber(s, idx);
	idx++;
	if(lua_getfield(s, 2, "speed") != LUA_TNIL)
		speed = luaL_checknumber(s, idx);
	/* Invoke the application callback to enforce this */
	kiavc_cb->add_room_walkbox(id, name, from_x, from_y, to_x, to_y, scale, speed, disabled);
	return 0;
}

/* Enable a room walkbox */
static int kiavc_lua_method_enableroomwalkbox(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *name = luaL_checkstring(s, 2);
	if(id == NULL || name == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room or walkbox ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->enable_room_walkbox(id, name);
	return 0;
}

/* Disable a room walkbox */
static int kiavc_lua_method_disableroomwalkbox(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *name = luaL_checkstring(s, 2);
	if(id == NULL || name == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room or walkbox ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->disable_room_walkbox(id, name);
	return 0;
}

/* Recalculate walkboxes in a room */
static int kiavc_lua_method_recalculateroomwalkboxes(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->recalculate_room_walkboxes(id);
	return 0;
}

/* Set the active room */
static int kiavc_lua_method_showroom(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_room(id);
	return 0;
}

/* Register a new actor in the engine */
static int kiavc_lua_method_registeractor(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new actor */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_actor(id);
	return 0;
}

/* Set the current costume for an actor */
static int kiavc_lua_method_setactorcostume(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *cost = luaL_checkstring(s, 2);
	if(id == NULL || cost == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor or costume ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_actor_costume(id, cost);
	return 0;
}

/* Move an actor to a specific room */
static int kiavc_lua_method_moveactorto(lua_State *s) {
	int n = lua_gettop(s), exp = 4;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *room = luaL_checkstring(s, 2);
	int x = luaL_checknumber(s, 3);
	int y = luaL_checknumber(s, 4);
	if(id == NULL || room == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor or room ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->move_actor_to(id, room, x, y);
	return 0;
}

/* Show an actor in the room they're in */
static int kiavc_lua_method_showactor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_actor(id);
	return 0;
}

/* Follow an actor in the room they're in */
static int kiavc_lua_method_followactor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n > exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = (n == 1 ? luaL_checkstring(s, 1) : NULL);
	/* Invoke the application callback to enforce this */
	kiavc_cb->follow_actor(id);
	return 0;
}

/* Hide an actor in the room they're in */
static int kiavc_lua_method_hideactor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->hide_actor(id);
	return 0;
}

/* Fade an actor in */
static int kiavc_lua_method_fadeactorin(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int ms = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_actor_to(id, 255, ms);
	return 0;
}

/* Fade an actor out */
static int kiavc_lua_method_fadeactorout(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int ms = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_actor_to(id, 0, ms);
	return 0;
}

/* Fade an actor to a specific alpha */
static int kiavc_lua_method_fadeactorto(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int alpha = luaL_checknumber(s, 2);
	int ms = luaL_checknumber(s, 3);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_actor_to(id, alpha, ms);
	return 0;
}

/* Set the alpha for the actor */
static int kiavc_lua_method_setactoralpha(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int alpha = luaL_checkinteger(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_actor_alpha(id, alpha);
	return 0;
}

/* Set the z-plane for the actor */
static int kiavc_lua_method_setactorplane(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int zplane = luaL_checkinteger(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_actor_plane(id, zplane);
	return 0;
}

/* Set the movement speed for the actor */
static int kiavc_lua_method_setactorspeed(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int speed = luaL_checkinteger(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_actor_speed(id, speed);
	return 0;
}

/* Scale an actor */
static int kiavc_lua_method_scaleactor(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	float scale = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->scale_actor(id, scale);
	return 0;
}

/* Walk an actor to some coordinates */
static int kiavc_lua_method_walkactorto(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int x = luaL_checknumber(s, 2);
	int y = luaL_checknumber(s, 3);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->walk_actor_to(id, x, y);
	return 0;
}

/* Have an actor say something */
static int kiavc_lua_method_sayactor(lua_State *s) {
	/* This method allows the Lua script to have an actor say something */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "text");
	const char *text = luaL_checkstring(s, 3);
	lua_getfield(s, 1, "font");
	const char *font = luaL_checkstring(s, 4);
	lua_getfield(s, 1, "color");
	luaL_checktype(s, 5, LUA_TTABLE);
	SDL_Color color = { 0 };
	lua_getfield(s, 5, "r");
	color.r = luaL_checknumber(s, 6);
	lua_getfield(s, 5, "g");
	color.g = luaL_checknumber(s, 7);
	lua_getfield(s, 5, "b");
	color.b = luaL_checknumber(s, 8);
	int or = -1, og = -1, ob = -1, idx = 8;
	if(lua_getfield(s, 1, "outline") != LUA_TNIL) {
		int tidx = ++idx;
		luaL_checktype(s, tidx, LUA_TTABLE);
		lua_getfield(s, tidx, "r");
		or = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "g");
		og = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "b");
		ob = luaL_checknumber(s, ++idx);
	}
	SDL_Color outline = { .r = or, .g = og, .b = ob };
	if(id == NULL || font == NULL || text == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID or text\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->say_actor(id, text, font, &color, (or != -1 && og != -1 && ob != -1 ? &outline : NULL));
	return 0;
}

/* Change the actor's direction */
static int kiavc_lua_method_setactordirection(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *direction = luaL_checkstring(s, 2);
	if(id == NULL || direction == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID or direction\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_actor_direction(id, direction);
	return 0;

}

/* Set the currently controlled actor */
static int kiavc_lua_method_controlledactor(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->controlled_actor(id);
	return 0;

}

/* Skip the text any actor is saying */
static int kiavc_lua_method_skipactorstext(lua_State *s) {
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->skip_actors_text();
	return 0;

}

/* Set a specific state for an actor */
static int kiavc_lua_method_setactorstate(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *type = luaL_checkstring(s, 2);
	if(id == NULL || type == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID or type\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_actor_state(id, type);
	return 0;
}

/* Register a new costume in the engine */
static int kiavc_lua_method_registercostume(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new costume */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing costume ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_costume(id);
	return 0;
}

/* Set the current animation for a costume */
static int kiavc_lua_method_setcostumeanimation(lua_State *s) {
	int n = lua_gettop(s), exp = 4;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *type = luaL_checkstring(s, 2);
	const char *direction = luaL_checkstring(s, 3);
	const char *anim = luaL_checkstring(s, 4);
	if(id == NULL || type == NULL || direction == NULL || anim == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing costume ID, type, direction or animation ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_costume_animation(id, type, direction, anim);
	return 0;
}

/* Register a new object in the engine */
static int kiavc_lua_method_registerobject(lua_State *s) {
	/* This method allows the Lua script to notify the engine about a new object */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->register_object(id);
	return 0;
}

/* Set the current animation for an object */
static int kiavc_lua_method_setobjectanimation(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *state = luaL_checkstring(s, 2);
	const char *anim = luaL_checkstring(s, 3);
	if(id == NULL || anim == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object animation ID\n");
		return 0;
	}
	if(state == NULL)
		state = "default";
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_animation(id, state, anim);
	return 0;
}

/* Mark whether this object can be interacted with */
static int kiavc_lua_method_setobjectinteractable(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	bool interactable = lua_toboolean(s, 2);
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_interactable(id, interactable);
	return 0;

}

/* Mark whether this object is of the UI */
static int kiavc_lua_method_setobjectui(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	bool ui = lua_toboolean(s, 2);
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_ui(id, ui);
	return 0;

}

/* Set the UI position for this object */
static int kiavc_lua_method_setobjectuiposition(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int x = luaL_checknumber(s, 2);
	int y = luaL_checknumber(s, 3);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_ui_position(id, x, y);
	return 0;

}

/* Set the current animation for an object, when part of the UI */
static int kiavc_lua_method_setobjectuianimation(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *anim = luaL_checkstring(s, 2);
	if(id == NULL || anim == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object animation ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_ui_animation(id, anim);
	return 0;
}

/* Set the parent for an object (start relative positioning), when part of the UI */
static int kiavc_lua_method_setobjectparent(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *parent = luaL_checkstring(s, 2);
	if(id == NULL || parent == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object IDs\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_parent(id, parent);
	return 0;
}

/* Remove the parent for an object (stop relative positioning), when part of the UI */
static int kiavc_lua_method_removeobjectparent(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->remove_object_parent(id);
	return 0;
}

/* Move an object to a specific room */
static int kiavc_lua_method_moveobjectto(lua_State *s) {
	int n = lua_gettop(s), exp = 4;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *room = luaL_checkstring(s, 2);
	int x = luaL_checknumber(s, 3);
	int y = luaL_checknumber(s, 4);
	if(id == NULL || room == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object or room ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->move_object_to(id, room, x, y);
	return 0;
}

/* Float an object at some coordinates at a certain speed */
static int kiavc_lua_method_floatobjectto(lua_State *s) {
	/* This method allows the Lua script to move an object around */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	lua_getfield(s, 1, "x");
	int x = luaL_checknumber(s, 3);
	lua_getfield(s, 1, "y");
	int y = luaL_checknumber(s, 4);
	lua_getfield(s, 1, "speed");
	int speed = luaL_checknumber(s, 5);
	/* Invoke the application callback to enforce this */
	kiavc_cb->float_object_to(id, x, y, speed);
	return 0;
}

/* Specify the hover coordinates for an object */
static int kiavc_lua_method_setobjecthover(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n > exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	luaL_checktype(s, 2, LUA_TTABLE);
	lua_getfield(s, 2, "x1");
	int from_x = luaL_checknumber(s, 3);
	lua_getfield(s, 2, "y1");
	int from_y = luaL_checknumber(s, 4);
	lua_getfield(s, 2, "x2");
	int to_x = luaL_checknumber(s, 5);
	lua_getfield(s, 2, "y2");
	int to_y = luaL_checknumber(s, 6);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing room ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_hover(id, from_x, from_y, to_x, to_y);
	return 0;
}

/* Show an object in the room they're in */
static int kiavc_lua_method_showobject(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_object(id);
	return 0;
}

/* Hide an object in the room they're in */
static int kiavc_lua_method_hideobject(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->hide_object(id);
	return 0;
}

/* Fade an object in */
static int kiavc_lua_method_fadeobjectin(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int ms = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_object_to(id, 255, ms);
	return 0;
}

/* Fade an object out */
static int kiavc_lua_method_fadeobjectout(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int ms = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_object_to(id, 0, ms);
	return 0;
}

/* Fade an object to a specific alpha */
static int kiavc_lua_method_fadeobjectto(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int alpha = luaL_checknumber(s, 2);
	int ms = luaL_checknumber(s, 3);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_object_to(id, alpha, ms);
	return 0;
}

/* Set the alpha for the object */
static int kiavc_lua_method_setobjectalpha(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int alpha = luaL_checkinteger(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_alpha(id, alpha);
	return 0;
}

/* Set the z-plane for the object */
static int kiavc_lua_method_setobjectplane(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int zplane = luaL_checkinteger(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_plane(id, zplane);
	return 0;
}

/* Set the state for the object */
static int kiavc_lua_method_setobjectstate(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *state = luaL_checkstring(s, 2);
	if(id == NULL || state == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID or state\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_object_state(id, state);
	return 0;
}

/* Scale an object */
static int kiavc_lua_method_scaleobject(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	float scale = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->scale_object(id, scale);
	return 0;
}

/* Add an object to an actor's inventory */
static int kiavc_lua_method_addobjecttoinventory(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *owner = luaL_checkstring(s, 2);
	if(id == NULL || owner == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object or actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->add_object_to_inventory(id, owner);
	return 0;
}

/* Remove an object from an actor's inventory */
static int kiavc_lua_method_removeobjectfrominventory(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	const char *owner = luaL_checkstring(s, 2);
	if(id == NULL || owner == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing object or actor ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->remove_object_from_inventory(id, owner);
	return 0;
}

/* Show some text at some coordinates for some time */
static int kiavc_lua_method_showtext(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "font");
	const char *font = luaL_checkstring(s, 2);
	lua_getfield(s, 1, "text");
	const char *text = luaL_checkstring(s, 3);
	if(font == NULL || text == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing actor ID or text\n");
		return 0;
	}
	lua_getfield(s, 1, "x");
	int x = luaL_checknumber(s, 4);
	lua_getfield(s, 1, "y");
	int y = luaL_checknumber(s, 5);
	lua_getfield(s, 1, "duration");
	int ms = luaL_checknumber(s, 6);
	lua_getfield(s, 1, "color");
	luaL_checktype(s, 7, LUA_TTABLE);
	SDL_Color color = { 0 };
	lua_getfield(s, 7, "r");
	color.r = luaL_checknumber(s, 8);
	lua_getfield(s, 7, "g");
	color.g = luaL_checknumber(s, 9);
	lua_getfield(s, 7, "b");
	color.b = luaL_checknumber(s, 10);
	int or = -1, og = -1, ob = -1, tidx = 11, idx = tidx;
	if(lua_getfield(s, 1, "outline") != LUA_TNIL) {
		luaL_checktype(s, tidx, LUA_TTABLE);
		lua_getfield(s, tidx, "r");
		or = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "g");
		og = luaL_checknumber(s, ++idx);
		lua_getfield(s, tidx, "b");
		ob = luaL_checknumber(s, ++idx);
	}
	SDL_Color outline = { .r = or, .g = og, .b = ob };
	const char *id = NULL;
	idx++;
	if(lua_getfield(s, 1, "id") != LUA_TNIL)
		id = luaL_checkstring(s, idx);
	int alpha = 255;
	idx++;
	if(lua_getfield(s, 1, "alpha") != LUA_TNIL)
		alpha = luaL_checknumber(s, idx);
	bool absolute = false;
	idx++;
	if(lua_getfield(s, 1, "absolute") != LUA_TNIL)
		absolute = lua_toboolean(s, idx);
	int plane = 50;
	idx++;
	if(lua_getfield(s, 1, "plane") != LUA_TNIL)
		plane = luaL_checknumber(s, idx);
	/* Invoke the application callback to enforce this */
	kiavc_cb->show_text(id, text, font, &color,
		(or != -1 && og != -1 && ob != -1 ? &outline : NULL), x, y, alpha, absolute, plane, ms);
	return 0;
}

/* Float some text at some coordinates at a certain speed */
static int kiavc_lua_method_floattextto(lua_State *s) {
	/* This method allows the Lua script to move text around */
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	luaL_checktype(s, 1, LUA_TTABLE);
	lua_getfield(s, 1, "id");
	const char *id = luaL_checkstring(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing text ID\n");
		return 0;
	}
	lua_getfield(s, 1, "x");
	int x = luaL_checknumber(s, 3);
	lua_getfield(s, 1, "y");
	int y = luaL_checknumber(s, 4);
	lua_getfield(s, 1, "speed");
	int speed = luaL_checknumber(s, 5);
	/* Invoke the application callback to enforce this */
	kiavc_cb->float_text_to(id, x, y, speed);
	return 0;
}

/* Fade rendered text in */
static int kiavc_lua_method_fadetextin(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int ms = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing text ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_text_to(id, 255, ms);
	return 0;
}

/* Fade rendered text out */
static int kiavc_lua_method_fadetextout(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int ms = luaL_checknumber(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing text ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_text_to(id, 0, ms);
	return 0;
}

/* Fade rendered text to a specific alpha */
static int kiavc_lua_method_fadetextto(lua_State *s) {
	int n = lua_gettop(s), exp = 3;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int alpha = luaL_checknumber(s, 2);
	int ms = luaL_checknumber(s, 3);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing text ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->fade_text_to(id, alpha, ms);
	return 0;
}

/* Set the alpha for the rendered text */
static int kiavc_lua_method_settextalpha(lua_State *s) {
	int n = lua_gettop(s), exp = 2;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	int alpha = luaL_checkinteger(s, 2);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing text ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->set_text_alpha(id, alpha);
	return 0;
}

/* Remove rendered text, if an ID had been provided */
static int kiavc_lua_method_removetext(lua_State *s) {
	int n = lua_gettop(s), exp = 1;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	const char *id = luaL_checkstring(s, 1);
	if(id == NULL) {
		/* Ignore */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Missing text ID\n");
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->remove_text(id);
	return 0;
}

/* Quit */
static int kiavc_lua_method_quit(lua_State *s) {
	/* This method allows the Lua script to quit the game */
	int n = lua_gettop(s), exp = 0;
	if(n != exp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[Lua] Wrong number of arguments: %d (expected %d)\n", n, exp);
		return 0;
	}
	/* Invoke the application callback to enforce this */
	kiavc_cb->quit();
	return 0;
}
