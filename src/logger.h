/*
 *
 * KIAVC logger implementation. It redefines the SDL logger so that
 * we can both print to output and log to the provided file.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_LOGGER_H
#define __KIAVC_LOGGER_H

#include <stdbool.h>

/* Initialize the logger */
void kiavc_logger_init(const char *app, bool term);
/* Destroy the logger */
void kiavc_logger_destroy(void);

#endif
