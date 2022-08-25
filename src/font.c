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

#include "engine.h"
#include "font.h"
#include "list.h"
#include "utils.h"

/* Font constructor */
kiavc_font *kiavc_font_create(const char *id, const char *path, int size) {
	if(!id || !path || size < 1)
		return NULL;
	kiavc_font *font = SDL_calloc(1, sizeof(kiavc_font));
	font->id = SDL_strdup(id);
	font->path = SDL_strdup(path);
	font->size = size;
	font->outline_size = 1;	/* FIXME */
	return font;
}

/* TTF font initialization */
int kiavc_font_load(kiavc_font *font) {
	if(!font)
		return -1;
	/* If we loaded this font already, do nothing */
	if(font->instance)
		return 0;
	/* Open the font */
	font->instance = TTF_OpenFontRW(kiavc_engine_open_file(font->path), 1, font->size);
	if(!font->instance) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading font: %s\n", TTF_GetError());
		return -2;
	}
	/* Open a different instance for the outline */
	font->outline = TTF_OpenFontRW(kiavc_engine_open_file(font->path), 1, font->size);
	if(!font->outline) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading outline font: %s\n", TTF_GetError());
		return -2;
	}
	TTF_SetFontOutline(font->outline, font->outline_size);
	//~ TTF_SetFontWrappedAlign(font->instance, TTF_WRAPPED_ALIGN_CENTER);
	//~ TTF_SetFontWrappedAlign(font->outline, TTF_WRAPPED_ALIGN_CENTER);
	return 0;
}

/* TTF font de-initialization */
void kiavc_font_unload(kiavc_font *font) {
	if(!font)
		return;
	if(font->instance)
		TTF_CloseFont(font->instance);
	font->instance = NULL;
	if(font->outline)
		TTF_CloseFont(font->outline);
	font->outline = NULL;
}

/* Private helper to render a single line of text, which is helpful in
 * case the string we need to render will actually be multiple lines */
static SDL_Surface *kiavc_font_render_helper(kiavc_font *font, SDL_Renderer *renderer,
		const char *text, SDL_Color *color, SDL_Color *bg_color, int max_width) {
	if(!font || !renderer || !text || !color)
		return NULL;
	int w = 0, h = 0;
	if(TTF_SizeUTF8(font->instance, text, &w, &h) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't estimate text size: %s\n", TTF_GetError());
	} else if(w > max_width) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Text to render is too long (%d > %d), you should split it with new lines\n",
			w, max_width);
	}
	SDL_Surface *s_text = NULL;
	if(!font->outline || !bg_color) {
		/* Render solid text */
		s_text = TTF_RenderUTF8_Solid(font->instance, text, *color);
		if(s_text == NULL) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error generating text: %s\n", TTF_GetError());
			return NULL;
		}
	} else {
		/* Blend foreground and outline */
		s_text = TTF_RenderUTF8_Blended(font->outline, text, *bg_color);
		if(s_text == NULL) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error generating outline text: %s\n", TTF_GetError());
			return NULL;
		}
		SDL_Surface *fg_text = TTF_RenderUTF8_Solid(font->instance, text, *color);
		if(fg_text == NULL) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error generating foreground text: %s\n", TTF_GetError());
			return NULL;
		}
		SDL_Rect rect = { .x = font->outline_size, .y = font->outline_size, .w = s_text->w, .h = s_text->h };
		SDL_BlitSurface(fg_text, NULL, s_text, &rect);
		SDL_FreeSurface(fg_text);
	}
	return s_text;
}

