/*
 *
 * KIAVC dialog puzzle session management, where we simply keep track
 * of an ongoing dialog puzzle, the lines to display, and any opaque
 * metadata related to them when we should report about the selection.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include "dialog.h"

/* Helper to free a dialog line */
static void kiavc_dialog_line_destroy(kiavc_dialog_line *line) {
	if(!line)
		return;
	SDL_free(line->name);
	kiavc_font_text_destroy(line->text);
	kiavc_font_text_destroy(line->selected);
	SDL_free(line);
}

/* Helper to create a dialog line */
static kiavc_dialog_line *kiavc_dialog_line_create(kiavc_dialog *dialog, const char *name, const char *text) {
	if(!dialog || !dialog->font || !dialog->font || !name || !text)
		return NULL;
	kiavc_dialog_line *line = SDL_calloc(1, sizeof(kiavc_dialog_line));
	line->name = SDL_strdup(name);
	line->text = kiavc_font_render_text(dialog->font, dialog->renderer, text,
		&dialog->color, dialog->border ? &dialog->outline : NULL, dialog->max_width);
	if(!line->text) {
		kiavc_dialog_line_destroy(line);
		return NULL;
	}
	line->text->owner_type = KIAVC_DIALOG;
	line->text->owner = dialog;
	line->selected = kiavc_font_render_text(dialog->font, dialog->renderer, text,
		&dialog->s_color, dialog->s_border ? &dialog->s_outline : NULL, dialog->max_width);
	if(!line->selected) {
		kiavc_dialog_line_destroy(line);
		return NULL;
	}
	line->selected->owner_type = KIAVC_DIALOG;
	line->selected->owner = dialog;
	/* Done */
	return line;
}

/* Helper to create a new dialog */
kiavc_dialog *kiavc_dialog_create(const char *id) {
	if(!id)
		return NULL;
	kiavc_dialog *dialog = SDL_calloc(1, sizeof(kiavc_dialog));
	dialog->res.type = KIAVC_DIALOG;
	dialog->res.zplane = 98;	/* By default a dialog line only goes below a pointer */
	dialog->id = SDL_strdup(id);
	return dialog;
}

/* Helper to add a line to a dialog */
kiavc_dialog_line *kiavc_dialog_add_line(kiavc_dialog *dialog, const char *name, const char *text) {
	if(!dialog || !name || !text)
		return NULL;
	kiavc_dialog_line *line = kiavc_dialog_line_create(dialog, name, text);
	if(!line) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error generating dialog line\n");
		return NULL;
	}
	line->index = kiavc_list_size(dialog->lines);
	/* FIXME */
	int h = dialog->area.h / 4;
	if(line->text) {
		line->text->x = 0;
		line->text->y = line->index * h;
	}
	if(line->selected) {
		line->selected->x = 0;
		line->selected->y = line->index * h;
	}
	dialog->lines = kiavc_list_append(dialog->lines, line);
	return line;
}

/* Helper to clear a dialog */
void kiavc_dialog_clear(kiavc_dialog *dialog) {
	if(!dialog)
		return;
	dialog->selected = NULL;
	g_list_free_full(dialog->lines, (GDestroyNotify)kiavc_dialog_line_destroy);
	dialog->lines = NULL;
}

/* Helper to destroy a dialog */
void kiavc_dialog_destroy(kiavc_dialog *dialog) {
	if(!dialog)
		return;
	SDL_free(dialog->id);
	g_list_free_full(dialog->lines, (GDestroyNotify)kiavc_dialog_line_destroy);
	SDL_free(dialog);
}
