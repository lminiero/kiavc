/*
 *
 * Main KIAVC engine implementation. It takes care of the C side of
 * things, with respect to user input (mouse, keyboard), updating the
 * world (e.g., progressing animations according to the ticks) and
 * actual audio and video rendering using SDL. Besides, it interacts
 * with the Lua scripts in both directions.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"
#include "utils.h"
#include "bag.h"
#include "icon.h"
#include "resources.h"
#include "pathfinding.h"
#include "map.h"
#include "list.h"
#include "scripts.h"
#include "cursor.h"
#include "font.h"
#include "animation.h"
#include "audio.h"
#include "room.h"
#include "actor.h"
#include "costume.h"
#include "object.h"
#include "dialog.h"

/* Global SDL resources */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *canvas = NULL;
static char *app_path = NULL;
static bool quit = false;

/* Window properties */
static char *kiavc_screen_title = NULL;
static char *kiavc_screen_icon = NULL;
static int kiavc_screen_scale = -1, kiavc_screen_scale_prev = -1;
static int kiavc_screen_width = -1;
static int kiavc_screen_height = -1;
static int kiavc_screen_fps = -1;
static bool kiavc_screen_grab_mouse = false;
static bool kiavc_screen_fullscreen = false, kiavc_screen_fullscreen_desktop = false;
static bool kiavc_screen_scanlines = false;
static SDL_Texture *kiavc_screen_scanlines_texture = NULL;

/* Test console */
static bool console_active = false;
static kiavc_font *console_font = NULL;
static char console_text[256];
static kiavc_font_text *console_rendered = NULL;
static kiavc_list *console_history = NULL, *console_current = NULL;

/* Visual debugging */
static bool kiavc_debug_objects = false, kiavc_debug_walkboxes = false;

/* Assets connection */
static kiavc_bag *bag = NULL;

/* Object maps */
static kiavc_map *animations = NULL;
static kiavc_map *fonts = NULL;
static kiavc_map *cursors = NULL;
static kiavc_map *audios = NULL;
static kiavc_map *rooms = NULL;
static kiavc_map *actors = NULL;
static kiavc_map *costumes = NULL;
static kiavc_map *objects = NULL;
static kiavc_map *texts = NULL;
static kiavc_map *dialogs = NULL;

/* Engine struct */
typedef struct kiavc_engine {
	/* Whether a cutscene is running (no interaction) */
	bool cutscene;
	/* Whether we should sneakily ignore user input */
	bool input_disabled;
	/* Whether we're fading in or out, and how long that should be */
	int fade_in, fade_out;
	/* Fade in/out black texture */
	SDL_Texture *fade_texture;
	/* Fade alpha to apply */
	Uint8 fade_alpha;
	/* Fade in/out ticks */
	uint32_t fade_ticks;
	/* Other fading resources, if any */
	kiavc_list *fading;
	/* Current mouse coordinates */
	int mouse_x, mouse_y;
	/* Current cursors (main and/or hotspot) */
	kiavc_cursor *main_cursor, *hotspot_cursor;
	/* Whether the cursor is supposed to be visible or not */
	bool cursor_visible;
	/* Current cursor text, if any */
	kiavc_font_text *cursor_text;
	/* Current room */
	kiavc_room *room;
	/* Room ticks */
	uint32_t room_ticks;
	/* FIXME Room directions, for scrolling */
	int room_direction_x, room_direction_y;
	/* Current controlled actor */
	kiavc_actor *actor;
	/* Actor we're following with the camera */
	kiavc_actor *following;
	/* List of resources to render */
	kiavc_list *render_list;
	/* Resource we're currently hovering on */
	kiavc_resource *hovering;
	/* Dialog we're running, if any */
	kiavc_dialog *dialog;
	/* Rendering ticks */
	uint32_t render_ticks;
} kiavc_engine;
static kiavc_engine engine = { 0 };

/* Scripting callbacks */
static void kiavc_engine_set_resolution(int width, int height, int fps, int scale);
static void kiavc_engine_set_title(const char *title);
static void kiavc_engine_set_icon(const char *path);
static void kiavc_engine_grab_mouse(bool grab);
static bool kiavc_engine_is_grabbing_mouse(void);
static void kiavc_engine_set_fullscreen(bool fullscreen, bool desktop);
static bool kiavc_engine_get_fullscreen(void);
static void kiavc_engine_set_scanlines(bool scanlines);
static bool kiavc_engine_get_scanlines(void);
static void kiavc_engine_debug_objects(bool debug);
static bool kiavc_engine_is_debugging_objects(void);
static void kiavc_engine_debug_walkboxes(bool debug);
static bool kiavc_engine_is_debugging_walkboxes(void);
static void kiavc_engine_save_screenshot(const char *path);
static void kiavc_engine_enable_console(const char *font);
static void kiavc_engine_show_console(void);
static void kiavc_engine_hide_console(void);
static void kiavc_engine_disable_console(void);
static bool kiavc_engine_is_console_enabled(void);
static bool kiavc_engine_is_console_visible(void);
static void kiavc_engine_enable_input(void);
static void kiavc_engine_disable_input(void);
static bool kiavc_engine_is_input_enabled(void);
static void kiavc_engine_start_cutscene(void);
static void kiavc_engine_stop_cutscene(void);
static void kiavc_engine_fade_in(int ms);
static void kiavc_engine_fade_out(int ms);
static void kiavc_engine_start_dialog(const char *id, const char *font, SDL_Color *color, SDL_Color *outline,
	SDL_Color *s_color, SDL_Color *s_outline, SDL_Color *background, SDL_Rect *area, bool autohide);
static void kiavc_engine_add_dialog_line(const char *id, const char *name, const char *text);
static void kiavc_engine_stop_dialog(const char *id);
static void kiavc_engine_register_animation(const char *id, const char *path, int ms, int frames, SDL_Color *transparency);
static void kiavc_engine_register_font(const char *id, const char *path, int size);
static void kiavc_engine_register_cursor(const char *id);
static void kiavc_engine_set_cursor_animation(const char *id, const char *canim);
static void kiavc_engine_set_main_cursor(const char *id);
static void kiavc_engine_set_hotspot_cursor(const char *id);
static void kiavc_engine_show_cursor(void);
static void kiavc_engine_hide_cursor(void);
static void kiavc_engine_show_cursor_text(const char *font, const char *text, SDL_Color *color, SDL_Color *outline);
static void kiavc_engine_hide_cursor_text(void);
static void kiavc_engine_register_audio(const char *id, const char *path);
static void kiavc_engine_play_audio(const char *id, int fade_ms, bool loop);
static void kiavc_engine_pause_audio(const char *id);
static void kiavc_engine_resume_audio(const char *id);
static void kiavc_engine_stop_audio(const char *id, int fade_ms);
static void kiavc_engine_register_room(const char *id);
static void kiavc_engine_set_room_background(const char *id, const char *bg);
static void kiavc_engine_add_room_layer(const char *id, const char *name, const char *bg, int zplane);
static void kiavc_engine_remove_room_layer(const char *id, const char *name);
static void kiavc_engine_add_room_walkbox(const char *id, const char *name, int x1, int y1, int x2, int y2, float scale, float speed, bool disabled);
static void kiavc_engine_enable_room_walkbox(const char *id, const char *name);
static void kiavc_engine_disable_room_walkbox(const char *id, const char *name);
static void kiavc_engine_recalculate_room_walkboxes(const char *id);
static void kiavc_engine_show_room(const char *id);
static void kiavc_engine_register_actor(const char *id);
static void kiavc_engine_set_actor_costume(const char *id, const char *cost);
static void kiavc_engine_move_actor_to(const char *id, const char *room, int x, int y);
static void kiavc_engine_show_actor(const char *id);
static void kiavc_engine_follow_actor(const char *id);
static void kiavc_engine_hide_actor(const char *id);
static void kiavc_engine_fade_actor_to(const char *id, int alpha, int ms);
static void kiavc_engine_set_actor_alpha(const char *id, int alpha);
static void kiavc_engine_set_actor_plane(const char *id, int zplane);
static void kiavc_engine_set_actor_speed(const char *id, int speed);
static void kiavc_engine_scale_actor(const char *id, float scale);
static void kiavc_engine_walk_actor_to(const char *id, int x, int y);
static void kiavc_engine_say_actor(const char *id, const char *text, const char *font, SDL_Color *color, SDL_Color *outline);
static void kiavc_engine_set_actor_direction(const char *id, const char *direction);
static void kiavc_engine_controlled_actor(const char *id);
static void kiavc_engine_skip_actors_text(void);
static void kiavc_engine_set_actor_state(const char *id, const char *type);
static void kiavc_engine_register_costume(const char *id);
static void kiavc_engine_set_costume_animation(const char *id, const char *type, const char *direction, const char *canim);
static void kiavc_engine_register_object(const char *id);
static void kiavc_engine_set_object_animation(const char *id, const char *canim);
static void kiavc_engine_set_object_interactable(const char *id, bool interactable);
static void kiavc_engine_set_object_ui(const char *id, bool ui);
static void kiavc_engine_set_object_ui_position(const char *id, int x, int y);
static void kiavc_engine_set_object_ui_animation(const char *id, const char *canim);
static void kiavc_engine_set_object_parent(const char *id, const char *parent);
static void kiavc_engine_remove_object_parent(const char *id);
static void kiavc_engine_move_object_to(const char *id, const char *room, int x, int y);
static void kiavc_engine_float_object_to(const char *id, int x, int y, int speed);
static void kiavc_engine_set_object_hover(const char *id, int from_x, int from_y, int to_x, int to_y);
static void kiavc_engine_show_object(const char *id);
static void kiavc_engine_hide_object(const char *id);
static void kiavc_engine_fade_object_to(const char *id, int alpha, int ms);
static void kiavc_engine_set_object_alpha(const char *id, int alpha);
static void kiavc_engine_set_object_plane(const char *id, int zplane);
static void kiavc_engine_scale_object(const char *id, float scale);
static void kiavc_engine_add_object_to_inventory(const char *id, const char *owner);
static void kiavc_engine_remove_object_from_inventory(const char *id, const char *owner);
static void kiavc_engine_show_text(const char *id, const char *text, const char *font,
	SDL_Color *color, SDL_Color *outline, int x, int y, int alpha, bool absolute, int zplane, Uint32 ms);
static void kiavc_engine_float_text_to(const char *id, int x, int y, int speed);
static void kiavc_engine_fade_text_to(const char *id, int alpha, int ms);
static void kiavc_engine_set_text_alpha(const char *id, int alpha);
static void kiavc_engine_remove_text(const char *id);
static void kiavc_engine_quit(void);
static kiavc_scripts_callbacks scripts_callbacks =
	{
		.set_resolution = kiavc_engine_set_resolution,
		.set_title = kiavc_engine_set_title,
		.set_icon = kiavc_engine_set_icon,
		.grab_mouse = kiavc_engine_grab_mouse,
		.is_grabbing_mouse = kiavc_engine_is_grabbing_mouse,
		.set_fullscreen = kiavc_engine_set_fullscreen,
		.get_fullscreen = kiavc_engine_get_fullscreen,
		.set_scanlines = kiavc_engine_set_scanlines,
		.get_scanlines = kiavc_engine_get_scanlines,
		.debug_objects = kiavc_engine_debug_objects,
		.is_debugging_objects = kiavc_engine_is_debugging_objects,
		.debug_walkboxes = kiavc_engine_debug_walkboxes,
		.is_debugging_walkboxes = kiavc_engine_is_debugging_walkboxes,
		.save_screenshot = kiavc_engine_save_screenshot,
		.enable_console = kiavc_engine_enable_console,
		.show_console = kiavc_engine_show_console,
		.hide_console = kiavc_engine_hide_console,
		.disable_console = kiavc_engine_disable_console,
		.is_console_enabled = kiavc_engine_is_console_enabled,
		.is_console_visible = kiavc_engine_is_console_visible,
		.enable_input = kiavc_engine_enable_input,
		.disable_input = kiavc_engine_disable_input,
		.is_input_enabled = kiavc_engine_is_input_enabled,
		.start_cutscene = kiavc_engine_start_cutscene,
		.stop_cutscene = kiavc_engine_stop_cutscene,
		.fade_in = kiavc_engine_fade_in,
		.fade_out = kiavc_engine_fade_out,
		.start_dialog = kiavc_engine_start_dialog,
		.add_dialog_line = kiavc_engine_add_dialog_line,
		.stop_dialog = kiavc_engine_stop_dialog,
		.register_animation = kiavc_engine_register_animation,
		.register_font = kiavc_engine_register_font,
		.register_cursor = kiavc_engine_register_cursor,
		.set_cursor_animation = kiavc_engine_set_cursor_animation,
		.set_main_cursor = kiavc_engine_set_main_cursor,
		.set_hotspot_cursor = kiavc_engine_set_hotspot_cursor,
		.show_cursor = kiavc_engine_show_cursor,
		.hide_cursor = kiavc_engine_hide_cursor,
		.show_cursor_text = kiavc_engine_show_cursor_text,
		.hide_cursor_text = kiavc_engine_hide_cursor_text,
		.register_audio = kiavc_engine_register_audio,
		.play_audio = kiavc_engine_play_audio,
		.pause_audio = kiavc_engine_pause_audio,
		.resume_audio = kiavc_engine_resume_audio,
		.stop_audio = kiavc_engine_stop_audio,
		.register_room = kiavc_engine_register_room,
		.set_room_background = kiavc_engine_set_room_background,
		.add_room_layer = kiavc_engine_add_room_layer,
		.remove_room_layer = kiavc_engine_remove_room_layer,
		.add_room_walkbox = kiavc_engine_add_room_walkbox,
		.enable_room_walkbox = kiavc_engine_enable_room_walkbox,
		.disable_room_walkbox = kiavc_engine_disable_room_walkbox,
		.recalculate_room_walkboxes = kiavc_engine_recalculate_room_walkboxes,
		.show_room = kiavc_engine_show_room,
		.register_actor = kiavc_engine_register_actor,
		.set_actor_costume = kiavc_engine_set_actor_costume,
		.move_actor_to = kiavc_engine_move_actor_to,
		.show_actor = kiavc_engine_show_actor,
		.follow_actor = kiavc_engine_follow_actor,
		.hide_actor = kiavc_engine_hide_actor,
		.fade_actor_to = kiavc_engine_fade_actor_to,
		.set_actor_alpha = kiavc_engine_set_actor_alpha,
		.set_actor_plane = kiavc_engine_set_actor_plane,
		.set_actor_speed = kiavc_engine_set_actor_speed,
		.scale_actor = kiavc_engine_scale_actor,
		.walk_actor_to = kiavc_engine_walk_actor_to,
		.say_actor = kiavc_engine_say_actor,
		.set_actor_direction = kiavc_engine_set_actor_direction,
		.controlled_actor = kiavc_engine_controlled_actor,
		.skip_actors_text = kiavc_engine_skip_actors_text,
		.set_actor_state = kiavc_engine_set_actor_state,
		.register_costume = kiavc_engine_register_costume,
		.set_costume_animation = kiavc_engine_set_costume_animation,
		.register_object = kiavc_engine_register_object,
		.set_object_animation = kiavc_engine_set_object_animation,
		.set_object_interactable = kiavc_engine_set_object_interactable,
		.set_object_ui = kiavc_engine_set_object_ui,
		.set_object_ui_position = kiavc_engine_set_object_ui_position,
		.set_object_ui_animation = kiavc_engine_set_object_ui_animation,
		.set_object_parent = kiavc_engine_set_object_parent,
		.remove_object_parent = kiavc_engine_remove_object_parent,
		.move_object_to = kiavc_engine_move_object_to,
		.float_object_to = kiavc_engine_float_object_to,
		.set_object_hover = kiavc_engine_set_object_hover,
		.show_object = kiavc_engine_show_object,
		.hide_object = kiavc_engine_hide_object,
		.fade_object_to = kiavc_engine_fade_object_to,
		.set_object_alpha = kiavc_engine_set_object_alpha,
		.set_object_plane = kiavc_engine_set_object_plane,
		.scale_object = kiavc_engine_scale_object,
		.add_object_to_inventory = kiavc_engine_add_object_to_inventory,
		.remove_object_from_inventory = kiavc_engine_remove_object_from_inventory,
		.show_text = kiavc_engine_show_text,
		.float_text_to = kiavc_engine_float_text_to,
		.fade_text_to = kiavc_engine_fade_text_to,
		.set_text_alpha = kiavc_engine_set_text_alpha,
		.remove_text = kiavc_engine_remove_text,
		.quit = kiavc_engine_quit,
	};

