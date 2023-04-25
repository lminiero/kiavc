/*
 *
 * Main KIAVC engine implementation. It takes care of the C side of
 * things, with respect to user input (mouse, keyboard), updating the
 * world (e.g., progressing animations according to the ticks) and
 * actual audio and video rendering using SDL. Besides, it interacts
 * with the Lua scripts in both directions.
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef __KIAVC_ENGINE_H
#define __KIAVC_ENGINE_H

#include <SDL2/SDL.h>

#include "bag.h"

/* Initialize the engine */
int kiavc_engine_init(const char *app, kiavc_bag *bagfile);
/* Return a SDL_RWops instance for a path */
SDL_RWops *kiavc_engine_open_file(const char *path);
/* Handle input from the user */
int kiavc_engine_handle_input(void);
/* Update the "world" */
int kiavc_engine_update_world(void);
/* Render the current frames */
int kiavc_engine_render(void);
/* Destroy the engine */
void kiavc_engine_destroy(void);

#endif
