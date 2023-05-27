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

#ifndef __KIAVC_SCRIPTS_H
#define __KIAVC_SCRIPTS_H

#include <stdbool.h>

/* Callbacks to notify the main application about calls from Lua scripts */
typedef struct kiavc_scripts_callbacks {
	bool (* const set_resolution)(int width, int height, int fps, int scale);
	bool (* const set_title)(const char *title);
	bool (* const set_icon)(const char *path);
	bool (* const grab_mouse)(bool grab);
	bool (* const is_grabbing_mouse)(void);
	bool (* const set_fullscreen)(bool fullscreen, bool desktop);
	bool (* const get_fullscreen)(void);
	bool (* const set_scanlines)(int alpha);
	int (* const get_scanlines)(void);
	bool (* const debug_objects)(bool debug);
	bool (* const is_debugging_objects)(void);
	bool (* const debug_walkboxes)(bool debug);
	bool (* const is_debugging_walkboxes)(void);
	bool (* const save_screenshot)(const char *path);
	bool (* const enable_console)(const char *font);
	bool (* const show_console)(void);
	bool (* const hide_console)(void);
	bool (* const disable_console)(void);
	bool (* const is_console_enabled)(void);
	bool (* const is_console_visible)(void);
	bool (* const enable_input)(void);
	bool (* const disable_input)(void);
	bool (* const is_input_enabled)(void);
	bool (* const start_cutscene)(void);
	bool (* const stop_cutscene)(void);
	bool (* const fade_in)(int ms);
	bool (* const fade_out)(int ms);
	bool (* const start_dialog)(const char *id, const char *font, SDL_Color *color, SDL_Color *outline,
		SDL_Color *s_color, SDL_Color *s_outline, SDL_Color *background, SDL_Rect *area, bool autohide);
	bool (* const add_dialog_line)(const char *id, const char *name, const char *text);
	bool (* const stop_dialog)(const char *id);
	bool (* const register_image)(const char *id, const char *path, SDL_Color *transparency);
	bool (* const register_animation)(const char *id, const char *path, int frames, int ms, SDL_Color *transparency);
	bool (* const register_font)(const char *id, const char *path, int size);
	bool (* const register_cursor)(const char *id);
	bool (* const set_cursor_animation)(const char *id, const char *anim);
	bool (* const set_main_cursor)(const char *id);
	bool (* const set_hotspot_cursor)(const char *id);
	bool (* const show_cursor)(void);
	bool (* const hide_cursor)(void);
	bool (* const show_cursor_text)(const char *font, const char *text, SDL_Color *color, SDL_Color *outline);
	bool (* const hide_cursor_text)(void);
	bool (* const register_audio)(const char *id, const char *path);
	bool (* const play_audio)(const char *id, int fade_ms, bool loop);
	bool (* const pause_audio)(const char *id);
	bool (* const resume_audio)(const char *id);
	bool (* const stop_audio)(const char *id, int fade_ms);
	bool (* const register_room)(const char *id);
	bool (* const set_room_background)(const char *id, const char *bg);
	bool (* const add_room_layer)(const char *id, const char *name, const char *bg, int zplane);
	bool (* const remove_room_layer)(const char *id, const char *name);
	bool (* const add_room_walkbox)(const char *id, const char *name, int x1, int y1, int x2, int y2, float scale, float speed, bool disabled);
	bool (* const enable_room_walkbox)(const char *id, const char *name);
	bool (* const disable_room_walkbox)(const char *id, const char *name);
	bool (* const recalculate_room_walkboxes)(const char *id);
	bool (* const show_room)(const char *id);
	bool (* const register_actor)(const char *id);
	bool (* const set_actor_costume)(const char *id, const char *cost);
	bool (* const move_actor_to)(const char *id, const char *room, int x, int y);
	bool (* const show_actor)(const char *id);
	bool (* const follow_actor)(const char *id);
	bool (* const hide_actor)(const char *id);
	bool (* const fade_actor_to)(const char *id, int alpha, int ms);
	bool (* const set_actor_alpha)(const char *id, int alpha);
	bool (* const set_actor_plane)(const char *id, int zplane);
	bool (* const set_actor_speed)(const char *id, int speed);
	bool (* const scale_actor)(const char *id, float scale);
	bool (* const walk_actor_to)(const char *id, int x, int y);
	bool (* const say_actor)(const char *id, const char *text, const char *font, SDL_Color *color, SDL_Color *outline);
	bool (* const set_actor_direction)(const char *id, const char *direction);
	bool (* const controlled_actor)(const char *id);
	bool (* const skip_actors_text)(void);
	bool (* const set_actor_state)(const char *id, const char *type);
	bool (* const register_costume)(const char *id);
	bool (* const set_costume_animation)(const char *id, const char *type, const char *direction, const char *anim);
	bool (* const register_object)(const char *id);
	bool (* const set_object_animation)(const char *id, const char *state, const char *anim);
	bool (* const set_object_interactable)(const char *id, bool ui);
	bool (* const set_object_ui)(const char *id, bool ui);
	bool (* const set_object_ui_position)(const char *id, int x, int y);
	bool (* const set_object_ui_animation)(const char *id, const char *anim);
	bool (* const set_object_parent)(const char *id, const char *parent);
	bool (* const remove_object_parent)(const char *id);
	bool (* const move_object_to)(const char *id, const char *room, int x, int y);
	bool (* const float_object_to)(const char *id, int x, int y, int speed);
	bool (* const set_object_hover)(const char *id, int from_x, int from_y, int to_x, int to_y);
	bool (* const show_object)(const char *id);
	bool (* const hide_object)(const char *id);
	bool (* const fade_object_to)(const char *id, int alpha, int ms);
	bool (* const set_object_alpha)(const char *id, int alpha);
	bool (* const set_object_plane)(const char *id, int zplane);
	bool (* const set_object_state)(const char *id, const char *state);
	bool (* const scale_object)(const char *id, float scale);
	bool (* const add_object_to_inventory)(const char *id, const char *owner);
	bool (* const remove_object_from_inventory)(const char *id, const char *owner);
	bool (* const show_text)(const char *id, const char *text, const char *font, SDL_Color *color, SDL_Color *outline,
		int x, int y, int alpha, bool absolute, int zplane, Uint32 ms);
	bool (* const float_text_to)(const char *id, int x, int y, int speed);
	bool (* const fade_text_to)(const char *id, int alpha, int ms);
	bool (* const set_text_alpha)(const char *id, int alpha);
	bool (* const remove_text)(const char *id);
	bool (* const load_plugin)(const char *name);
	bool (* const quit)(void);
} kiavc_scripts_callbacks;

/* Initialize the script engine and load the main script */
int kiavc_scripts_load(const char *path, const kiavc_scripts_callbacks *callbacks);
/* Run the provided script command */
void kiavc_scripts_run_command(const char *fmt, ...);
/* Update the world in the script */
int kiavc_scripts_update_world(Uint32 ticks);
/* Register an external function */
void kiavc_scripts_register_function(const char *name, int (* const function)(void *s));
/* Close the script engine */
void kiavc_scripts_unload(void);

#endif