/* Helper to renegerate the scanlines texture (if scanlines are enabled) */
static void kiavc_engine_regenerate_scanlines(void) {
	if(kiavc_screen_width == 0 || kiavc_screen_height == 0 || !renderer)
		return;
	if(kiavc_screen_scanlines_texture)
		SDL_DestroyTexture(kiavc_screen_scanlines_texture);
	kiavc_screen_scanlines_texture = NULL;
	if(kiavc_screen_scanlines) {
		int w = kiavc_screen_width * kiavc_screen_scale;
		int h = kiavc_screen_height * kiavc_screen_scale;
		SDL_Surface *scanlines = kiavc_create_surface(w, h);
		if(!scanlines)
			return;
		Uint32 color = SDL_MapRGB(scanlines->format, 0, 0, 0);
		SDL_Rect rect = { .x = 0, .y = 0, .w = w, .h = 1 };
		int i = 0;
		for(i=1; i<h; i+= 3) {
			rect.y = i;
			SDL_FillRect(scanlines, &rect, color);
		}
		kiavc_screen_scanlines_texture = SDL_CreateTextureFromSurface(renderer, scanlines);
		SDL_FreeSurface(scanlines);
		SDL_SetTextureBlendMode(kiavc_screen_scanlines_texture, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(kiavc_screen_scanlines_texture, 24);
	}
}

/* Helper to renegerate the fade in/out black texture (if fading) */
static void kiavc_engine_regenerate_fade(void) {
	if(engine.fade_texture || engine.fade_alpha == 0)
		return;
	SDL_Surface *surface = kiavc_create_surface(kiavc_screen_width, kiavc_screen_height);
	if(!surface)
		return;
	Uint32 color = SDL_MapRGB(surface->format, 0, 0, 0);
	SDL_FillRect(surface, NULL, color);
	engine.fade_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
}

/* Helper to trigger a fullscreen change */
static void kiavc_engine_trigger_fullscreen(void) {
	if(kiavc_screen_width == 0 || kiavc_screen_height == 0 || !window || !renderer)
		return;
	if(!kiavc_screen_fullscreen) {
		/* Back to windowed mode */
		SDL_Log("Windowed mode\n");
		SDL_SetWindowFullscreen(window, 0);
		if(kiavc_screen_scale_prev > 0) {
			kiavc_screen_scale = kiavc_screen_scale_prev;
			SDL_SetWindowSize(window, kiavc_screen_width * kiavc_screen_scale, kiavc_screen_height * kiavc_screen_scale);
		}
		return;
	}
	/* We need to go fullscreen, check the mode */
	if(!kiavc_screen_fullscreen_desktop) {
		/* Just go "real" fullscreen */
		SDL_Log("Fullscreen mode\n");
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	} else {
		/* We're using the desktop mode, query the display first */
		int display = SDL_GetWindowDisplayIndex(window);
		if(display < 0) {
			/* Just go "real" fullscreen */
			kiavc_screen_fullscreen_desktop = false;
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't query window display, using Fullscreen mode\n");
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		} else {
			SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
			if(SDL_GetDisplayMode(display, 0, &mode) < 0) {
				/* Just go "real" fullscreen */
				SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Couldn't get display mode, using Fullscreen mode\n");
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			} else {
				int w = kiavc_screen_width;
				int h = kiavc_screen_height;
				if(mode.w <= w || mode.h <= h) {
					/* Just go "real" fullscreen */
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Display mode resolution too small, using Fullscreen mode\n");
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
				} else {
					SDL_Log("Fullscreen mode (desktop)\n");
					int scale_w = mode.w / w;
					int scale_h = mode.h / h;
					kiavc_screen_scale_prev = kiavc_screen_scale;
					kiavc_screen_scale = scale_w < scale_h ? scale_w : scale_h;
					SDL_SetWindowSize(window, kiavc_screen_width * kiavc_screen_scale, kiavc_screen_height * kiavc_screen_scale);
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
				}
			}
		}
	}
}

/* Helper to sort rendering resources */
static int kiavc_engine_sort_resources(const kiavc_resource *r1, const kiavc_resource *r2) {
	if(!r1 && !r2)
		return 0;
	else if(!r1)
		return -1;
	else if(!r2)
		return 1;
	/* We check the z-plane first */
	if(r1->zplane != r2->zplane)
		return r1->zplane - r2->zplane;
	/* For resources with the same z-plane, we check which one has a higher y coordinate */
	return r1->y - r2->y;
}

/* Initialize the engine */
int kiavc_engine_init(const char *app, kiavc_bag *bagfile) {
	bag = bagfile;

	/* Create maps */
	animations = kiavc_map_create((kiavc_map_value_destroy)&kiavc_animation_destroy);
	fonts = kiavc_map_create((kiavc_map_value_destroy)&kiavc_font_destroy);
	cursors = kiavc_map_create((kiavc_map_value_destroy)&kiavc_cursor_destroy);
	audios = kiavc_map_create((kiavc_map_value_destroy)&kiavc_audio_destroy);
	rooms = kiavc_map_create((kiavc_map_value_destroy)&kiavc_room_destroy);
	actors = kiavc_map_create((kiavc_map_value_destroy)&kiavc_actor_destroy);
	costumes = kiavc_map_create((kiavc_map_value_destroy)&kiavc_costume_destroy);
	objects = kiavc_map_create((kiavc_map_value_destroy)&kiavc_object_destroy);
	texts = kiavc_map_create((kiavc_map_value_destroy)&kiavc_font_text_destroy);
	dialogs = kiavc_map_create((kiavc_map_value_destroy)&kiavc_dialog_destroy);

	/* FIXME As in the logger, we get the path where we can save files, which
	 * we'll used for saved screenshots. And as in the logger, we're currently
	 * hardcoding "KIAVC" as both org and app, which needs to be changed. */
	app_path = SDL_GetPrefPath("KIAVC", app);

	/* Initialize the scripting engine */
	if(kiavc_scripts_load("./lua/main.lua", &scripts_callbacks) < 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error initializing scripting engine\n");
		return -1;
	}
	/* Make sure the script initialized a resolution and scaling */
	if(kiavc_screen_width < 1 || kiavc_screen_height < 1 || kiavc_screen_fps < 1 || kiavc_screen_scale < 1) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Invalid screen resolution\n");
		return -1;
	}
	if(!kiavc_screen_title)
		kiavc_screen_title = SDL_strdup("KIAVC Is an Adventure Videogame Creator (KIAVC)");

	/* Detect number of video displays */
	int displays = SDL_GetNumVideoDisplays();
	if(displays < 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error getting number of video displays: %s\n", SDL_GetError());
	} else {
		SDL_Log("There are %d connected screens\n", displays);
		/* FIXME Simple iterator on displays and display modes */
		int i = 0, j = 0, modes = 0;
		SDL_DisplayMode mode;
		for(i=0; i<displays; i++) {
			modes = SDL_GetNumDisplayModes(i);
			SDL_Log("[%d] There are %d display modes on this screen\n", i, modes);
			for(j=0; j<modes; j++) {
				if(SDL_GetDisplayMode(i, j, &mode) == 0) {
					SDL_Log("  -- %dx%d @ %dhz\n", mode.w, mode.h, mode.refresh_rate);
				}
			}
		}
	}
	/* Initialize SDL rendering and create the main window */
	window = SDL_CreateWindow(kiavc_screen_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		kiavc_screen_width * kiavc_screen_scale, kiavc_screen_height * kiavc_screen_scale, SDL_WINDOW_SHOWN);
	if(window == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error creating window: %s\n", SDL_GetError());
		return -1;
	}
	int display = SDL_GetWindowDisplayIndex(window);
	if(display < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error querying the window display: %s\n", SDL_GetError());
	} else {
		SDL_Log("The window was created on display #%d\n", display);
	}
	/* Set an icon for the window */
	SDL_Surface *icon = NULL;
	if(kiavc_screen_icon) {
		/* Load the provided icon */
		SDL_RWops *icon_rw = kiavc_engine_open_file(kiavc_screen_icon);
		icon = IMG_Load_RW(icon_rw, 1);
		if(!icon)
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load icon '%s', falling back to engine icon\n", kiavc_screen_icon);
	}
	if(!icon) {
		/* Use the hardcoded Kiavc icon */
		SDL_RWops *icon_rw = SDL_RWFromConstMem(icon_png.data, icon_png.size);
		icon = IMG_Load_RW(icon_rw, 1);
	}
	if(icon) {
		SDL_SetWindowIcon(window, icon);
		SDL_FreeSurface(icon);
	}
	/* Finally, create a renderer */
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error creating renderer: %s\n", SDL_GetError());
		return -1;
	}
	canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET, kiavc_screen_width, kiavc_screen_height);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	/* Check if we need to confine the mouse to the window */
	if(kiavc_screen_grab_mouse)
		SDL_SetWindowMouseGrab(window, kiavc_screen_grab_mouse);
	SDL_ShowCursor(SDL_DISABLE);
	/* Check if we need to start fullscreen */
	if(kiavc_screen_fullscreen)
		kiavc_engine_trigger_fullscreen();
	/* Check if we need to enable scanlines */
	if(kiavc_screen_scanlines)
		kiavc_engine_regenerate_scanlines();

	/* Done */
	return 0;
}

/* Return a SDL_RWops instance for a path */
SDL_RWops *kiavc_engine_open_file(const char *path) {
	if(!path)
		return NULL;
	/* Check if we have a BAG file or not */
	SDL_RWops *rwops = NULL;
	if(bag) {
		rwops = kiavc_bag_asset_export_rw(bag, path);
	} else {
		rwops = SDL_RWFromFile(path, "rb");
	}
	if(rwops == NULL)
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error opening asset file '%s'\n", path);
	return rwops;
}

/* Helper method to check if we're hovering on something */
static void kiavc_engine_check_hovering(void) {
	if(engine.main_cursor && engine.main_cursor->animation) {
		engine.main_cursor->res.x = engine.mouse_x - (engine.main_cursor->animation->w/2);
		engine.main_cursor->res.y = engine.mouse_y - (engine.main_cursor->animation->h/2);
	}
	if(engine.hotspot_cursor && engine.hotspot_cursor->animation) {
		engine.hotspot_cursor->res.x = engine.mouse_x - (engine.hotspot_cursor->animation->w/2);
		engine.hotspot_cursor->res.y = engine.mouse_y - (engine.hotspot_cursor->animation->h/2);
	}
	/* FIXME Check if we're hovering on an object */
	if(engine.room && !engine.cutscene && !engine.input_disabled && !engine.dialog) {
		int x = engine.mouse_x + (int)engine.room->res.x;
		int y = engine.mouse_y + (int)engine.room->res.y;
		kiavc_resource *resource, *hovering = NULL;
		kiavc_list *item = engine.render_list;
		while(item) {
			resource = (kiavc_resource *)item->data;
			if(resource->type == KIAVC_OBJECT) {
				/* FIXME Check if we're in the box */
				kiavc_object *object = (kiavc_object *)resource;
				if(!object->interactable) {
					item = item->next;
					continue;
				}
				if(!object->ui && object->room != engine.room) {
					item = item->next;
					continue;
				}
				if(object->hover.from_x >= 0 || object->hover.from_y >= 0 ||
						object->hover.to_x >= 0 || object->hover.to_y >= 0) {
					/* Use the provided hover coordinates to detect where the object is */
					if(x >= object->hover.from_x && y >= object->hover.from_y &&
							x <= object->hover.to_x && y <= object->hover.to_y) {
						hovering = resource;
					}
				} else {
					/* Use the object coordinates and the image size */
					kiavc_animation *animation = object->ui ? object->ui_animation : object->animation;
					int w = 0, h = 0;
					if(animation) {
						w = animation->w;
						h = animation->h;
					}
					if(object->scale) {
						w *= object->scale;
						h *= object->scale;
					}
					if(object->ui) {
						int ui_x = x - (int)engine.room->res.x;
						int ui_y = y - (int)engine.room->res.y;
						int object_x = (int)object->res.x + (object->parent ? (int)object->parent->res.x : 0);
						int object_y = (int)object->res.y + (object->parent ? (int)object->parent->res.y : 0);
						if(w > 0 && h > 0 && ui_x >= object_x && ui_y >= object_y &&
								ui_x <= object_x + w && ui_y <= object_y + h) {
							hovering = resource;
						}
					} else {
						int object_x = (int)object->res.x + (object->parent ? (int)object->parent->res.x : 0);
						int object_y = (int)object->res.y + (object->parent ? (int)object->parent->res.y : 0);
						if(w > 0 && h > 0 && x >= object_x - w/2 && y >= object_y - h &&
								x <= object_x + w/2 && y <= object_y) {
							hovering = resource;
						}
					}
				}
			} else if(resource->type == KIAVC_ACTOR) {
				/* FIXME Check if we're in the box */
				kiavc_actor *actor = (kiavc_actor *)resource;
				if(actor != engine.actor && actor->costume && actor->room && actor->room == engine.room) {
					int w = 0, h = 0;
					kiavc_costume_set *set = kiavc_costume_get_set(actor->costume, kiavc_actor_state_str(actor->state));
					if(set && set->animations[actor->direction]) {
						w = set->animations[actor->direction]->w;
						h = set->animations[actor->direction]->h;
					}
					if(actor->scale != 1.0 || (actor->walkbox && actor->walkbox->scale != 1.0)) {
						float ws = actor->walkbox ? actor->walkbox->scale : 1.0;
						w *= (actor->scale * ws);
						h *= (actor->scale * ws);
					}
					if(w > 0 && h > 0 && x >= (int)actor->res.x - w/2 && y >= (int)actor->res.y - h &&
							x <= (int)actor->res.x + w/2 && y <= (int)actor->res.y) {
						hovering = resource;
					}
				}
			}
			item = item->next;
		}
		if(hovering != engine.hovering) {
			if(engine.hovering) {
				/* We're not hovering on this resourse anymore */
				if(engine.hovering->type == KIAVC_OBJECT) {
					kiavc_object *object = (kiavc_object *)engine.hovering;
					SDL_Log("Stopped hovering over %s", object->id);
					kiavc_scripts_run_command("hovering('%s', false)", object->id);
				} else if(engine.hovering->type == KIAVC_ACTOR) {
					kiavc_actor *actor = (kiavc_actor *)engine.hovering;
					SDL_Log("Stopped hovering over %s", actor->id);
					kiavc_scripts_run_command("hovering('%s', false)", actor->id);
				} else {
					SDL_Log("Stopped hovering over unknown resource");
				}
			}
			if(hovering) {
				/* We're now hovering on this resourse */
				if(hovering->type == KIAVC_OBJECT) {
					kiavc_object *object = (kiavc_object *)hovering;
					SDL_Log("Hovering over %s", object->id);
					kiavc_scripts_run_command("hovering('%s', true)", object->id);
				} else if(hovering->type == KIAVC_ACTOR) {
					kiavc_actor *actor = (kiavc_actor *)hovering;
					SDL_Log("Hovering over %s", actor->id);
					kiavc_scripts_run_command("hovering('%s', true)", actor->id);
				} else {
					SDL_Log("Started hovering over unknown resource");
				}
			}
			engine.hovering = hovering;
		}
	} else if(engine.dialog) {
		/* We're now hovering on this resourse */
		if(engine.hovering) {
			if(engine.hovering->type == KIAVC_OBJECT) {
				kiavc_object *object = (kiavc_object *)engine.hovering;
				SDL_Log("Stopped hovering over %s", object->id);
				kiavc_scripts_run_command("hovering('%s', false)", object->id);
			} else if(engine.hovering && engine.hovering->type == KIAVC_ACTOR) {
				kiavc_actor *actor = (kiavc_actor *)engine.hovering;
				SDL_Log("Stopped hovering over %s", actor->id);
				kiavc_scripts_run_command("hovering('%s', false)", actor->id);
			}
			engine.hovering = NULL;
		}
		/* FIXME We should check if we're hovering on a dialog line */
		int x = engine.mouse_x;
		int y = engine.mouse_y;
		kiavc_dialog_line *selected = NULL;
		if(x >= engine.dialog->area.x && y>= engine.dialog->area.y &&
				x <= engine.dialog->area.x + engine.dialog->area.w &&
				y <= engine.dialog->area.y + engine.dialog->area.h) {
			/* FIXME We're in the dialog area */
			y -= engine.dialog->area.y;
			int size = engine.dialog->area.h / 4;
			int index = y/size;
			kiavc_list *item = g_list_nth(engine.dialog->lines, index);
			if(item) {
				selected = (kiavc_dialog_line *)item->data;
			}
		}
		if(engine.dialog->selected) {
			engine.render_list = kiavc_list_remove(engine.render_list, engine.dialog->selected->selected);
			engine.render_list = kiavc_list_remove(engine.render_list, engine.dialog->selected->text);
			engine.render_list = kiavc_list_append(engine.render_list, engine.dialog->selected->text);
		}
		engine.dialog->selected = selected;
		if(selected)
			engine.render_list = kiavc_list_append(engine.render_list, selected->selected);
	}
}

