/*
 *
 * KIAVC scripting interface, specifically using Lua. The interfaces
 * with the engine are abstracted so that in theory different scripting
 * languages can be used in the future, even though they're mostly
 * tailored to be used with Lua. Functions implemented in the engine
 * are exposed here as Lua functions, and bubbled up to the engine via
 * callbacks that just contain the provided data.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_SCRIPTS_H
#define __KIAVC_SCRIPTS_H

#include <stdbool.h>

/* Callbacks to notify the main application about calls from Lua scripts */
typedef struct kiavc_scripts_callbacks {
	void (* const set_resolution)(int width, int height, int fps, int scale);
	void (* const set_title)(const char *title);
	void (* const set_icon)(const char *path);
	void (* const grab_mouse)(bool grab);
	bool (* const is_grabbing_mouse)(void);
	void (* const set_fullscreen)(bool fullscreen, bool desktop);
	bool (* const get_fullscreen)(void);
	void (* const set_scanlines)(bool scanlines);
	bool (* const get_scanlines)(void);
	void (* const debug_objects)(bool debug);
	bool (* const is_debugging_objects)(void);
	void (* const debug_walkboxes)(bool debug);
	bool (* const is_debugging_walkboxes)(void);
	void (* const save_screenshot)(const char *path);
	void (* const enable_console)(const char *font);
	void (* const show_console)(void);
	void (* const hide_console)(void);
	void (* const disable_console)(void);
	bool (* const is_console_enabled)(void);
	bool (* const is_console_visible)(void);
	void (* const enable_input)(void);
	void (* const disable_input)(void);
	bool (* const is_input_enabled)(void);
	void (* const start_cutscene)(void);
	void (* const stop_cutscene)(void);
	void (* const fade_in)(int ms);
	void (* const fade_out)(int ms);
	void (* const start_dialog)(const char *id, const char *font, SDL_Color *color, SDL_Color *outline,
		SDL_Color *s_color, SDL_Color *s_outline, SDL_Color *background, SDL_Rect *area, bool autohide);
	void (* const add_dialog_line)(const char *id, const char *name, const char *text);
	void (* const stop_dialog)(const char *id);
	void (* const register_image)(const char *id, const char *path, SDL_Color *transparency);
	void (* const register_animation)(const char *id, const char *path, int frames, int ms, SDL_Color *transparency);
	void (* const register_font)(const char *id, const char *path, int size);
	void (* const register_cursor)(const char *id);
	void (* const set_cursor_animation)(const char *id, const char *anim);
	void (* const set_main_cursor)(const char *id);
	void (* const set_hotspot_cursor)(const char *id);
	void (* const show_cursor)(void);
	void (* const hide_cursor)(void);
	void (* const show_cursor_text)(const char *font, const char *text, SDL_Color *color, SDL_Color *outline);
	void (* const hide_cursor_text)(void);
	void (* const register_audio)(const char *id, const char *path);
	void (* const play_audio)(const char *id, int fade_ms, bool loop);
	void (* const pause_audio)(const char *id);
	void (* const resume_audio)(const char *id);
	void (* const stop_audio)(const char *id, int fade_ms);
	void (* const register_room)(const char *id);
	void (* const set_room_background)(const char *id, const char *bg);
	void (* const add_room_layer)(const char *id, const char *name, const char *bg, int zplane);
	void (* const remove_room_layer)(const char *id, const char *name);
	void (* const add_room_walkbox)(const char *id, const char *name, int x1, int y1, int x2, int y2, float scale, float speed, bool disabled);
	void (* const enable_room_walkbox)(const char *id, const char *name);
	void (* const disable_room_walkbox)(const char *id, const char *name);
	void (* const recalculate_room_walkboxes)(const char *id);
	void (* const show_room)(const char *id);
	void (* const register_actor)(const char *id);
	void (* const set_actor_costume)(const char *id, const char *cost);
	void (* const move_actor_to)(const char *id, const char *room, int x, int y);
	void (* const show_actor)(const char *id);
	void (* const follow_actor)(const char *id);
	void (* const hide_actor)(const char *id);
	void (* const fade_actor_to)(const char *id, int alpha, int ms);
	void (* const set_actor_alpha)(const char *id, int alpha);
	void (* const set_actor_plane)(const char *id, int zplane);
	void (* const set_actor_speed)(const char *id, int speed);
	void (* const scale_actor)(const char *id, float scale);
	void (* const walk_actor_to)(const char *id, int x, int y);
	void (* const say_actor)(const char *id, const char *text, const char *font, SDL_Color *color, SDL_Color *outline);
	void (* const set_actor_direction)(const char *id, const char *direction);
	void (* const controlled_actor)(const char *id);
	void (* const skip_actors_text)(void);
	void (* const set_actor_state)(const char *id, const char *type);
	void (* const register_costume)(const char *id);
	void (* const set_costume_animation)(const char *id, const char *type, const char *direction, const char *anim);
	void (* const register_object)(const char *id);
	void (* const set_object_animation)(const char *id, const char *anim);
	void (* const set_object_interactable)(const char *id, bool ui);
	void (* const set_object_ui)(const char *id, bool ui);
	void (* const set_object_ui_position)(const char *id, int x, int y);
	void (* const set_object_ui_animation)(const char *id, const char *anim);
	void (* const set_object_parent)(const char *id, const char *parent);
	void (* const remove_object_parent)(const char *id);
	void (* const move_object_to)(const char *id, const char *room, int x, int y);
	void (* const float_object_to)(const char *id, int x, int y, int speed);
	void (* const set_object_hover)(const char *id, int from_x, int from_y, int to_x, int to_y);
	void (* const show_object)(const char *id);
	void (* const hide_object)(const char *id);
	void (* const fade_object_to)(const char *id, int alpha, int ms);
	void (* const set_object_alpha)(const char *id, int alpha);
	void (* const set_object_plane)(const char *id, int zplane);
	void (* const scale_object)(const char *id, float scale);
	void (* const add_object_to_inventory)(const char *id, const char *owner);
	void (* const remove_object_from_inventory)(const char *id, const char *owner);
	void (* const show_text)(const char *id, const char *text, const char *font, SDL_Color *color, SDL_Color *outline,
		int x, int y, int alpha, bool absolute, int zplane, Uint32 ms);
	void (* const float_text_to)(const char *id, int x, int y, int speed);
	void (* const fade_text_to)(const char *id, int alpha, int ms);
	void (* const set_text_alpha)(const char *id, int alpha);
	void (* const remove_text)(const char *id);
	void (* const quit)(void);
} kiavc_scripts_callbacks;

/* Initialize the script engine and load the main script */
int kiavc_scripts_load(const char *path, const kiavc_scripts_callbacks *callbacks);
/* Run the provided script command */
void kiavc_scripts_run_command(const char *fmt, ...);
/* Update the world in the script */
int kiavc_scripts_update_world(Uint32 ticks);
/* Close the script engine */
void kiavc_scripts_unload(void);

#endif