/* Helper to render text using the specified font */
kiavc_font_text *kiavc_font_render_text(kiavc_font *font, SDL_Renderer *renderer,
		const char *text, SDL_Color *color, SDL_Color *bg_color, int max_width) {
	if(!font || !renderer || !text || !color)
		return NULL;
	kiavc_font_load(font);
	if(!font->instance) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error generating text: invalid font\n");
		return NULL;
	}
	/* Estimate the rendered text size */
	int w = 0, h = 0;
	if(TTF_SizeUTF8(font->instance, text, &w, &h) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't estimate text size: %s\n", TTF_GetError());
	}
	/* Check if we should split on multiple lines */
	SDL_Surface *s_text = NULL;
	if(max_width == 0 || w <= max_width) {
		/* The text fits the maximum width, render as it is */
		s_text = kiavc_font_render_helper(font, renderer, text, color, bg_color, max_width);
	} else {
		/* Split in multiple strings before rendering */
		int len = strlen(text);
		float diff = (float)w / (float)max_width;
		int diff_dec = (int)diff;
		if(diff > (float)diff_dec)
			diff_dec++;
		int line_len = len / diff_dec;
		char **words = g_strsplit(text, " ", -1);
		int index = 0, s_height = 0, s_width = 0, word_len = 0, line_offset = 0;
		char line[256];
		line[0] = '\0';
		SDL_Surface *s_line = NULL;
		kiavc_list *surfaces = NULL;
		if(words) {
			while(true) {
				word_len = words[index] ? SDL_strlen(words[index]) : 0;
				if(!words[index] || line_offset + word_len > line_len) {
					/* Render the string we came up with so far */
					s_line = kiavc_font_render_helper(font, renderer, line, color, bg_color, max_width);
					if(s_width < s_line->w)
						s_width = s_line->w;
					s_height += s_line->h;
					surfaces = kiavc_list_append(surfaces, s_line);
					line[0] = '\0';
					line_offset = 0;
				}
				if(!words[index])
					break;
				/* Add another word to the current line */
				SDL_snprintf(line + line_offset, sizeof(line) - line_offset - 1,
					"%s%s", line_offset ? " " : "", words[index]);
				line_offset += word_len;
				if(line_offset > word_len)
					line_offset++;
				index++;
			}
			g_strfreev(words);
		}
		/* Create the complete surface */
		s_text = kiavc_create_surface(s_width, s_height);
		/* Blit the individual lines on the target surface */
		kiavc_list *temp = surfaces;
		SDL_Rect rect = { 0 };
		s_height = 0;
		while(s_text && temp) {
			s_line = (SDL_Surface *)temp->data;
			rect.x = (s_text->w - s_line->w) / 2;
			rect.y = s_height;
			rect.w = s_line->w;
			rect.h = s_line->h;
			if(SDL_BlitSurface(s_line, NULL, s_text, &rect) < 0) {
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error blitting text: %s\n", SDL_GetError());
				SDL_FreeSurface(s_text);
				s_text = NULL;
				break;
			}
			s_height += s_line->h;
			temp = temp->next;
		}
		/* Done */
		g_list_free_full(surfaces, (GDestroyNotify)SDL_FreeSurface);
	}
	if(!s_text)
		return NULL;
	/* Now that we have rendered text, create a struct to host it */
	kiavc_font_text *ft = SDL_calloc(1, sizeof(kiavc_font_text));
	ft->res.type = KIAVC_FONT_TEXT;
	ft->res.zplane = 50;	/* By default text is written on top of most things */
	ft->texture = SDL_CreateTextureFromSurface(renderer, s_text);
	ft->w = s_text->w;
	ft->h = s_text->h;
	ft->x = -1;
	ft->y = -1;
	/* FIXME Compute how long this should be displayed */
	ft->duration = 1000 * (SDL_strlen(text)/10);
	if(ft->duration == 0)
		ft->duration = 500;
	SDL_FreeSurface(s_text);
	if(ft->texture == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating text texture: %s\n", SDL_GetError());
		kiavc_font_text_destroy(ft);
		return NULL;
	}
	return ft;
}

/* Font destructor */
void kiavc_font_destroy(kiavc_font *font) {
	if(font) {
		SDL_free(font->id);
		kiavc_font_unload(font);
		SDL_free(font);
	}
}

/* Font text destructor */
void kiavc_font_text_destroy(kiavc_font_text *text) {
	if(text && text->texture) {
		SDL_DestroyTexture(text->texture);
		SDL_free(text);
	}
}