/* Handle input from the user */
int kiavc_engine_handle_input(void) {
	if(quit)
		return -1;
	/* Poll for events */
	SDL_Event e = { 0 };
	while(SDL_PollEvent(&e) != 0) {
		if(e.type == SDL_QUIT) {
			/* No need to go on */
			return -1;
		} else if(e.type == SDL_MOUSEMOTION) {
			engine.mouse_x = e.motion.x / kiavc_screen_scale;
			engine.mouse_y = e.motion.y / kiavc_screen_scale;
			kiavc_engine_check_hovering();
		} else if(e.type == SDL_MOUSEBUTTONUP) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			x = x/kiavc_screen_scale;
			y = y/kiavc_screen_scale;
			if(engine.room) {
				x += (int)engine.room->res.x;
				y += (int)engine.room->res.y;
			}
			if(engine.dialog) {
				if(engine.dialog->active && engine.dialog->selected) {
					char *id = SDL_strdup(engine.dialog->id);
					char *name = SDL_strdup(engine.dialog->selected->name);
					/* Get rid of the dialog lines, waiting for the next round */
					engine.dialog->active = false;
					kiavc_list *temp = engine.dialog->lines;
					kiavc_dialog_line *line = NULL;
					while(temp) {
						line = (kiavc_dialog_line *)temp->data;
						if(line->text)
							engine.render_list = kiavc_list_remove(engine.render_list, line->text);
						if(line->selected)
							engine.render_list = kiavc_list_remove(engine.render_list, line->selected);
						temp = temp->next;
					}
					kiavc_dialog_clear(engine.dialog);
					/* Notify the script about the choice */
					kiavc_scripts_run_command("dialogSelected('%s', '%s')", id, name);
					SDL_free(id);
					SDL_free(name);
				}
			} else if(!engine.cutscene && !engine.input_disabled) {
				if(e.button.button == SDL_BUTTON_LEFT) {
					kiavc_scripts_run_command("leftClick(%d, %d)", x, y);
				} else if(e.button.button == SDL_BUTTON_RIGHT) {
					kiavc_scripts_run_command("rightClick(%d, %d)", x, y);
				}
			}
		} else if(e.type == SDL_TEXTINPUT) {
			if(console_active) {
				if(!(SDL_GetModState() & KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' ||
						e.text.text[0] == 'v' || e.text.text[0] == 'V'))) {
					size_t len = SDL_strlen(console_text);
					if(len < (sizeof(console_text)-1)) {
						SDL_snprintf(console_text+len, sizeof(console_text)-len-1, "%s", e.text.text);
						kiavc_font_text_destroy(console_rendered);
						SDL_Color color = { .r = 128, .g = 128, .b = 128, 0 };
						console_rendered = kiavc_font_render_text(console_font, renderer, console_text, &color, NULL, kiavc_screen_width);
					}
				}
			}
		} else if(e.type == SDL_KEYDOWN) {
			/* We handle text input different depending on whether the
			 * debugging console is active or not */
			if(console_active) {
				if(e.key.keysym.sym == SDLK_ESCAPE) {
					kiavc_engine_hide_console();
					continue;
				} else if(e.key.keysym.sym == SDLK_RETURN) {
					kiavc_scripts_run_command("%s", console_text + 2);
					console_history = kiavc_list_prepend(console_history, SDL_strdup(console_text + 2));
					console_current = NULL;
					*(console_text + 2) = '\0';
				} else if(e.key.keysym.sym == SDLK_UP) {
					if(console_current == NULL)
						console_current = console_history;
					else
						console_current = console_current->next ? console_current->next : console_current;
					SDL_snprintf(console_text, sizeof(console_text)-1, "> %s", console_current ? (char *)console_current->data : "");
				} else if(e.key.keysym.sym == SDLK_DOWN) {
					console_current = console_current ? console_current->prev : NULL;
					SDL_snprintf(console_text, sizeof(console_text)-1, "> %s", console_current ? (char *)console_current->data : "");
				} else if(e.key.keysym.sym == SDLK_BACKSPACE && SDL_strlen(console_text) > 2) {
					console_text[SDL_strlen(console_text)-1] = '\0';
				} else if(e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL) {
					char *clipboard = SDL_GetClipboardText();
					size_t len = SDL_strlen(console_text);
					if(clipboard && SDL_strlen(clipboard) > 0 && len < (sizeof(console_text)-1)) {
						SDL_snprintf(console_text+len, sizeof(console_text)-len-1, "%s", clipboard);
					}
				}
				kiavc_font_text_destroy(console_rendered);
				SDL_Color color = { .r = 128, .g = 128, .b = 128 };
				console_rendered = kiavc_font_render_text(console_font, renderer, console_text, &color, NULL, kiavc_screen_width);
				continue;
			}
			/* If we got here, the console is not active: pass the key to the script */
			const char *key = SDL_GetKeyName(e.key.keysym.sym);
			kiavc_scripts_run_command("userInput('%s')", key);
		}
	}
	/* Done */
	return 0;
}

