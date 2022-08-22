/*
 *
 * KIAVC font implementation. It's basically a wrapper to the SDL_ttf
 * functionality, where each font instance can be abstracted in a
 * separate instance and used accordingly. A rendered piece of text
 * is also abstracted as its own instance, in order to make it easier
 * to create textures out of text we need to display somewhere using
 * one of the fonts we registered before.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_FONT_H
#define __KIAVC_FONT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "resources.h"

/* Abstraction of a font in the KIAVC engine */
typedef struct kiavc_font {
	/* Unique ID of the font */
	char *id;
	/* Path to the font */
	char *path;
	/* Size of the font */
	int size;
	/* Size of the outline, if any */
	int outline_size;
	/* Font instances */
	TTF_Font *instance, *outline;
} kiavc_font;

/* A rendered text */
typedef struct kiavc_font_text {
	/* Common resource info */
	kiavc_resource res;
	/* Texture for the rendered text */
	SDL_Texture *texture;
	/* Size of the text */
	int w, h;
	/* Where to place the text */
	int x, y;
	/* Timers */
	uint32_t duration, started;
	/* Resource type of who owns this text */
	int owner_type;
	/* Opaque pointer to the owner resource, if any */
	void *owner;
} kiavc_font_text;

/* Font constructor */
kiavc_font *kiavc_font_create(const char *id, const char *path, int size);
/* TTF font initialization */
int kiavc_font_load(kiavc_font *font);
/* TTF font de-initialization */
void kiavc_font_unload(kiavc_font *font);
/* Helper to render text using the specified font */
kiavc_font_text *kiavc_font_render_text(kiavc_font *font, SDL_Renderer *renderer,
	const char *text, SDL_Color *color, SDL_Color *bg_color, int max_width);
/* Font destructor */
void kiavc_font_destroy(kiavc_font *font);
/* Font text destructor */
void kiavc_font_text_destroy(kiavc_font_text *text);

#endif
