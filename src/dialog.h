/*
 *
 * KIAVC dialog puzzle session management, where we simply keep track
 * of an ongoing dialog puzzle, the lines to display, and any opaque
 * metadata related to them when we should report about the selection.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_DIALOG_H
#define __KIAVC_DIALOG_H

#include "list.h"
#include "font.h"
#include "resources.h"

/* Dialog session line */
typedef struct kiavc_dialog_line {
	/* Index of the line in the dialog */
	int index;
	/* Name of the line, if provided */
	char *name;
	/* Rendered text */
	kiavc_font_text *text, *selected;
} kiavc_dialog_line;

/* Dialog session */
typedef struct kiavc_dialog {
	/* Common resource info */
	kiavc_resource res;
	/* ID of the dialog */
	char *id;
	/* SDL renderer */
	SDL_Renderer *renderer;
	/* Font used to render dialog lines */
	kiavc_font *font;
	/* Max width of the text */
	int max_width;
	/* Text colors */
	SDL_Color color, outline;
	/* Selected text colors */
	SDL_Color s_color, s_outline;
	/* Whether we should draw an outline */
	bool border, s_border;
	/* Background color */
	SDL_Color background;
	/* Area of the window where we should draw the dialog */
	SDL_Rect area;
	/* Whether the background should be hidden when a line has been selected */
	bool autohide;
	/* List of lines to display */
	kiavc_list *lines;
	/* Currently selected line */
	kiavc_dialog_line *selected;
	/* Whether this dialog is currently active */
	bool active;
} kiavc_dialog;

/* Helper to create a new dialog */
kiavc_dialog *kiavc_dialog_create(const char *id);
/* Helper to add a line to a dialog */
kiavc_dialog_line *kiavc_dialog_add_line(kiavc_dialog *dialog, const char *name, const char *text);
/* Helper to clear a dialog */
void kiavc_dialog_clear(kiavc_dialog *dialog);
/* Helper to destroy a dialog */
void kiavc_dialog_destroy(kiavc_dialog *dialog);

#endif