/* Update the "world" */
int kiavc_engine_update_world(void) {
	if(quit)
		return -1;
	/* Update the world in the script first */
	uint32_t ticks = SDL_GetTicks();
	if(kiavc_scripts_update_world(ticks) < 0)
		return -1;
	/* Now update the world in the engine: initialize ticks, if needed */
	if(engine.room_ticks == 0)
		engine.room_ticks = ticks;
	/* Check if we're fading in/out the whole screen */
	if((engine.fade_in > 0 || engine.fade_out > 0) && engine.fade_ticks == 0) {
		engine.fade_ticks = ticks;
		engine.fade_alpha = 255;
		kiavc_engine_regenerate_fade();
	}
	/* Check if we're fading in/out individual resources */
	kiavc_resource *resource = NULL;
	kiavc_list *fading = engine.fading;
	while(fading) {
		resource = (kiavc_resource *)fading->data;
		if(resource->fade_ms > 0 && resource->fade_ticks == 0)
			resource->fade_ticks = ticks;
		fading = fading->next;
	}
	/* Loop on all resources to render, which are already ordered by z-plane */
	kiavc_list *item = engine.render_list, *to_remove = NULL;
	bool sort = false;
	while(item) {
		resource = (kiavc_resource *)item->data;
		if(resource->ticks == 0)
			resource->ticks = ticks;
		if(resource->type == KIAVC_ACTOR) {
			/* This is an actor */
			kiavc_actor *actor = (kiavc_actor *)resource;
			if(actor->line && actor->line->started == 0)
				actor->line->started = ticks;
			if(actor->res.move_ticks == 0)
				actor->res.move_ticks = ticks;
			if(ticks - actor->res.move_ticks >= (1000/kiavc_screen_fps)) {
				actor->res.move_ticks += (1000/kiavc_screen_fps);
				if(actor->res.target_x != -1 && actor->res.target_y != -1) {
					if(actor->state != KIAVC_ACTOR_WALKING)
						actor->frame = 0;
					actor->state = KIAVC_ACTOR_WALKING;
					if(actor->line) {
						to_remove = kiavc_list_append(to_remove, actor->line);
						actor->line = NULL;
					}
					int diff_x = (int)actor->res.x - actor->res.target_x;
					int diff_y = (int)actor->res.y - actor->res.target_y;
					if(abs(diff_x) > abs(diff_y)) {
						/* Left/right */
						if(diff_x > 0)
							actor->direction = KIAVC_LEFT;
						else if(diff_x < 0)
							actor->direction = KIAVC_RIGHT;
					} else {
						/* Up/down */
						if(diff_y > 0)
							actor->direction = KIAVC_UP;
						else if(diff_y < 0)
							actor->direction = KIAVC_DOWN;
					}
					float speed = (float)actor->res.speed;
					if(actor->walkbox && actor->walkbox->speed != 1.0) {
						speed = (float)(speed) * actor->walkbox->speed;
						if(speed < 1.0)
							speed = 1.0;
					}
					if((int)actor->res.x != actor->res.target_x || (int)actor->res.y != actor->res.target_y) {
						float movement = speed/kiavc_screen_fps;
						float d = sqrt(((actor->res.target_x - actor->res.x)*(actor->res.target_x - actor->res.x)) +
							((actor->res.target_y - actor->res.y)*(actor->res.target_y - actor->res.y)));
						float p = d/movement;
						float mx = actor->res.x + ((float)actor->res.target_x - actor->res.x) / p;
						float my = actor->res.y + ((float)actor->res.target_y - actor->res.y) / p;
						if((actor->res.x > (float)actor->res.target_x && mx < (float)actor->res.target_x)
								|| (actor->res.x < (float)actor->res.target_x && mx > (float)actor->res.target_x))
							mx = (float)actor->res.target_x;
						if((actor->res.y > (float)actor->res.target_y && my < (float)actor->res.target_y)
								|| (actor->res.y < (float)actor->res.target_y && my > (float)actor->res.target_y))
							my = (float)actor->res.target_y;
						if((int)my != (int)actor->res.y)
							sort = true;
						actor->res.x = mx;
						actor->res.y = my;
					}
					if((int)actor->res.x == actor->res.target_x && (int)actor->res.y == actor->res.target_y) {
						/* Arrived, is it over? */
						if(actor->step) {
							/* We have to walk more */
							kiavc_pathfinding_point *p = (kiavc_pathfinding_point *)actor->step->data;
							actor->res.target_x = p->x;
							actor->res.target_y = p->y;
							actor->step = actor->step->next;
						} else {
							/* We're done */
							g_list_free_full(actor->path, (GDestroyNotify)kiavc_pathfinding_point_destroy);
							actor->path = NULL;
							actor->step = NULL;
							actor->state = KIAVC_ACTOR_STILL;
							actor->res.target_x = -1;
							actor->res.target_y = -1;
							/* FIXME */
							kiavc_scripts_run_command("signal('%s')", actor->id);
						}
					}
					/* FIXME Check which walkbox we're in */
					if(engine.room && engine.room->pathfinding && engine.room->pathfinding->walkboxes) {
						int x = (int)actor->res.x;
						int y = (int)actor->res.y;
						kiavc_pathfinding_point point = { .x = x, .y = y };
						kiavc_pathfinding_walkbox *walkbox = kiavc_pathfinding_context_find_walkbox(engine.room->pathfinding, &point);
						if(walkbox != actor->walkbox) {
							if(walkbox) {
								SDL_Log("Actor '%s' now in walkbox (%dx%d -> %dx%d)", actor->id,
									walkbox->p1.x, walkbox->p1.y, walkbox->p2.x, walkbox->p2.y);
								if(walkbox->name) {
									/* Signal script */
									SDL_Log("Actor '%s' triggered walkbox '%s'\n", actor->id, walkbox->name);
									kiavc_scripts_run_command("triggerWalkbox('%s', '%s', '%s')",
										engine.room->id, walkbox->name, actor->id);
								}
							}
							actor->walkbox = walkbox;
						}
					}
				}
				if(actor->room == engine.room && engine.following == actor) {
					int width = kiavc_screen_width;
					int height = kiavc_screen_height;
					if(engine.room_direction_x == 0) {
						/* FIXME */
						int portion = width/3;
						if((int)actor->res.x - (int)engine.room->res.x < portion)
							engine.room_direction_x = -1;
						else if((int)actor->res.x - (int)engine.room->res.x > (width - portion))
							engine.room_direction_x = 1;
					} else {
						/* FIXME */
						int portion = width/2;
						if((int)actor->res.x - (int)engine.room->res.x > (portion-5) && (int)actor->res.x - (int)engine.room->res.x < (portion+5)) {
							engine.room_direction_x = 0;
						}
					}
					if(engine.room_direction_y == 0) {
						/* FIXME */
						int portion = height/2;
						if((int)actor->res.y - (int)engine.room->res.y < portion)
							engine.room_direction_y = -1;
						else if((int)actor->res.y - (int)engine.room->res.y > (height - portion))
							engine.room_direction_y = 1;
					} else {
						/* FIXME */
						int portion = height/2;
						if((int)actor->res.y - (int)engine.room->res.y > (portion-5) && (int)actor->res.y - (int)engine.room->res.y < (portion+5)) {
							engine.room_direction_y = 0;
						}
					}
				}
			}
			kiavc_costume_set *set = kiavc_costume_get_set(actor->costume, kiavc_actor_state_str(actor->state));
			int ms = (set && set->animations[actor->direction]) ? set->animations[actor->direction]->ms : 100;
			if(ticks - actor->res.ticks >= ms) {
				actor->res.ticks += ms;
				actor->frame++;
			}
		} else if(resource->type == KIAVC_OBJECT) {
			/* This is an object */
			kiavc_object *object = (kiavc_object *)resource;
			int ms = object->animation ? object->animation->ms : 100;
			if(ticks - object->res.ticks >= ms) {
				object->res.ticks += ms;
				if(object->animation) {
					object->frame++;
				} else {
					object->frame = 0;
				}
			}
			if(object->res.speed > 0) {
				/* We're moving this object around */
				if(object->res.move_ticks == 0)
					object->res.move_ticks = ticks;
				if(((int)object->res.x != object->res.target_x || (int)object->res.y != object->res.target_y) &&
						ticks - object->res.move_ticks >= (1000/kiavc_screen_fps)) {
					object->res.move_ticks += (1000/kiavc_screen_fps);
					float movement = (float)(object->res.speed)/(kiavc_screen_fps);
					float d = sqrt(((object->res.target_x - object->res.x)*(object->res.target_x - object->res.x)) +
						((object->res.target_y - object->res.y)*(object->res.target_y - object->res.y)));
					float p = d/movement;
					float mx = object->res.x + ((float)object->res.target_x - object->res.x) / p;
					float my = object->res.y + ((float)object->res.target_y - object->res.y) / p;
					if((object->res.x > (float)object->res.target_x && mx < (float)object->res.target_x)
							|| (object->res.x < (float)object->res.target_x && mx > (float)object->res.target_x))
						mx = (float)object->res.target_x;
					if((object->res.y > (float)object->res.target_y && my < (float)object->res.target_y)
							|| (object->res.y < (float)object->res.target_y && my > (float)object->res.target_y))
						my = (float)object->res.target_y;
					if((int)my != (int)object->res.y)
						sort = true;
					object->res.x = mx;
					object->res.y = my;
				}
				if((int)object->res.x == object->res.target_x && (int)object->res.y == object->res.target_y) {
					/* We're done */
					object->res.speed = 0;
					/* Signal the script that the object has finished moving */
					kiavc_scripts_run_command("signal('%s')", object->id);
				}
			}
		} else if(resource->type == KIAVC_FONT_TEXT) {
			/* This is a font text line (not belonging to an actor) */
			kiavc_font_text *line = (kiavc_font_text *)resource;
			if(line->owner_type != KIAVC_CURSOR && line->owner_type != KIAVC_DIALOG) {
				/* Only cursor text and dialog lines remain there until we manually remove it */
				if(line->started == 0)
					line->started = ticks;
			}
			if(line->res.speed > 0) {
				/* We're moving this text around */
				if(line->res.move_ticks == 0)
					line->res.move_ticks = ticks;
				if(((int)line->res.x != line->res.target_x || (int)line->res.y != line->res.target_y) &&
						ticks - line->res.move_ticks >= (1000/kiavc_screen_fps)) {
					line->res.move_ticks += (1000/kiavc_screen_fps);
					float movement = (float)(line->res.speed)/(kiavc_screen_fps);
					float d = sqrt(((line->res.target_x - line->res.x)*(line->res.target_x - line->res.x)) +
						((line->res.target_y - line->res.y)*(line->res.target_y - line->res.y)));
					float p = d/movement;
					float mx = line->res.x + ((float)line->res.target_x - line->res.x) / p;
					float my = line->res.y + ((float)line->res.target_y - line->res.y) / p;
					if((line->res.x > (float)line->res.target_x && mx < (float)line->res.target_x)
							|| (line->res.x < (float)line->res.target_x && mx > (float)line->res.target_x))
						mx = (float)line->res.target_x;
					if((line->res.y > (float)line->res.target_y && my < (float)line->res.target_y)
							|| (line->res.y < (float)line->res.target_y && my > (float)line->res.target_y))
						my = (float)line->res.target_y;
					line->res.x = mx;
					line->res.y = my;
				}
				if((int)line->res.x == line->res.target_x && (int)line->res.y == line->res.target_y) {
					/* We're done */
					line->res.speed = 0;
					/* Signal the script that the text has finished moving */
					kiavc_scripts_run_command("signal('%s')", line->id);
				}
			}
			if(line->started && line->duration && (ticks - line->started >= line->duration)) {
				/* We've displayed this text line long enough */
				to_remove = kiavc_list_append(to_remove, line);
				/* If this is owned by an actor, handle it */
				if(line->owner_type == KIAVC_ACTOR) {
					kiavc_actor *actor = (kiavc_actor *)line->owner;
					actor->state = KIAVC_ACTOR_STILL;
					actor->line = NULL;
					/* FIXME */
					kiavc_scripts_run_command("signal('%s')", actor->id);
				}
			}
		}
		item = item->next;
	}
	while(to_remove) {
		resource = (kiavc_resource *)to_remove->data;
		if(resource->type == KIAVC_FONT_TEXT) {
			/* This is a font text line */
			kiavc_font_text *line = (kiavc_font_text *)resource;
			engine.render_list = kiavc_list_remove(engine.render_list, line);
			/* Destroy the text line */
			if(line->owner_type == KIAVC_ACTOR) {
				kiavc_actor *actor = (kiavc_actor *)line->owner;
				actor->line = NULL;
			} else if(line->owner_type == KIAVC_CURSOR) {
				engine.cursor_text = NULL;
			}
			if(line->owner_type != KIAVC_DIALOG) {
				if(line->id)
					kiavc_map_remove(texts, line->id);
				else
					kiavc_font_text_destroy(line);
			}
		}
		to_remove = kiavc_list_remove(to_remove, resource);
	}
	if(sort)
		engine.render_list = kiavc_list_sort(engine.render_list, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	if(ticks - engine.room_ticks >= 15) {
		engine.room_ticks += 15;
		if(engine.room && engine.room->background) {
			if(engine.room->background->w) {
				engine.room->res.x += engine.room_direction_x;
				if((int)engine.room->res.x > engine.room->background->w - kiavc_screen_width) {
					engine.room->res.x = engine.room->background->w - kiavc_screen_width;
					engine.room_direction_x = 0;
				} else if(engine.room->res.x < 0) {
					engine.room->res.x = 0;
					engine.room_direction_x = 0;
				}
			}
			if(engine.room->background->h) {
				engine.room->res.y += engine.room_direction_y;
				if((int)engine.room->res.y > engine.room->background->h - kiavc_screen_height) {
					engine.room->res.y = engine.room->background->h - kiavc_screen_height;
					engine.room_direction_y = 0;
				} else if(engine.room->res.y < 0) {
					engine.room->res.y = 0;
					engine.room_direction_y = 0;
				}
			}
		}
	}
	if(engine.main_cursor) {
		if(engine.main_cursor->res.ticks == 0)
			engine.main_cursor->res.ticks = ticks;
		int ms = engine.main_cursor->animation ? engine.main_cursor->animation->ms : 100;
		if(ticks - engine.main_cursor->res.ticks >= ms) {
			engine.main_cursor->res.ticks += ms;
			if(engine.main_cursor->animation) {
				engine.main_cursor->frame++;
			} else {
				engine.main_cursor->frame = 0;
			}
		}
	}
	if(engine.hotspot_cursor && engine.hotspot_cursor != engine.main_cursor) {
		if(engine.hotspot_cursor->res.ticks == 0)
			engine.hotspot_cursor->res.ticks = ticks;
		int ms = engine.hotspot_cursor->animation ? engine.hotspot_cursor->animation->ms : 100;
		if(ticks - engine.hotspot_cursor->res.ticks >= ms) {
			engine.hotspot_cursor->res.ticks += ms;
			if(engine.hotspot_cursor->animation) {
				engine.hotspot_cursor->frame++;
			} else {
				engine.hotspot_cursor->frame = 0;
			}
		}
	}
	if(engine.fade_ticks > 0) {
		if(ticks >= (engine.fade_ticks + (engine.fade_in > 0 ? engine.fade_in : engine.fade_out))) {
			/* We're done */
			engine.fade_ticks = 0;
			engine.fade_alpha = engine.fade_in > 0 ? 0 : 255;
			engine.fade_in = 0;
			engine.fade_out = 0;
			if(engine.fade_alpha == 0) {
				SDL_DestroyTexture(engine.fade_texture);
				engine.fade_texture = NULL;
			}
			kiavc_scripts_run_command("signal('fade')");
		} else {
			/* Calculate the alpha to use for the black texture */
			int diff = ticks - engine.fade_ticks;
			float percent = (float)diff/(float)(engine.fade_in > 0 ? engine.fade_in : engine.fade_out);
			float update = 255 * percent;
			engine.fade_alpha = engine.fade_out > 0 ? (int)update : (255 - (int)update);
		}
	}
	/* Check if we're fading in/out individual resources, and update the state in case */
	kiavc_list *faded = NULL;
	fading = engine.fading;
	while(fading) {
		resource = (kiavc_resource *)fading->data;
		if(resource->fade_alpha == resource->fade_target || ticks >= (resource->fade_ticks + resource->fade_ms)) {
			/* We're done */
			resource->fade_ticks = 0;
			resource->fade_ms = 0;
			faded = kiavc_list_append(faded, resource);
			/* Signal the script */
			if(resource->type == KIAVC_OBJECT) {
				kiavc_object *object = (kiavc_object *)resource;
				kiavc_scripts_run_command("signal('fade-%s')", object->id);
			} else if(resource->type == KIAVC_ACTOR) {
				kiavc_actor *actor = (kiavc_actor *)resource;
				kiavc_scripts_run_command("signal('fade-%s')", actor->id);
			} else if(resource->type == KIAVC_FONT_TEXT) {
				kiavc_font_text *line = (kiavc_font_text *)resource;
				if(line->id)
					kiavc_scripts_run_command("signal('fade-%s')", line->id);
			}
		} else {
			/* Calculate the alpha to use for the resource texture */
			int diff = ticks - resource->fade_ticks;
			float percent = (float)diff/(float)(resource->fade_ms);
			float update = (resource->fade_target - resource->fade_start) * percent;
			resource->fade_alpha = resource->fade_start + (int)update;
		}
		fading = fading->next;
	}
	fading = faded;
	while(fading) {
		engine.fading = kiavc_list_remove(engine.fading, fading->data);
		fading = fading->next;
	}
	kiavc_list_destroy(faded);
	/* Done */
	return 0;
}

/* Render the current frames */
int kiavc_engine_render(void) {
	if(quit)
		return -1;
	/* Draw the images on screen */
	SDL_Rect rect = { 0 }, clip = { 0 };
	uint32_t ticks = SDL_GetTicks();
	if(engine.render_ticks == 0)
		engine.render_ticks = ticks;
	bool background_drawn = false;
	if(ticks - engine.render_ticks >= (1000/kiavc_screen_fps)) {
		engine.render_ticks += (1000/kiavc_screen_fps);
		/* The game may need to be scaled, so let's use the texture as the render target */
		SDL_SetRenderTarget(renderer, canvas);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		/* Now we iterate on dynamic resources (rooms, layers, actors, objects, text, etc.) */
		kiavc_resource *resource = NULL;
		kiavc_list *item = engine.render_list;
		while(item) {
			resource = (kiavc_resource *)item->data;
			if(resource->type == KIAVC_ROOM) {
				/* This is the room background */
				kiavc_animation_load(engine.room->background, engine.room, renderer);
				if(engine.room->background) {
					clip.x = (int)engine.room->res.x;
					clip.y = (int)engine.room->res.y;
					clip.w = kiavc_screen_width;
					clip.h = kiavc_screen_height;
					rect.x = 0;
					rect.y = 0;
					rect.w = kiavc_screen_width;
					rect.h = kiavc_screen_height;
					if(engine.room->background->texture)
						SDL_RenderCopy(renderer, engine.room->background->texture, &clip, &rect);
				}
			} else if(resource->type == KIAVC_ROOM_LAYER) {
				/* This is a room layer */
				kiavc_room_layer *layer = (kiavc_room_layer *)resource;
				kiavc_animation_load(layer->background, layer, renderer);
				/* FIXME */
				if(layer && layer->background && engine.room && engine.room->background) {
					/* Since parallax may be involved, check how much
					 * the width of background and this layer differ */
					int room_range_w = engine.room->background->w - kiavc_screen_width;
					int layer_range_w = layer->background->w - kiavc_screen_width;
					float layer_x = 0;
					if(room_range_w > 0 && layer_range_w > 0)
						layer_x = (engine.room->res.x / (float)room_range_w) * (float)layer_range_w;
					int room_range_h = engine.room->background->h - kiavc_screen_height;
					int layer_range_h = layer->background->h - kiavc_screen_height;
					float layer_y = 0;
					if(room_range_h > 0 && layer_range_h > 0)
						layer_y = (engine.room->res.y / (float)room_range_h) * (float)layer_range_h;
					clip.x = (int)layer_x;
					clip.y = (int)layer_y;
					clip.w = kiavc_screen_width;
					clip.h = kiavc_screen_height;
					rect.x = 0;
					rect.y = 0;
					rect.w = kiavc_screen_width;
					rect.h = kiavc_screen_height;
					if(layer->background->texture)
						SDL_RenderCopy(renderer, layer->background->texture, &clip, &rect);
				}
			} else if(resource->type == KIAVC_ACTOR) {
				/* This is an actor */
				kiavc_actor *actor = (kiavc_actor *)resource;
				if(actor && actor->room == engine.room && actor->visible && actor->costume) {
					int room_x = engine.room ? (int)engine.room->res.x : 0;
					int room_y = engine.room ? (int)engine.room->res.y : 0;
					kiavc_costume_set *set = kiavc_costume_get_set(actor->costume, kiavc_actor_state_str(actor->state));
					if(set && set->animations[actor->direction]) {
						kiavc_costume_load_set(set, actor, renderer);
						clip.w = set->animations[actor->direction]->w;
						clip.h = set->animations[actor->direction]->h;
						if(actor->frame < 0 || actor->frame >= set->animations[actor->direction]->frames)
							actor->frame = 0;
						clip.x = actor->frame*(clip.w);
						clip.y = 0;
						int w = set->animations[actor->direction]->w;
						int h = set->animations[actor->direction]->h;
						if(actor->scale != 1.0 || (actor->walkbox && actor->walkbox->scale != 1.0)) {
							float ws = actor->walkbox ? actor->walkbox->scale : 1.0;
							w *= (actor->scale * ws);
							h *= (actor->scale * ws);
						}
						rect.x = (int)actor->res.x - w/2 - room_x;
						rect.y = (int)actor->res.y - h - room_y;
						rect.w = w;
						rect.h = h;
						if(rect.x < kiavc_screen_width && rect.y < kiavc_screen_height &&
								rect.x + rect.w > 0 && rect.y + rect.h > 0) {
							SDL_SetTextureAlphaMod(set->animations[actor->direction]->texture, actor->res.fade_alpha);
							SDL_RenderCopy(renderer, set->animations[actor->direction]->texture, &clip, &rect);
						}
					}
				}
			} else if(resource->type == KIAVC_OBJECT) {
				/* This is an object */
				kiavc_object *object = (kiavc_object *)resource;
				if(object && ((object->ui && !engine.cutscene && !engine.dialog) || object->room == engine.room)) {
					int room_x = !object->ui && engine.room ? (int)engine.room->res.x : 0;
					int room_y = !object->ui && engine.room ? (int)engine.room->res.y : 0;
					kiavc_animation *animation = object->ui ? object->ui_animation : object->animation;
					if(animation) {
						kiavc_animation_load(animation, object, renderer);
						clip.w = animation->w;
						clip.h = animation->h;
						if(object->frame < 0 || object->frame >= animation->frames)
							object->frame = 0;
						clip.x = object->frame*(clip.w);
						clip.y = 0;
						int w = animation->w;
						int h = animation->h;
						if(object->scale != 1.0) {
							w *= object->scale;
							h *= object->scale;
						}
						int object_x = (int)object->res.x + (object->parent ? (int)object->parent->res.x : 0);
						int object_y = (int)object->res.y + (object->parent ? (int)object->parent->res.y : 0);
						if(object->ui) {
							rect.x = object_x - room_x;
							rect.y = object_y - room_y;
						} else {
							rect.x = object_x - w/2 - room_x;
							rect.y = object_y - h - room_y;
						}
						rect.w = w;
						rect.h = h;
						if(rect.x < kiavc_screen_width && rect.y < kiavc_screen_height &&
								rect.x + rect.w > 0 && rect.y + rect.h > 0) {
							SDL_SetTextureAlphaMod(animation->texture, object->res.fade_alpha);
							SDL_RenderCopy(renderer, animation->texture, &clip, &rect);
						}
					}
				}
			} else if(resource->type == KIAVC_FONT_TEXT) {
				/* This is a text line */
				bool draw = false;
				kiavc_font_text *line = (kiavc_font_text *)resource;
				int room_x = (!line->absolute && engine.room) ? (int)engine.room->res.x : 0;
				int room_y = (!line->absolute && engine.room) ? (int)engine.room->res.y : 0;
				rect.w = line->w;
				rect.h = line->h;
				if(line->owner_type == 0) {
					/* Unowned text */
					draw = true;
					rect.x = (int)line->res.x - line->w/2 - room_x;
					rect.y = (int)line->res.y - line->h/2 - room_y;
				} else {
					/* Check who the owner is */
					if(line->owner_type == KIAVC_ACTOR) {
						kiavc_actor *actor = (kiavc_actor *)line->owner;
						if(actor && actor->room != engine.room) {
							/* FIXME Since the actor isn't in the room, we render the text at the center */
							draw = true;
							line->absolute = true;
							line->res.x = kiavc_screen_width/2;
							line->res.y = kiavc_screen_height/2;
							rect.x = (int)line->res.x - line->w/2 - room_x;
							rect.y = (int)line->res.y - line->h/2 - room_y;
						} else if(actor && actor->state == KIAVC_ACTOR_TALKING) {
							kiavc_costume_set *set = kiavc_costume_get_set(actor->costume, "talking");
							draw = true;
							/* Don't draw the text if the actor isn't visible */
							int w = (set && set->animations[actor->direction]) ?
								set->animations[actor->direction]->w : 0;
							int h = (set && set->animations[actor->direction]) ?
								set->animations[actor->direction]->h : 0;
							int ax = (int)actor->res.x - w/2 - room_x;
							int ay = (int)actor->res.y - h - room_y;
							int aw = w;
							int ah = h;
							if(w == 0 || h == 0 || ax >= kiavc_screen_width || ay >= kiavc_screen_height ||
									ax + aw <= 0 || ay + ah <= 0)
								draw = false;
							rect.x = (int)actor->res.x - room_x - (line->w/2);
							if(rect.x < 0)
								rect.x = 0;
							else if(rect.x + line->w > kiavc_screen_width)
								rect.x = kiavc_screen_width - line->w;
							int diff_y = kiavc_screen_height/20;
							if(set && set->animations[actor->direction]) {
								int h = set->animations[actor->direction]->h;
								if(actor->scale != 1.0 || (actor->walkbox && actor->walkbox->scale != 1.0)) {
									float ws = actor->walkbox ? actor->walkbox->scale : 1.0;
									h *= (actor->scale * ws);
								}
								rect.y = (int)actor->res.y - h - room_y - line->h - diff_y;
							} else {
								rect.y = (int)actor->res.y - room_y - line->h - diff_y;
							}
							if(rect.y < 0)
								rect.y = 0;
						}
					} else if(line->owner_type == KIAVC_CURSOR) {
						kiavc_cursor *cursor = (engine.hovering && engine.hotspot_cursor && engine.hotspot_cursor->animation) ?
							engine.hotspot_cursor : engine.main_cursor;
						if(cursor && cursor->animation) {
							draw = true;
							rect.x = engine.mouse_x - line->w/2;
							rect.y = engine.mouse_y - line->h - cursor->animation->h/2;
							if(rect.y < 0)
								rect.y = engine.mouse_y;
						}
					} else if(line->owner_type == KIAVC_DIALOG) {
						if(engine.dialog && engine.dialog == line->owner) {
							/* FIXME We draw ourselves in a viewport */
							clip.x = engine.dialog->area.x;
							clip.y = engine.dialog->area.y - 4;
							clip.w = engine.dialog->area.w;
							clip.h = engine.dialog->area.h + 4;
							SDL_RenderSetViewport(renderer, &clip);
							/* If we haven't drawn the background yet, do it now */
							if(!background_drawn) {
								background_drawn = true;
								SDL_SetRenderDrawColor(renderer, engine.dialog->background.r,
									engine.dialog->background.g, engine.dialog->background.b, engine.dialog->background.a);
								SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
								SDL_RenderFillRect(renderer, NULL);
								SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
							}
							/* Draw the text */
							rect.x = (int)line->res.x;
							if(rect.x < 0)
								rect.x = 0;
							rect.y = (int)line->res.y;
							if(rect.y < 0)
								rect.y = 0;
							SDL_RenderCopy(renderer, line->texture, NULL, &rect);
							SDL_RenderSetViewport(renderer, NULL);
						}
					}
				}
				if(draw && rect.x < kiavc_screen_width && rect.y < kiavc_screen_height &&
						rect.x + rect.w > 0 && rect.y + rect.h > 0) {
					SDL_SetTextureAlphaMod(line->texture, line->res.fade_alpha);
					SDL_RenderCopy(renderer, line->texture, NULL, &rect);
				}
			}
			item = item->next;
		}
		/* If we haven't drawn the dialog background yet, do it now */
		if(engine.dialog && !background_drawn && (engine.dialog->lines || !engine.dialog->autohide)) {
			background_drawn = true;
			/* We draw in a viewport */
			clip.x = engine.dialog->area.x;
			clip.y = engine.dialog->area.y - 4;
			clip.w = engine.dialog->area.w * kiavc_screen_scale;
			clip.h = engine.dialog->area.h + 4;
			SDL_RenderSetViewport(renderer, &clip);
			SDL_SetRenderDrawColor(renderer, engine.dialog->background.r,
				engine.dialog->background.g, engine.dialog->background.b, engine.dialog->background.a);
			SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
			SDL_RenderFillRect(renderer, NULL);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
			SDL_RenderSetViewport(renderer, NULL);
		}
		/* If we're fading in or out, draw the black fade texture with the right alpha */
		if(engine.fade_texture && engine.fade_alpha > 0) {
			SDL_SetTextureAlphaMod(engine.fade_texture, engine.fade_alpha);
			SDL_RenderCopy(renderer, engine.fade_texture, NULL, NULL);
		}
		/* The cursor is always the last game thing we draw */
		kiavc_cursor *cursor = (engine.hovering && engine.hotspot_cursor && engine.hotspot_cursor->animation) ?
			engine.hotspot_cursor : engine.main_cursor;
		if(engine.cursor_visible && cursor && cursor->animation && cursor->animation && (!engine.cutscene || engine.dialog)) {
			kiavc_animation_load(cursor->animation, cursor, renderer);
			if(cursor->frame < 0 || cursor->frame >= cursor->animation->frames)
				cursor->frame = 0;
			clip.w = cursor->animation->w;
			clip.h = cursor->animation->h;
			clip.x = cursor->frame*(clip.w);
			clip.y = 0;
			rect.x = (int)cursor->res.x;
			rect.y = (int)cursor->res.y;
			rect.w = cursor->animation->w;
			rect.h = cursor->animation->h;
			if(cursor->animation->texture)
				SDL_RenderCopy(renderer, cursor->animation->texture, &clip, &rect);
		}
		/* Now that we're done with game stuff, let's pass the back texture to the renderer:
		 * this will allow us to render debugging stuff and filters at the actual resolution */
		SDL_SetRenderTarget(renderer, NULL);
		SDL_RenderCopy(renderer, canvas, NULL, NULL);
		/* Check if we're debugging objects */
		if(kiavc_debug_objects && engine.render_list) {
			SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
			kiavc_resource *resource = NULL;
			kiavc_object *object = NULL;
			int x = 0, y = 0, w = 0, h = 0,
				x1 = 0, y1 = 0, x2 = 0, y2 = 0;
			kiavc_list *temp = engine.render_list;
			while(temp) {
				resource = (kiavc_resource *)temp->data;
				if(resource->type != KIAVC_OBJECT) {
					temp = temp->next;
					continue;
				}
				object = (kiavc_object *)resource;
				if(object->ui)
					SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
				else
					SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
				x = y = w = h = 0;
				int object_x = (int)object->res.x + (object->parent ? (int)object->parent->res.x : 0);
				int object_y = (int)object->res.y + (object->parent ? (int)object->parent->res.y : 0);
				if(object->hover.from_x >= 0 || object->hover.from_y >= 0 ||
						object->hover.to_x >= 0 || object->hover.to_y >= 0) {
					x = object->hover.from_x;
					y = object->hover.from_y;
					w = object->hover.to_x - x;
					h = object->hover.to_y - y;
				} else if(!object->ui && object->animation) {
					x = object_x - object->animation->w/2;
					y = object_y - object->animation->h;
					w = object->animation->w;
					h = object->animation->h;
				} else if(object->ui && object->ui_animation) {
					x = object_x;
					y = object_y;
					w = object->ui_animation->w;
					h = object->ui_animation->h;
				}
				x1 = (x - (object->ui ? 0 : (int)engine.room->res.x)) * kiavc_screen_scale;
				y1 = (y - (object->ui ? 0 : (int)engine.room->res.y)) * kiavc_screen_scale;
				x2 = (x + w - (object->ui ? 0 : (int)engine.room->res.x)) * kiavc_screen_scale;
				y2 = (y + h - (object->ui ? 0 : (int)engine.room->res.y)) * kiavc_screen_scale;
				SDL_RenderDrawLine(renderer, x1, y1, x2, y1);
				SDL_RenderDrawLine(renderer, x2, y1, x2, y2);
				SDL_RenderDrawLine(renderer, x2, y2, x1, y2);
				SDL_RenderDrawLine(renderer, x1, y2, x1, y1);
				temp = temp->next;
			}
		}
		/* Check if we're debugging walkboxes */
		if(kiavc_debug_walkboxes && engine.room && engine.room->pathfinding && engine.room->pathfinding->walkboxes) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
			kiavc_pathfinding_walkbox *w = NULL;
			int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
			kiavc_list *temp = engine.room->pathfinding->walkboxes;
			while(temp) {
				w = (kiavc_pathfinding_walkbox *)temp->data;
				if(w->disabled) {
					temp = temp->next;
					continue;
				}
				x1 = (w->p1.x - (int)engine.room->res.x) * kiavc_screen_scale;
				y1 = (w->p1.y - (int)engine.room->res.y) * kiavc_screen_scale;
				x2 = (w->p2.x - (int)engine.room->res.x) * kiavc_screen_scale;
				y2 = (w->p2.y - (int)engine.room->res.y) * kiavc_screen_scale;
				SDL_RenderDrawLine(renderer, x1, y1, x2, y1);
				SDL_RenderDrawLine(renderer, x2, y1, x2, y2);
				SDL_RenderDrawLine(renderer, x2, y2, x1, y2);
				SDL_RenderDrawLine(renderer, x1, y2, x1, y1);
				temp = temp->next;
			}
			if(engine.actor && engine.actor->path) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
				kiavc_pathfinding_point *p1 = NULL, *p2 = NULL;
				temp = engine.actor->path;
				while(temp) {
					p1 = (kiavc_pathfinding_point *)temp->data;
					p2 = (kiavc_pathfinding_point *)(temp->next ? temp->next->data : NULL);
					if(p2) {
						x1 = (p1->x - (int)engine.room->res.x) * kiavc_screen_scale;
						y1 = (p1->y - (int)engine.room->res.y) * kiavc_screen_scale;
						x2 = (p2->x - (int)engine.room->res.x) * kiavc_screen_scale;
						y2 = (p2->y - (int)engine.room->res.y) * kiavc_screen_scale;
						SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
					}
					temp = temp->next;
				}
			}
		}
		/* If the console's active, draw that now */
		if(console_active && console_rendered) {
			rect.x = 0;
			rect.y = (kiavc_screen_height * kiavc_screen_scale) - console_rendered->h;
			rect.w = console_rendered->w;
			rect.h = console_rendered->h;
			SDL_RenderCopy(renderer, console_rendered->texture, NULL, &rect);
		}
		/* If the scanlines filter is active, display that too */
		if(kiavc_screen_scanlines_texture)
			SDL_RenderCopy(renderer, kiavc_screen_scanlines_texture, NULL, NULL);
		/* Done, render to the screen */
		SDL_RenderPresent(renderer);
	}
	SDL_Delay(10);
	/* Done */
	return 0;
}

