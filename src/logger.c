/*
 *
 * KIAVC logger implementation. It redefines the SDL logger so that
 * we can both print to output and log to the provided file.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <errno.h>

#include <SDL2/SDL.h>
#include <glib.h>

#include "logger.h"

/* Copied from SDL_log.c */
static const char *SDL_priority_prefixes[SDL_NUM_LOG_PRIORITIES] = {
	NULL,
	"VERBOSE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"CRITICAL"
};

/* The SDL2 provided user-and-app-specific path where files can be written */
static char *path = NULL;
/* File where we're saving the log to */
static FILE *logfile = NULL;
/* Whether we should log to stdout too */
static bool logterm = true;

/* Our custom logger, which prints to output and saves to file */
static void kiavc_logger(void *userdata, int category, SDL_LogPriority priority, const char *message) {
	/* If a log file exists, print there */
	if(logfile) {
		time_t ltime = time(NULL);
		struct tm *timeinfo = localtime(&ltime);
		char log_ts[64] = "";
		strftime(log_ts, sizeof(log_ts), "[%d/%m/%y %H:%M:%S] ", timeinfo);
		char line[1024];
		g_snprintf(line, sizeof(line)-1, "%s%s: %s\n",
			log_ts, SDL_priority_prefixes[priority], message);
		fwrite(line, strlen(line), 1, logfile);
		fflush(logfile);
	}
	if(logterm) {
		/* Print to standard output (too) */
		g_print("%s: %s\n", SDL_priority_prefixes[priority], message);
	}
}

/* Initialize the logger */
void kiavc_logger_init(const char *app, bool term) {
	/* First or all, let's get the path where we're supposed to be able
	 * to write files: at the moment we're hardcoding KIAVC as the org,
	 * while we expect the app name to be provided to us. KIAVC will
	 * read it in the game.kvc file (from the bag file, if needed) */
	path = SDL_GetPrefPath("KIAVC", app);
	/* Create a log file in that path
	 * FIXME This should take into account the Linux/Windows/Mac differences */
	char logpath[1024];
	g_snprintf(logpath, sizeof(logpath)-1, "%skiavc.log", path);
	logfile = fopen(logpath, "at");
	if(logfile == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Error creating logfile: %s\n", strerror(errno));
	} else {
		/* Change the logger function with our own, so that we log to file too */
		SDL_Log("Logging to '%s'\n", logpath);
		logterm = term;
		SDL_LogSetOutputFunction(&kiavc_logger, NULL);
	}
}

/* Destroy the logger */
void kiavc_logger_destroy(void) {
	if(logfile)
		fclose(logfile);
	SDL_free(path);
}