/* Destroy the engine */
void kiavc_engine_destroy(void) {
	/* Destroy all resources */
	kiavc_scripts_unload();
	kiavc_map_destroy(cursors);
	kiavc_map_destroy(rooms);
	kiavc_map_destroy(actors);
	kiavc_map_destroy(costumes);
	kiavc_map_destroy(objects);
	kiavc_map_destroy(dialogs);
	kiavc_map_destroy(texts);
	kiavc_map_destroy(fonts);
	kiavc_map_destroy(audios);
	kiavc_map_destroy(animations);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(canvas);
	if(kiavc_screen_title)
		SDL_free(kiavc_screen_title);
	if(kiavc_screen_icon)
		SDL_free(kiavc_screen_icon);
	if(kiavc_screen_scanlines_texture)
		SDL_DestroyTexture(kiavc_screen_scanlines_texture);
	SDL_free(app_path);
}

/* Scripting callbacks */
static void kiavc_engine_set_resolution(int width, int height, int fps, int scale) {
	if(width < 1 || height < 1 || fps < 1 || scale < 1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid resolution: %dx%d @ %d (scale: %d)\n", width, height, fps, scale);
		return;
	}
	bool changed = (kiavc_screen_width != width || kiavc_screen_height != height || kiavc_screen_scale != scale);
	kiavc_screen_width = width;
	kiavc_screen_height = height;
	kiavc_screen_fps = fps;
	kiavc_screen_scale = scale;
	SDL_Log("Updated resolution: %dx%d @ %d (scale: %d --> %d/%d)\n",
		kiavc_screen_width, kiavc_screen_width, kiavc_screen_fps, kiavc_screen_scale,
		kiavc_screen_width * kiavc_screen_scale, kiavc_screen_height * kiavc_screen_scale);
	if(window) {
		if(changed) {
			kiavc_engine_regenerate_scanlines();
			kiavc_engine_regenerate_fade();
		}
		SDL_SetWindowSize(window, kiavc_screen_width * kiavc_screen_scale, kiavc_screen_height * kiavc_screen_scale);
	}
}
static void kiavc_engine_set_title(const char *title) {
	if(!title)
		return;
	SDL_free(kiavc_screen_title);
	kiavc_screen_title = SDL_strdup(title);
	if(window)
		SDL_SetWindowTitle(window, kiavc_screen_title);
}
static void kiavc_engine_set_icon(const char *path) {
	if(!path)
		return;	/* Been here already */
	SDL_free(kiavc_screen_icon);
	kiavc_screen_icon = SDL_strdup(path);
	if(window) {
		/* Load the provided icon */
		SDL_RWops *icon_rw = kiavc_engine_open_file(kiavc_screen_icon);
		SDL_Surface *icon = IMG_Load_RW(icon_rw, 1);
		if(!icon) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load icon '%s'\n", path);
			return;
		}
		SDL_SetWindowIcon(window, icon);
		SDL_FreeSurface(icon);
	}
}
static void kiavc_engine_grab_mouse(bool grab) {
	if(kiavc_screen_grab_mouse == grab) {
		/* Nothing to do */
		return;
	}
	kiavc_screen_grab_mouse = grab;
	if(window)
		SDL_SetWindowMouseGrab(window, kiavc_screen_grab_mouse);
	SDL_Log("%s mouse grabbing\n", kiavc_screen_grab_mouse ? "Enabling" : "Disabling");
}
static bool kiavc_engine_is_grabbing_mouse(void) {
	return kiavc_screen_grab_mouse;
}
static void kiavc_engine_set_fullscreen(bool fullscreen, bool desktop) {
	if(kiavc_screen_fullscreen == fullscreen) {
		/* Nothing to do */
		return;
	}
	kiavc_screen_fullscreen = fullscreen;
	kiavc_screen_fullscreen_desktop = desktop;
	kiavc_engine_trigger_fullscreen();
	SDL_Log("%s full-screen\n", kiavc_screen_fullscreen ? "Enabling" : "Disabling");
}
static bool kiavc_engine_get_fullscreen(void) {
	return kiavc_screen_fullscreen;
}
static void kiavc_engine_set_scanlines(bool scanlines) {
	if(kiavc_screen_scanlines == scanlines) {
		/* Nothing to do */
		return;
	}
	kiavc_screen_scanlines = scanlines;
	kiavc_engine_regenerate_scanlines();
	SDL_Log("%s scanlines\n", kiavc_screen_scanlines ? "Enabling" : "Disabling");
}
static bool kiavc_engine_get_scanlines(void) {
	return kiavc_screen_scanlines;
}
static void kiavc_engine_debug_objects(bool debug) {
	if(kiavc_debug_objects == debug) {
		/* Nothing to do */
		return;
	}
	kiavc_debug_objects = debug;
	SDL_Log("%s objects debugging\n", kiavc_debug_objects ? "Enabling" : "Disabling");
}
static bool kiavc_engine_is_debugging_objects(void) {
	return kiavc_debug_objects;
}
static void kiavc_engine_debug_walkboxes(bool debug) {
	if(kiavc_debug_walkboxes == debug) {
		/* Nothing to do */
		return;
	}
	kiavc_debug_walkboxes = debug;
	SDL_Log("%s walkboxes debugging\n", kiavc_debug_walkboxes ? "Enabling" : "Disabling");
}
static bool kiavc_engine_is_debugging_walkboxes(void) {
	return kiavc_debug_walkboxes;
}
static void kiavc_engine_save_screenshot(const char *filename) {
	if(!filename)
		return;
	SDL_Surface *screenshot = kiavc_create_surface(
		kiavc_screen_width * kiavc_screen_scale,
		kiavc_screen_height * kiavc_screen_scale);
	if(!screenshot)
		return;
	if(SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, screenshot->pixels, screenshot->pitch) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error reading rendered pixels: %s\n", SDL_GetError());
		SDL_FreeSurface(screenshot);
		return;
	}
	char fullpath[1024];
	g_snprintf(fullpath, sizeof(fullpath)-1, "%s%s", app_path, filename);
	if(IMG_SavePNG(screenshot, fullpath) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error saving screenshot: %s\n", IMG_GetError());
	} else {
		SDL_Log("Saved screenshot to '%s'\n", fullpath);
	}
	SDL_FreeSurface(screenshot);
}
static void kiavc_engine_enable_console(const char *font) {
	if(!font)
		return;
	kiavc_font *f = kiavc_map_lookup(fonts, font);
	if(!f) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't enable console, no such font '%s'\n", font);
		return;
	}
	console_font = f;
	SDL_Log("Enabled console with font '%s'\n", font);
}
static void kiavc_engine_show_console(void) {
	if(!console_active) {
		console_active = true;
		SDL_StartTextInput();
		SDL_snprintf(console_text, sizeof(console_text)-1, "%s", "> -- Console active");
		SDL_Color color = { .r = 128, .g = 128, .b = 128, 0 };
		console_rendered = kiavc_font_render_text(console_font, renderer, console_text, &color, NULL, kiavc_screen_width);
		SDL_Log("Showing console\n");
	}
}
static void kiavc_engine_hide_console(void) {
	if(console_active) {
		SDL_StopTextInput();
		console_active = FALSE;
		console_text[0] = '\0';
		kiavc_font_text_destroy(console_rendered);
		console_rendered = NULL;
		SDL_Log("Hidden console\n");
	}
}
static void kiavc_engine_disable_console(void) {
	SDL_StopTextInput();
	console_active = FALSE;
	console_text[0] = '\0';
	kiavc_font_text_destroy(console_rendered);
	console_rendered = NULL;
	console_font = NULL;
	SDL_Log("Disabled console\n");
}
static bool kiavc_engine_is_console_enabled(void) {
	return !!console_font;
}
static bool kiavc_engine_is_console_visible(void) {
	return !!console_font && console_active;
}
static void kiavc_engine_enable_input(void) {
	if(engine.input_disabled) {
		SDL_Log("Enabling user input\n");
		engine.input_disabled = false;
		kiavc_engine_check_hovering();
	}
}
static void kiavc_engine_disable_input(void) {
	if(!engine.input_disabled) {
		SDL_Log("Disabling user input\n");
		engine.input_disabled = true;
		kiavc_engine_hide_cursor_text();
		kiavc_engine_check_hovering();
	}
}
static bool kiavc_engine_is_input_enabled(void) {
	return !engine.input_disabled;
}
static void kiavc_engine_start_cutscene(void) {
	if(!engine.cutscene) {
		SDL_Log("Starting cutscene\n");
		engine.cutscene = true;
		kiavc_engine_hide_cursor_text();
		kiavc_engine_check_hovering();
	}
}
static void kiavc_engine_stop_cutscene(void) {
	if(engine.cutscene) {
		SDL_Log("Stopping cutscene\n");
		engine.cutscene = false;
		kiavc_engine_check_hovering();
	}
}
static void kiavc_engine_fade_in(int ms) {
	if(ms < 1)
		return;
	if(engine.fade_in || engine.fade_out) {
		/* Already fading */
		return;
	}
	engine.fade_in = ms;
	engine.fade_ticks = 0;
	SDL_Log("Fading in (%d ms)", ms);
}
static void kiavc_engine_fade_out(int ms) {
	if(ms < 1)
		return;
	if(engine.fade_in || engine.fade_out) {
		/* Already fading */
		return;
	}
	engine.fade_out = ms;
	engine.fade_ticks = 0;
	SDL_Log("Fading out (%d ms)", ms);
}
static void kiavc_engine_start_dialog(const char *id, const char *fid, SDL_Color *color, SDL_Color *outline,
		SDL_Color *s_color, SDL_Color *s_outline, SDL_Color *background, SDL_Rect *area, bool autohide) {
	if(!id || !fid || !color || !s_color || !area || !background)
		return;
	/* Get the dialog */
	kiavc_dialog *dialog = kiavc_map_lookup(dialogs, id);
	if(!dialog) {
		/* No such dialog, create one now */
		dialog = kiavc_dialog_create(id);
		kiavc_map_insert(dialogs, SDL_strdup(id), dialog);
	}
	if(dialog->active) {
		/* Dialog already active */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't start dialog, dialog '%s' is already active\n", id);
		return;
	}
	kiavc_font *font = kiavc_map_lookup(fonts, fid);
	if(!font) {
		/* No font */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't start dialog, no such font '%s'\n", fid);
		return;
	}
	dialog->active = true;
	dialog->renderer = renderer;
	dialog->font = font;
	dialog->background = *background;
	dialog->color = *color;
	if(outline) {
		dialog->border = true;
		dialog->outline = *outline;
	}
	dialog->s_color = *s_color;
	if(s_outline) {
		dialog->s_border = true;
		dialog->s_outline = *s_outline;
	}
	dialog->max_width = kiavc_screen_width;
	dialog->area = *area;
	dialog->autohide = autohide;
	kiavc_engine_check_hovering();
	/* Done */
	engine.dialog = dialog;
	SDL_Log("Started dialog '%s'\n", id);
}
static void kiavc_engine_add_dialog_line(const char *id, const char *name, const char *text) {
	if(!id || !name || !text)
		return;
	/* Get the dialog */
	kiavc_dialog *dialog = kiavc_map_lookup(dialogs, id);
	if(!dialog) {
		/* No dialog session */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add dialog line, no such dialog '%s'\n", id);
		return;
	}
	if(!dialog->active || engine.dialog != dialog) {
		/* Not an active dialog session */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add dialog line, dialog '%s' is not running\n", id);
		return;
	}
	kiavc_dialog_line *line = kiavc_dialog_add_line(dialog, name, text);
	if(line == NULL) {
		/* Something went wrong */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error adding dialog line to dialog '%s'\n", id);
		return;
	}
	bool selected = (line == dialog->selected);
	engine.render_list = kiavc_list_append(engine.render_list, selected ? line->selected : line->text);
	kiavc_engine_check_hovering();
	/* Done */
	SDL_Log("Added dialog line to '%s' (%s)\n", id, name);
}
static void kiavc_engine_stop_dialog(const char *id) {
	if(!id)
		return;
	/* Get the dialog */
	kiavc_dialog *dialog = kiavc_map_lookup(dialogs, id);
	if(!dialog) {
		/* No dialog session */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't stop dialog, no such dialog '%s'\n", id);
		return;
	}
	dialog->active = false;
	/* Get rid of the dialog lines */
	kiavc_list *temp = dialog->lines;
	kiavc_dialog_line *line = NULL;
	while(temp) {
		line = (kiavc_dialog_line *)temp->data;
		if(line->text)
			engine.render_list = kiavc_list_remove(engine.render_list, line->text);
		if(line->selected)
			engine.render_list = kiavc_list_remove(engine.render_list, line->selected);
		temp = temp->next;
	}
	kiavc_dialog_clear(dialog);
	/* Done */
	engine.dialog = NULL;
	SDL_Log("Stopped dialog '%s'\n", id);
}
static void kiavc_engine_register_animation(const char *id, const char *path, int frames, int ms, SDL_Color *transparency) {
	if(!id || !path || frames < 1)
		return;
	/* Check if this animation ID exists already */
	kiavc_animation *anim = kiavc_map_lookup(animations, id);
	if(anim) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new animation with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new animation instance and add it to the map */
	anim = kiavc_animation_create(id, path, frames, ms, transparency);
	kiavc_map_insert(animations, anim->id, anim);
	/* Done */
	SDL_Log("Registered %d-frames animation '%s' (%s)\n", frames, anim->id, anim->path);
}
static void kiavc_engine_register_font(const char *id, const char *path, int size) {
	if(!id || !path || size < 1)
		return;
	/* Check if this font ID exists already */
	kiavc_font *font = kiavc_map_lookup(fonts, id);
	if(font) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new font with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new font instance and add it to the map */
	font = kiavc_font_create(id, path, size);
	kiavc_map_insert(fonts, font->id, font);
	/* Done */
	SDL_Log("Registered font '%s'\n", font->id);
}
static void kiavc_engine_register_cursor(const char *id) {
	if(!id)
		return;
	/* Check if this cursor ID exists already */
	kiavc_cursor *cursor = kiavc_map_lookup(cursors, id);
	if(cursor) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new cursor with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new cursor instance and add it to the map */
	cursor = kiavc_cursor_create(id);
	kiavc_map_insert(cursors, cursor->id, cursor);
	/* Done */
	SDL_Log("Registered cursor '%s'\n", cursor->id);
}
static void kiavc_engine_set_cursor_animation(const char *id, const char *canim) {
	if(!id || !canim)
		return;
	/* Access cursor and animation from the respective maps */
	kiavc_cursor *cursor = kiavc_map_lookup(cursors, id);
	if(!cursor) {
		/* No such cursor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set cursor animation, no such cursor '%s'\n", id);
		return;
	}
	kiavc_animation *anim = kiavc_map_lookup(animations, canim);
	if(!anim) {
		/* No such animation */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set animation for cursor '%s', no such animation '%s'\n", id, canim);
		return;
	}
	/* Done */
	cursor->animation = anim;
	SDL_Log("Set animation of cursor '%s' to '%s'\n", cursor->id, anim->id);
}
static void kiavc_engine_set_main_cursor(const char *id) {
	if(!id)
		return;
	/* Get the cursor */
	kiavc_cursor *cursor = kiavc_map_lookup(cursors, id);
	if(!cursor) {
		/* No such cursor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set main cursor, no such cursor '%s'\n", id);
		return;
	}
	/* If we're showing a different cursor, hide that one first */
	if(cursor->animation) {
		cursor->res.x = engine.mouse_x - (cursor->animation->w/2);
		cursor->res.y = engine.mouse_y - (cursor->animation->h/2);
	}
	if(engine.main_cursor)
		engine.main_cursor->res.ticks = 0;
	engine.main_cursor = cursor;
	engine.main_cursor->res.ticks = 0;
	/* Done */
	SDL_Log("Set main cursor '%s'\n", cursor->id);
}
static void kiavc_engine_set_hotspot_cursor(const char *id) {
	if(!id)
		return;
	/* Get the cursor */
	kiavc_cursor *cursor = kiavc_map_lookup(cursors, id);
	if(!cursor) {
		/* No such cursor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set hotspot cursor, no such cursor '%s'\n", id);
		return;
	}
	/* If we're showing a different cursor, hide that one first */
	if(cursor->animation) {
		cursor->res.x = engine.mouse_x - (cursor->animation->w/2);
		cursor->res.y = engine.mouse_y - (cursor->animation->h/2);
	}
	if(engine.hotspot_cursor)
		engine.hotspot_cursor->res.ticks = 0;
	engine.hotspot_cursor = cursor;
	engine.hotspot_cursor->res.ticks = 0;
	/* Done */
	SDL_Log("Set hotspot cursor '%s'\n", cursor->id);
}
static void kiavc_engine_show_cursor(void) {
	engine.cursor_visible = true;
	/* Done */
	SDL_Log("Shown cursor\n");
}
static void kiavc_engine_hide_cursor(void) {
	engine.cursor_visible = false;
	/* Done */
	SDL_Log("Hidden cursor\n");
}
static void kiavc_engine_show_cursor_text(const char *fid, const char *text, SDL_Color *color, SDL_Color *outline) {
	if(!fid || !text || !color)
		return;
	kiavc_font *font = kiavc_map_lookup(fonts, fid);
	if(!font) {
		/* No font */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't show cursor text, no such font '%s'\n", fid);
		return;
	}
	/* Create a line of text */
	if(engine.cursor_text) {
		engine.render_list = kiavc_list_remove(engine.render_list, engine.cursor_text);
		kiavc_font_text_destroy(engine.cursor_text);
	}
	engine.cursor_text = kiavc_font_render_text(font, renderer, text, color, outline, kiavc_screen_width);
	if(engine.cursor_text == NULL)
		return;
	engine.cursor_text->owner_type = KIAVC_CURSOR;
	engine.render_list = kiavc_list_insert_sorted(engine.render_list, engine.cursor_text, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	/* Done */
	SDL_Log("Added cursor text\n");
}
static void kiavc_engine_hide_cursor_text(void) {
	if(engine.cursor_text) {
		engine.render_list = kiavc_list_remove(engine.render_list, engine.cursor_text);
		kiavc_font_text_destroy(engine.cursor_text);
	}
	engine.cursor_text = NULL;
	/* Done */
	SDL_Log("Hidden cursor text\n");
}
static void kiavc_engine_register_audio(const char *id, const char *path) {
	if(!id || !path)
		return;
	/* Check if this audio track ID exists already */
	kiavc_audio *audio = kiavc_map_lookup(audios, id);
	if(audio) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new audio track with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new audio instance and add it to the map */
	audio = kiavc_audio_create(id, path);
	kiavc_map_insert(audios, audio->id, audio);
	/* Done */
	SDL_Log("Registered audio track '%s'\n", audio->id);
}
static void kiavc_engine_play_audio(const char *id, int fade_ms, bool loop) {
	if(!id || fade_ms < 0)
		return;
	/* Access audio track from the map */
	kiavc_audio *audio = kiavc_map_lookup(audios, id);
	if(!audio) {
		/* No such audio track */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't play audio, no such audio track '%s'\n", id);
		return;
	}
	kiavc_audio_play(audio, fade_ms, loop);
	/* Done */
	SDL_Log("%s audio track '%s'\n", (fade_ms ? "Fading in" : "Playing"), audio->id);
}
static void kiavc_engine_pause_audio(const char *id) {
	if(!id)
		return;
	/* Access audio track from the map */
	kiavc_audio *audio = kiavc_map_lookup(audios, id);
	if(!audio) {
		/* No such audio track */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't pause audio, no such audio track '%s'\n", id);
		return;
	}
	kiavc_audio_pause(audio);
	/* Done */
	SDL_Log("Paused audio track '%s'\n", audio->id);
}
static void kiavc_engine_resume_audio(const char *id) {
	if(!id)
		return;
	/* Access audio track from the map */
	kiavc_audio *audio = kiavc_map_lookup(audios, id);
	if(!audio) {
		/* No such audio track */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't resume audio, no such audio track '%s'\n", id);
		return;
	}
	kiavc_audio_resume(audio);
	/* Done */
	SDL_Log("Resumed audio track '%s'\n", audio->id);
}
static void kiavc_engine_stop_audio(const char *id, int fade_ms) {
	if(!id || fade_ms < 0)
		return;
	/* Access audio track from the map */
	kiavc_audio *audio = kiavc_map_lookup(audios, id);
	if(!audio) {
		/* No such audio track */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't stop audio, no such audio track '%s'\n", id);
		return;
	}
	kiavc_audio_stop(audio, fade_ms);
	/* Done */
	SDL_Log("%s audio track '%s'\n", (fade_ms ? "Fading out" : "Stopping"), audio->id);
}
static void kiavc_engine_register_room(const char *id) {
	if(!id)
		return;
	/* Check if this room ID exists already */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(room) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new room with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new room instance and add it to the map */
	room = kiavc_room_create(id);
	kiavc_map_insert(rooms, room->id, room);
	/* Done */
	SDL_Log("Registered room '%s'\n", room->id);
}
static void kiavc_engine_set_room_background(const char *id, const char *bg) {
	if(!id || !bg)
		return;
	/* Access room and image from the respective maps */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set room background, no such room '%s'\n", id);
		return;
	}
	kiavc_animation *img = kiavc_map_lookup(animations, bg);
	if(!img) {
		/* No such image */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set room background for room '%s', no such image '%s'\n", id, bg);
		return;
	}
	/* Done */
	if(room->background)
		kiavc_animation_unload(room->background, room);
	room->background = img;
	room->res.x = 0;
	room->res.y = 0;
	SDL_Log("Set background of room '%s' to '%s'\n", room->id, img->id);
}
static void kiavc_engine_add_room_layer(const char *id, const char *name, const char *bg, int zplane) {
	if(!id || !name || !bg)
		return;
	/* Access room from the map */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add room layer, no such room '%s'\n", id);
		return;
	}
	kiavc_animation *img = kiavc_map_lookup(animations, bg);
	if(!img) {
		/* No such image */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add room layer '%s' for room '%s', no such image '%s'\n", name, id, bg);
		return;
	}
	kiavc_room_layer *layer = kiavc_room_add_layer(room, name, zplane);
	if(!layer) {
		/* Error adding layer */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add room layer '%s' for room '%s'\n", name, id);
		return;
	}
	layer->background = img;
	engine.render_list = kiavc_list_insert_sorted(engine.render_list, layer, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	/* Done */
	SDL_Log("Added layer '%s' to room '%s'\n", name, room->id);
}
static void kiavc_engine_remove_room_layer(const char *id, const char *name) {
	if(!id || !name)
		return;
	/* Access room from the map */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add room layer, no such room '%s'\n", id);
		return;
	}
	kiavc_list *list = room->layers;
	while(list) {
		kiavc_room_layer *layer = (kiavc_room_layer *)list->data;
		if(layer->id && !SDL_strcasecmp(layer->id, id)) {
			/* Found */
			kiavc_animation_unload(layer->background, layer);
			engine.render_list = kiavc_list_remove(engine.render_list, layer);
			break;
		}
	}
	if(kiavc_room_remove_layer(room, name) < 0) {
		/* Error adding layer */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove room layer '%s' from room '%s'\n", name, id);
		return;
	}
	/* Done */
	SDL_Log("Removed layer '%s' from room '%s'\n", name, room->id);
}
static void kiavc_engine_add_room_walkbox(const char *id, const char *name, int x1, int y1, int x2, int y2, float scale, float speed, bool disabled) {
	if(!id)
		return;
	/* Access room from the map */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add room walkbox, no such room '%s'\n", id);
		return;
	}
	/* Create a new walkbox instance */
	kiavc_pathfinding_walkbox *walkbox = kiavc_pathfinding_walkbox_create(name, x1, y1, x2, y2, scale, speed, disabled);
	if(kiavc_room_add_walkbox(room, walkbox) < 0) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't add walkbox to room '%s'\n", id);
		return;
	}
	/* Done */
	SDL_Log("Added '%s' walkbox to room '%s'\n", name ? name : "unnamed", room->id);
}
static void kiavc_engine_enable_room_walkbox(const char *id, const char *name) {
	if(!id || !name)
		return;
	/* Access room from the map */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't enable room walkbox, no such room '%s'\n", id);
		return;
	}
	if(kiavc_room_enable_walkbox(room, name) < 0) {
		/* No such walkbox */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't enable room walkbox, no such walkbox '%s'\n", name);
		return;
	}
	/* Done */
	SDL_Log("Enabled walkbox '%s' in room '%s'\n", name, room->id);
}
static void kiavc_engine_disable_room_walkbox(const char *id, const char *name) {
	if(!id || !name)
		return;
	/* Access room from the map */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't disable room walkbox, no such room '%s'\n", id);
		return;
	}
	if(kiavc_room_disable_walkbox(room, name) < 0) {
		/* No such walkbox */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't disable room walkbox, no such walkbox '%s'\n", name);
		return;
	}
	/* Done */
	SDL_Log("Disabled walkbox '%s' in room '%s'\n", name, room->id);
}
static void kiavc_engine_recalculate_room_walkboxes(const char *id) {
	if(!id)
		return;
	/* Access room from the map */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't recalculate room walkboxes, no such room '%s'\n", id);
		return;
	}
	kiavc_pathfinding_context_recalculate(room->pathfinding);
	/* Done */
	SDL_Log("Recalculated walkboxes in room '%s'\n", room->id);
}
static void kiavc_engine_show_room(const char *id) {
	if(!id)
		return;
	/* Access room */
	kiavc_room *room = kiavc_map_lookup(rooms, id);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set show room, no such room '%s'\n", id);
		return;
	}
	/* Cleanup previous room and render list */
	kiavc_resource *resource = NULL;
	kiavc_list *tmp = engine.render_list;
	while(tmp) {
		resource = (kiavc_resource *)tmp->data;
		if(resource->type == KIAVC_ACTOR) {
			kiavc_actor *actor = (kiavc_actor *)resource;
			kiavc_costume_unload_sets(actor->costume, actor);
		} else if(resource->type == KIAVC_OBJECT) {
			kiavc_object *object = (kiavc_object *)resource;
			if(!object->ui)
				kiavc_animation_unload(object->animation, object);
		} else if(resource->type == KIAVC_ROOM_LAYER) {
			kiavc_room_layer *layer = (kiavc_room_layer *)resource;
			kiavc_animation_unload(layer->background, layer);
		} else if(resource->type == KIAVC_ROOM) {
			kiavc_room *room = (kiavc_room *)resource;
			kiavc_animation_unload(room->background, room);
		}
		tmp = tmp->next;
	}
	kiavc_list_destroy(engine.render_list);
	engine.render_list = NULL;
	/* Setup new room */
	engine.room = room;
	engine.render_list = kiavc_list_insert_sorted(engine.render_list, room, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	kiavc_actor *actor = NULL;
	kiavc_list *item = room->actors;
	while(item) {
		actor = (kiavc_actor *)item->data;
		if(actor->visible) {
			actor->res.ticks = 0;
			engine.render_list = kiavc_list_insert_sorted(engine.render_list, actor, (kiavc_list_item_compare)kiavc_engine_sort_resources);
		}
		item = item->next;
	}
	kiavc_object *object = NULL;
	item = room->objects;
	while(item) {
		object = (kiavc_object *)item->data;
		if(object->visible) {
			object->res.ticks = 0;
			engine.render_list = kiavc_list_insert_sorted(engine.render_list, object, (kiavc_list_item_compare)kiavc_engine_sort_resources);
		}
		item = item->next;
	}
	kiavc_list *objs = kiavc_map_get_values(objects);
	item = objs;
	while(item) {
		object = (kiavc_object *)item->data;
		if(object->ui && object->visible) {
			object->res.ticks = 0;
			engine.render_list = kiavc_list_insert_sorted(engine.render_list, object, (kiavc_list_item_compare)kiavc_engine_sort_resources);
		}
		item = item->next;
	}
	kiavc_list_destroy(objs);
	kiavc_room_layer *layer = NULL;
	item = room->layers;
	while(item) {
		layer = (kiavc_room_layer *)item->data;
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, layer, (kiavc_list_item_compare)kiavc_engine_sort_resources);
		item = item->next;
	}
	if(engine.following && engine.following->room == room) {
		engine.room->res.x = (int)engine.following->res.x - kiavc_screen_width/2;
		engine.room->res.y = (int)engine.following->res.y - kiavc_screen_height/2;
	}
	kiavc_engine_check_hovering();
	/* Done */
	SDL_Log("Shown room '%s'\n", room->id);
}
static void kiavc_engine_register_actor(const char *id) {
	if(!id)
		return;
	/* Check if this actor ID exists already */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(actor) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new actor with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new actor instance and add it to the map */
	actor = kiavc_actor_create(id);
	kiavc_map_insert(actors, actor->id, actor);
	/* Done */
	SDL_Log("Registered actor '%s'\n", actor->id);
}
static void kiavc_engine_set_actor_costume(const char *id, const char *cost) {
	if(!id || !cost)
		return;
	/* Access actor and costume from the respective maps */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor costume, no such actor '%s'\n", id);
		return;
	}
	kiavc_costume *costume = kiavc_map_lookup(costumes, cost);
	if(!costume) {
		/* No such costume */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor costume for actor '%s', no such costume '%s'\n", id, cost);
		return;
	}
	/* Done */
	if(actor->costume)
		kiavc_costume_unload_sets(actor->costume, actor);
	actor->costume = costume;
	SDL_Log("Set costume of actor '%s' to '%s'\n", actor->id, costume->id);
}
static void kiavc_engine_move_actor_to(const char *id, const char *rid, int x, int y) {
	if(!id || !rid)
		return;
	/* Access actor and room from the respective maps */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't move actor, no such actor '%s'\n", id);
		return;
	}
	kiavc_room *room = kiavc_map_lookup(rooms, rid);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't move actor '%s', no such room '%s'\n", id, rid);
		return;
	}
	/* Done */
	if(actor->room)
		actor->room->actors = kiavc_list_remove(actor->room->actors, actor);
	if(!kiavc_list_find(room->actors, actor))
		room->actors = kiavc_list_append(room->actors, actor);
	kiavc_costume_unload_sets(actor->costume, actor);
	engine.render_list = kiavc_list_remove(engine.render_list, actor);
	actor->room = room;
	actor->state = KIAVC_ACTOR_STILL;
	actor->res.x = x;
	actor->res.y = y;
	if(actor->visible && engine.room && engine.room == room)
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, actor, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	if(engine.room && engine.following == actor && engine.following->room == room) {
		engine.room->res.x = (int)engine.following->res.x - kiavc_screen_width/2;
		engine.room->res.y = (int)engine.following->res.y - kiavc_screen_height/2;
	}
	g_list_free_full(actor->path, (GDestroyNotify)kiavc_pathfinding_point_destroy);
	actor->path = NULL;
	actor->step = NULL;
	actor->res.target_x = -1;
	actor->res.target_y = -1;
	/* Check which walkbox we're in */
	if(actor->room && actor->room->pathfinding && actor->room->pathfinding->walkboxes) {
		int x = (int)actor->res.x;
		int y = (int)actor->res.y;
		kiavc_pathfinding_point point = { .x = x, .y = y };
		kiavc_pathfinding_walkbox *walkbox = kiavc_pathfinding_context_find_walkbox(actor->room->pathfinding, &point);
		if(walkbox != actor->walkbox) {
			if(walkbox && actor->room == engine.room) {
				SDL_Log("Actor '%s' now in walkbox (%dx%d -> %dx%d)", actor->id,
					walkbox->p1.x, walkbox->p1.y, walkbox->p2.x, walkbox->p2.y);
				if(walkbox->name) {
					/* Signal script */
					SDL_Log("Actor '%s' triggered walkbox '%s'\n", actor->id, walkbox->name);
					kiavc_scripts_run_command("triggerWalkbox('%s', '%s', '%s')",
						engine.room->id, walkbox->name, actor->id);
				}
			}
			actor->walkbox = walkbox;
		}
	}
	SDL_Log("Moved actor '%s' to room '%s' (%dx%d)\n", actor->id, room->id, (int)actor->res.x, (int)actor->res.y);
}
static void kiavc_engine_show_actor(const char *id) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't show actor, no such actor '%s'\n", id);
		return;
	}
	actor->visible = true;
	/* FIXME Should these be configurable? */
	actor->state = KIAVC_ACTOR_STILL;
	/* Done */
	if(actor->room == engine.room && !kiavc_list_find(engine.render_list, actor))
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, actor, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	SDL_Log("Shown actor '%s'\n", actor->id);
}
static void kiavc_engine_follow_actor(const char *id) {
	/* Get the actor, if an ID has been provided */
	kiavc_actor *actor = id ? kiavc_map_lookup(actors, id) : NULL;
	engine.following = actor;
	/* Done */
	if(actor) {
		SDL_Log("Camera following actor '%s'\n", actor->id);
	} else {
		SDL_Log("Camera not following any actor\n");
		engine.room_direction_x = 0;
		engine.room_direction_y = 0;
	}
}
static void kiavc_engine_hide_actor(const char *id) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't hide actor, no such actor '%s'\n", id);
		return;
	}
	actor->visible = false;
	actor->res.ticks = 0;
	kiavc_costume_unload_sets(actor->costume, actor);
	engine.render_list = kiavc_list_remove(engine.render_list, actor);
	/* Done */
	SDL_Log("Hidden actor '%s'\n", actor->id);
}
static void kiavc_engine_fade_actor_to(const char *id, int alpha, int ms) {
	if(!id || ms < 1)
		return;
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't fade actor, no such actor '%s'\n", id);
		return;
	}
	if(actor->room == engine.room && !kiavc_list_find(engine.render_list, actor))
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, actor, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	actor->res.fade_ms = ms;
	actor->res.fade_start = actor->res.fade_alpha;
	actor->res.fade_target = alpha;
	actor->res.fade_ticks = 0;
	actor->visible = true;
	if(!kiavc_list_find(engine.fading, actor))
		engine.fading = kiavc_list_append(engine.fading, actor);
	/* Done */
	SDL_Log("Fading actor '%s' alpha to '%d'\n", actor->id, alpha);
}
static void kiavc_engine_set_actor_alpha(const char *id, int alpha) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor alpha, no such actor '%s'\n", id);
		return;
	}
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	actor->res.fade_alpha = alpha;
	/* Done */
	SDL_Log("Set actor '%s' alpha to '%d'\n", actor->id, alpha);
}
static void kiavc_engine_set_actor_plane(const char *id, int zplane) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor plane, no such actor '%s'\n", id);
		return;
	}
	actor->res.zplane = zplane;
	engine.render_list = kiavc_list_sort(engine.render_list, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	/* Done */
	SDL_Log("Set actor '%s' plane to '%d'\n", actor->id, zplane);
}
static void kiavc_engine_set_actor_speed(const char *id, int speed) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor speed, no such actor '%s'\n", id);
		return;
	}
	if(speed < 1) {
		/* Invalid speed */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor speed, invalid value '%d'\n", speed);
		return;
	}
	actor->res.speed = speed;
	/* Done */
	SDL_Log("Set actor '%s' speed to '%d'\n", actor->id, speed);
}
static void kiavc_engine_scale_actor(const char *id, float scale) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't scale actor, no such actor '%s'\n", id);
		return;
	}
	actor->scale = scale;
	/* Done */
	SDL_Log("Set actor '%s' scaling to '%f'\n", actor->id, scale);
}
static void kiavc_engine_walk_actor_to(const char *id, int x, int y) {
	if(!id || !engine.room || !engine.room->pathfinding)
		return;
	/* Access actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't walk actor, no such actor '%s'\n", id);
		return;
	}
	/* Find a path to the destination */
	kiavc_pathfinding_point from = { .x = (int)actor->res.x, .y = (int)actor->res.y };
	kiavc_pathfinding_point to = { .x = x, .y = y };
	actor->path = kiavc_pathfinding_context_find_path(engine.room->pathfinding, &from, &to);
	if(!actor->path) {
		/* No path */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't walk actor, no path to destination\n");
		return;
	}
	/* Set the first target */
	kiavc_pathfinding_point *p = (kiavc_pathfinding_point *)actor->path->data;
	actor->res.target_x = p->x;
	actor->res.target_y = p->y;
	actor->step = actor->path->next;
	/* Done */
	SDL_Log("Walking actor '%s' to %dx%d\n", actor->id, to.x, to.y);
}
static void kiavc_engine_say_actor(const char *id, const char *text, const char *fid, SDL_Color *color, SDL_Color *outline) {
	if(!id || !text || !fid || !color)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't have actor talk, no such actor '%s'\n", id);
		return;
	}
	kiavc_font *font = kiavc_map_lookup(fonts, fid);
	if(!font) {
		/* No font */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't have actor talk, no such font '%s'\n", fid);
		return;
	}
	/* Create a line of text */
	if(actor->line) {
		engine.render_list = kiavc_list_remove(engine.render_list, actor->line);
		kiavc_font_text_destroy(actor->line);
	}
	int max_width = (2 * kiavc_screen_width) / 3;
	actor->line = kiavc_font_render_text(font, renderer, text, color, outline, max_width);
	if(actor->line == NULL)
		return;
	actor->line->owner_type = KIAVC_ACTOR;
	actor->line->owner = actor;
	actor->res.target_x = -1;
	actor->res.target_y = -1;
	actor->state = KIAVC_ACTOR_TALKING;
	engine.render_list = kiavc_list_insert_sorted(engine.render_list, actor->line, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	/* Done */
	SDL_Log("Created text for actor '%s'\n", actor->id);
}
static void kiavc_engine_set_actor_direction(const char *id, const char *direction) {
	if(!id || !direction)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor direction, no such actor '%s'\n", id);
		return;
	}
	int dir = kiavc_costume_direction(direction);
	if(dir == KIAVC_NONE) {
		/* Invalid direction */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set actor direction, invalid direction '%s'\n", direction);
		return;
	}
	actor->direction = dir;
	/* Done */
	SDL_Log("Changed actor '%s' direction to '%s'\n", actor->id, direction);
}
static void kiavc_engine_controlled_actor(const char *id) {
	if(!id)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set controlled actor, no such actor '%s'\n", id);
		return;
	}
	engine.actor = actor;
	/* Done */
	SDL_Log("Changed controlled actor to '%s'\n", actor->id);
}
static void kiavc_engine_skip_actors_text(void) {
	/* Iterate on all actor lines and set their duration to 1 */
	kiavc_resource *resource = NULL;
	kiavc_font_text *line = NULL;
	kiavc_list *temp = engine.render_list;
	while(temp) {
		resource = (kiavc_resource *)temp->data;
		if(resource->type == KIAVC_FONT_TEXT) {
			line = (kiavc_font_text *)resource;
			if(line->owner_type == KIAVC_ACTOR)
				line->duration = 1;
		}
		temp = temp->next;
	}
	/* Done */
	SDL_Log("Skipped actors text\n");
}
static void kiavc_engine_set_actor_state(const char *id, const char *type) {
	if(!id || !type)
		return;
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, id);
	if(!actor) {
		/* No such actor */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't have actor use object, no such object '%s'\n", id);
		return;
	}
	actor->state = kiavc_actor_state(type);
	/* Done */
	SDL_Log("Set actor '%s' state to '%s'\n", id, type);
}

static void kiavc_engine_register_costume(const char *id) {
	if(!id)
		return;
	/* Check if this costume ID exists already */
	kiavc_costume *costume = kiavc_map_lookup(costumes, id);
	if(costume) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new costume with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new costume instance and add it to the map */
	costume = kiavc_costume_create(id);
	kiavc_map_insert(costumes, costume->id, costume);
	/* Done */
	SDL_Log("Registered costume '%s'\n", costume->id);
}
static void kiavc_engine_set_costume_animation(const char *id, const char *type, const char *direction, const char *canim) {
	if(!id || !type || !direction || !canim)
		return;
	/* FIXME We only accept still, walking, talking and using for animations at the moment */
	if(SDL_strcasecmp(type, "still") && SDL_strcasecmp(type, "walking") && SDL_strcasecmp(type, "talking") &&
			SDL_strcasecmp(type, "usehigh") && SDL_strcasecmp(type, "usemid") && SDL_strcasecmp(type, "uselow")) {
		/* Invalid direction */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set costume animation, invalid type '%s'\n", type);
		return;
	}
	int dir = kiavc_costume_direction(direction);
	if(dir == KIAVC_NONE) {
		/* Invalid direction */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set costume animation, invalid direction '%s'\n", direction);
		return;
	}
	/* Access costume and image from the respective maps */
	kiavc_costume *costume = kiavc_map_lookup(costumes, id);
	if(!costume) {
		/* No such costume */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set costume animation, no such costume '%s'\n", id);
		return;
	}
	kiavc_animation *anim = kiavc_map_lookup(animations, canim);
	if(!anim) {
		/* No such animation */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set costume animation for costume '%s', no such animation '%s'\n", id, canim);
		return;
	}
	/* FIXME Done */
	kiavc_costume_set *set = kiavc_costume_get_set(costume, type);
	if(!set) {
		/* Error accessing set */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set costume animation for costume '%s', error adding/retrieving set '%s'\n", id, type);
		return;
	}
	set->animations[dir] = anim;
	SDL_Log("Set %s %s animation of costume '%s' to '%s'\n", direction, type, costume->id, anim->id);
}
static void kiavc_engine_register_object(const char *id) {
	if(!id)
		return;
	/* Check if this object ID exists already */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(object) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot register new object with ID '%s', it already exists\n", id);
		return;
	}
	/* Create a new object instance and add it to the map */
	object = kiavc_object_create(id);
	kiavc_map_insert(objects, object->id, object);
	/* Done */
	SDL_Log("Registered object '%s'\n", object->id);
}
static void kiavc_engine_set_object_animation(const char *id, const char *canim) {
	if(!id || !canim)
		return;
	/* Access object and image from the respective maps */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object animation, no such object '%s'\n", id);
		return;
	}
	kiavc_animation *anim = kiavc_map_lookup(animations, canim);
	if(!anim) {
		/* No such animation */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object animation for object '%s', no such animation '%s'\n", id, canim);
		return;
	}
	/* Done */
	object->animation = anim;
	SDL_Log("Set animation of object '%s' to '%s'\n", object->id, anim->id);
}
static void kiavc_engine_set_object_interactable(const char *id, bool interactable) {
	if(!id)
		return;
	/* Access object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object interactable state, no such object '%s'\n", id);
		return;
	}
	/* Done */
	object->interactable = interactable;
	object->res.x = -1;
	object->res.y = -1;
	SDL_Log("Marked object '%s' as %s\n", object->id, interactable ? "interactable" : "NOT interactable");
}
static void kiavc_engine_set_object_ui(const char *id, bool ui) {
	if(!id)
		return;
	/* Access object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object UI state, no such object '%s'\n", id);
		return;
	}
	/* Done */
	object->ui = ui;
	object->res.x = -1;
	object->res.y = -1;
	SDL_Log("Marked object '%s' as %s of the UI\n", object->id, ui ? "part" : "NOT part");
}
static void kiavc_engine_set_object_ui_position(const char *id, int x, int y) {
	if(!id)
		return;
	/* Access object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object UI position, no such object '%s'\n", id);
		return;
	}
	if(!object->ui) {
		/* Not part of the UI */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object UI position, object '%s' not part of the UI\n", id);
		return;
	}
	/* Done */
	object->res.x = x;
	object->res.y = y;
	SDL_Log("Marked object '%s' position in the UI to [%d,%d]\n", object->id, x, y);
}
static void kiavc_engine_set_object_ui_animation(const char *id, const char *canim) {
	if(!id || !canim)
		return;
	/* Access object and image from the respective maps */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object UI animation, no such object '%s'\n", id);
		return;
	}
	kiavc_animation *anim = kiavc_map_lookup(animations, canim);
	if(!anim) {
		/* No such animation */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object UI animation for object '%s', no such animation '%s'\n", id, canim);
		return;
	}
	/* Done */
	object->ui_animation = anim;
	SDL_Log("Set UI animation of object '%s' to '%s'\n", object->id, anim->id);
}
static void kiavc_engine_set_object_parent(const char *id, const char *parent) {
	if(!id || !parent)
		return;
	/* Access objects from the map */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object or not part of the UI */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object parent, no such object '%s'\n", id);
		return;
	}
	kiavc_object *pobj = kiavc_map_lookup(objects, parent);
	if(!pobj) {
		/* No such object or not part of the UI */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object parent, no such object '%s'\n", parent);
		return;
	}
	/* Done */
	object->parent = pobj;
	SDL_Log("Set UI parent of object '%s' to '%s'\n", object->id, pobj->id);
}
static void kiavc_engine_remove_object_parent(const char *id) {
	if(!id)
		return;
	/* Access objects from the map */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove object UI parent, no such object '%s'\n", id);
		return;
	}
	/* Done */
	object->parent = NULL;
	SDL_Log("Removed UI parent of object '%s'\n", object->id);
}
static void kiavc_engine_move_object_to(const char *id, const char *rid, int x, int y) {
	if(!id || !rid)
		return;
	/* Access object and room from the respective maps */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't move object, no such object '%s'\n", id);
		return;
	}
	kiavc_room *room = kiavc_map_lookup(rooms, rid);
	if(!room) {
		/* No such room */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't move object '%s', no such room '%s'\n", id, rid);
		return;
	}
	/* Done */
	if(object->room)
		object->room->objects = kiavc_list_remove(object->room->objects, object);
	if(object->owner)
		object->owner = NULL;
	if(!kiavc_list_find(room->objects, object))
		room->objects = kiavc_list_append(room->objects, object);
	kiavc_animation_unload(object->animation, object);
	engine.render_list = kiavc_list_remove(engine.render_list, object);
	object->room = room;
	object->ui = 0;
	object->res.x = x;
	object->res.y = y;
	if(object->visible && engine.room && engine.room == room)
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, object, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	SDL_Log("Moved object '%s' to room '%s' (%dx%d)\n", object->id, room->id, (int)object->res.x, (int)object->res.y);
}
static void kiavc_engine_float_object_to(const char *id, int x, int y, int speed) {
	if(!id)
		return;
	/* Make sure this object ID exists */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* It does */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot float object, no such object '%s'\n", id);
		return;
	}
	if(speed < 1) {
		/* Invalid speed */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object speed, invalid value '%d'\n", speed);
		return;
	}
	object->res.target_x = x;
	object->res.target_y = y;
	object->res.speed = speed;
	object->res.move_ticks = 0;
	/* Done */
}
static void kiavc_engine_set_object_hover(const char *id, int from_x, int from_y, int to_x, int to_y) {
	if(!id || from_x < 0 || from_y < 0 || to_x < 0 || to_y < 0)
		return;
	/* Access object and room from the respective maps */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't move object, no such object '%s'\n", id);
		return;
	}
	/* Done */
	object->hover.from_x = from_x;
	object->hover.from_y = from_y;
	object->hover.to_x = to_x;
	object->hover.to_y = to_y;
	SDL_Log("Set hover coordinated for object '%s' (%dx%d -> %dx%d)\n", object->id,
		object->hover.from_x, object->hover.from_y, object->hover.to_x, object->hover.to_y);
}
static void kiavc_engine_show_object(const char *id) {
	if(!id)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't show object, no such object '%s'\n", id);
		return;
	}
	object->visible = true;
	/* Done */
	if((object->ui || object->room == engine.room) && !kiavc_list_find(engine.render_list, object))
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, object, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	SDL_Log("Shown object '%s'\n", object->id);
}
static void kiavc_engine_hide_object(const char *id) {
	if(!id)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't hide object, no such object '%s'\n", id);
		return;
	}
	object->visible = false;
	object->res.ticks = 0;
	kiavc_animation_unload(object->animation, object);
	kiavc_animation_unload(object->ui_animation, object);
	engine.render_list = kiavc_list_remove(engine.render_list, object);
	/* Done */
	SDL_Log("Hidden object '%s'\n", object->id);
}
static void kiavc_engine_fade_object_to(const char *id, int alpha, int ms) {
	if(!id || ms < 1)
		return;
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't fade object, no such object '%s'\n", id);
		return;
	}
	if((object->ui || object->room == engine.room) && !kiavc_list_find(engine.render_list, object))
		engine.render_list = kiavc_list_insert_sorted(engine.render_list, object, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	object->res.fade_ms = ms;
	object->res.fade_start = object->res.fade_alpha;
	object->res.fade_target = alpha;
	object->res.fade_ticks = 0;
	object->visible = true;
	if(!kiavc_list_find(engine.fading, object))
		engine.fading = kiavc_list_append(engine.fading, object);
	/* Done */
	SDL_Log("Fading object '%s' alpha to '%d'\n", object->id, alpha);
}
static void kiavc_engine_set_object_alpha(const char *id, int alpha) {
	if(!id)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object alpha, no such object '%s'\n", id);
		return;
	}
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	object->res.fade_alpha = alpha;
	/* Done */
	SDL_Log("Set object '%s' alpha to '%d'\n", object->id, alpha);
}
static void kiavc_engine_set_object_plane(const char *id, int zplane) {
	if(!id)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set object plane, no such object '%s'\n", id);
		return;
	}
	object->res.zplane = zplane;
	engine.render_list = kiavc_list_sort(engine.render_list, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	/* Done */
	SDL_Log("Set object '%s' plane to '%d'\n", object->id, zplane);
}
static void kiavc_engine_scale_object(const char *id, float scale) {
	if(!id)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't scale object, no such object '%s'\n", id);
		return;
	}
	object->scale = scale;
	/* Done */
	SDL_Log("Set object '%s' scaling to '%f'\n", object->id, scale);
}
static void kiavc_engine_add_object_to_inventory(const char *id, const char *owner) {
	if(!id || !owner)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add object to inventory, no such object '%s'\n", id);
		return;
	}
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, owner);
	if(!actor) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't add object to inventory, no such actor '%s'\n", id);
		return;
	}
	/* FIXME */
	if(object->room)
		object->room->objects = kiavc_list_remove(object->room->objects, object);
	object->room = NULL;
	object->owner = actor;
	object->visible = false;
	/* Done */
	SDL_Log("Added object '%s' to actor '%s' inventory\n", object->id, actor->id);
}
static void kiavc_engine_remove_object_from_inventory(const char *id, const char *owner) {
	if(!id || !owner)
		return;
	/* Get the object */
	kiavc_object *object = kiavc_map_lookup(objects, id);
	if(!object) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove object from inventory, no such object '%s'\n", id);
		return;
	}
	/* Get the actor */
	kiavc_actor *actor = kiavc_map_lookup(actors, owner);
	if(!actor) {
		/* No such object */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove object from inventory, no such actor '%s'\n", id);
		return;
	}
	/* FIXME */
	if(object->room)
		object->room->objects = kiavc_list_remove(object->room->objects, object);
	object->room = NULL;
	object->owner = actor;
	/* Done */
	SDL_Log("Removed object '%s' from actor '%s' inventory\n", object->id, actor->id);
}
static void kiavc_engine_show_text(const char *id, const char *text, const char *fid,
		SDL_Color *color, SDL_Color *outline, int x, int y, int alpha, bool absolute, int zplane, Uint32 ms) {
	if(!text || !fid || !color)
		return;
	if(id) {
		/* Check if this text ID exists already */
		kiavc_font_text *line = kiavc_map_lookup(texts, id);
		if(line) {
			/* It does */
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot show text with ID '%s', it already exists\n", id);
			return;
		}
	}
	kiavc_font *font = kiavc_map_lookup(fonts, fid);
	if(!font) {
		/* No font */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't show text, no such font '%s'\n", fid);
		return;
	}
	int max_width = kiavc_screen_width;
	if(max_width > (kiavc_screen_width - 10))
		max_width = kiavc_screen_width - 10;
	kiavc_font_text *line = kiavc_font_render_text(font, renderer, text, color, outline, max_width);
	if(line == NULL)
		return;
	line->res.x = x;
	line->res.y = y;
	line->res.zplane = zplane;
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	line->res.fade_alpha = alpha;
	line->absolute = absolute;
	line->duration = ms;
	if(id) {
		SDL_Log("Assigning ID to new text line: '%s'\n", id);
		line->id = SDL_strdup(id);
		kiavc_map_insert(texts, SDL_strdup(id), line);
	}
	engine.render_list = kiavc_list_insert_sorted(engine.render_list, line, (kiavc_list_item_compare)kiavc_engine_sort_resources);
	/* Done */
}
static void kiavc_engine_float_text_to(const char *id, int x, int y, int speed) {
	if(!id)
		return;
	/* Make sure this text ID exists */
	kiavc_font_text *line = kiavc_map_lookup(texts, id);
	if(!line) {
		/* It doesn't */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot float text, no such text '%s'\n", id);
		return;
	}
	if(speed < 1) {
		/* Invalid speed */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set text speed, invalid value '%d'\n", speed);
		return;
	}
	line->res.target_x = x;
	line->res.target_y = y;
	line->res.speed = speed;
	line->res.move_ticks = 0;
	/* Done */
	SDL_Log("Floating text '%s' to %dx%d at speed %d\n", line->id, x, y, speed);
}
static void kiavc_engine_fade_text_to(const char *id, int alpha, int ms) {
	if(!id || ms < 1)
		return;
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	/* Make sure this text ID exists */
	kiavc_font_text *line = kiavc_map_lookup(texts, id);
	if(!line) {
		/* It doesn't */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't fade text, no such text '%s'\n", id);
		return;
	}
	line->res.fade_ms = ms;
	line->res.fade_start = line->res.fade_alpha;
	line->res.fade_target = alpha;
	line->res.fade_ticks = 0;
	if(!kiavc_list_find(engine.fading, line))
		engine.fading = kiavc_list_append(engine.fading, line);
	/* Done */
	SDL_Log("Fading text '%s' alpha to '%d'\n", line->id, alpha);
}
static void kiavc_engine_set_text_alpha(const char *id, int alpha) {
	if(!id)
		return;
	/* Make sure this text ID exists */
	kiavc_font_text *line = kiavc_map_lookup(texts, id);
	if(!line) {
		/* It doesn't */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't set the text alpha, no such text '%s'\n", id);
		return;
	}
	if(alpha > 255)
		alpha = 255;
	else if(alpha < 0)
		alpha = 0;
	line->res.fade_alpha = alpha;
	/* Done */
	SDL_Log("Set text '%s' alpha to '%d'\n", line->id, alpha);
}
static void kiavc_engine_remove_text(const char *id) {
	if(!id)
		return;
	/* Get the text */
	kiavc_font_text *line = kiavc_map_lookup(texts, id);
	if(!line) {
		/* No such text */
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Can't remove text, no such text '%s'\n", id);
		return;
	}
	engine.render_list = kiavc_list_remove(engine.render_list, line);
	engine.fading = kiavc_list_remove(engine.fading, line);
	kiavc_map_remove(texts, id);
	/* Done */
	SDL_Log("Removed text '%s'\n", id);
}
static void kiavc_engine_quit(void) {
	SDL_Log("Quitting the engine\n");
	quit = true;
}
