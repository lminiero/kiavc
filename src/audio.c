/*
 *
 * KIAVC audio management implementation. This structure abstracts the
 * functionality of SDL_mixer with respect to how you can import, play,
 * pause, resume and stop the playback of track tracks or sound effects,
 * optionally with fade-in and fade-out. The abstraction also internally
 * assigns different mixer channels to different track tracks as they're
 * imported, meaning multiple audio tracks can be played at the same
 * time if so required. Notice that you can choose whether to loop
 * audio tracks automatically when playing (e.g., for background track)
 * or just play them once (e.g., for sound effects).
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#include <SDL2/SDL_mixer.h>

#include "engine.h"
#include "audio.h"
#include "map.h"

/* Callback to be notified when playback of a track finishes */
static SDL_mutex *mutex = NULL;
static kiavc_map *tracks_by_channel = NULL;
static void kiavc_audio_channel_finished(int channel) {
	/* FIXME This comes from a different thread, so we need locking */
	char channel_str[10];
	channel_str[0] = '\0';
	SDL_LockMutex(mutex);
	SDL_snprintf(channel_str, sizeof(channel_str)-1, "%d", channel);
	kiavc_audio *track = kiavc_map_lookup(tracks_by_channel, channel_str);
	if(!track) {
		SDL_Log("Audio channel %d finished playing (unknown track)\n", channel);
	} else {
		SDL_Log("Audio track '%s' finished playing\n", track->id);
		track->channel = -1;
		track->playing = false;
		track->paused = false;
		kiavc_map_remove(tracks_by_channel, channel_str);
	}
	SDL_UnlockMutex(mutex);
}

/* Music constructor */
kiavc_audio *kiavc_audio_create(const char *id, const char *path) {
	if(!id || !path)
		return NULL;
	kiavc_audio *track = SDL_calloc(1, sizeof(kiavc_audio));
	track->id = SDL_strdup(id);
	track->path = SDL_strdup(path);
	track->channel = -1;
	/* If we haven't started the "channel finished" callback yet, let's do it now */
	if(tracks_by_channel == NULL) {
		tracks_by_channel = kiavc_map_create(NULL);
		Mix_ChannelFinished(&kiavc_audio_channel_finished);
	}
	/* Do the same with the global audio mutex */
	if(mutex == NULL)
		mutex = SDL_CreateMutex();
	return track;
}

/* Music track initialization */
int kiavc_audio_load(kiavc_audio *track) {
	SDL_LockMutex(mutex);
	if(!track) {
		SDL_UnlockMutex(mutex);
		return -1;
	}
	/* If the chunk exists already, do nothing */
	if(track->chunk) {
		SDL_UnlockMutex(mutex);
		return 0;
	}
	track->chunk = Mix_LoadWAV_RW(kiavc_engine_open_file(track->path), 1);
	if(track->chunk == NULL) {
		SDL_UnlockMutex(mutex);
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading track track: %s\n", Mix_GetError());
		return -2;
	}
	SDL_UnlockMutex(mutex);
	return 0;
}

/* Music track de-initialization */
void kiavc_audio_unload(kiavc_audio *track) {
	if(!track)
		return;
	SDL_LockMutex(mutex);
	kiavc_audio_stop(track, 0);
	if(track->chunk)
		Mix_FreeChunk(track->chunk);
	track->chunk = NULL;
	track->playing = false;
	track->paused = false;
	SDL_UnlockMutex(mutex);
}

/* Start audio track */
void kiavc_audio_play(kiavc_audio *track, int fade_ms, bool loop) {
	SDL_LockMutex(mutex);
	kiavc_audio_load(track);
	if(track && track->chunk && !track->playing) {
		if(fade_ms > 0) {
			track->channel = Mix_FadeInChannel(track->channel, track->chunk, loop ? -1 : 0, fade_ms);
		} else {
			track->channel = Mix_PlayChannel(track->channel, track->chunk, loop ? -1 : 0);
		}
		if(track->channel == -1) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error playing track track '%s': %s\n", track->id, Mix_GetError());
		} else {
			track->playing = true;
			/* Map the channel to this track */
			char channel_str[10];
			channel_str[0] = '\0';
			SDL_snprintf(channel_str, sizeof(channel_str)-1, "%d", track->channel);
			kiavc_map_insert(tracks_by_channel, channel_str, track);
		}
	}
	SDL_UnlockMutex(mutex);
}

/* Pause audio track */
void kiavc_audio_pause(kiavc_audio *track) {
	SDL_LockMutex(mutex);
	if(track && !track->paused && track->channel != -1) {
		Mix_Pause(track->channel);
		track->paused = true;
	}
	SDL_UnlockMutex(mutex);
}

/* Resume audio track */
void kiavc_audio_resume(kiavc_audio *track) {
	SDL_LockMutex(mutex);
	if(track && track->paused && track->channel != -1) {
		Mix_Resume(track->channel);
		track->paused = true;
	}
	SDL_UnlockMutex(mutex);
}

/* Stop audio track */
void kiavc_audio_stop(kiavc_audio *track, int fade_ms) {
	SDL_LockMutex(mutex);
	if(track && track->playing && track->channel != -1) {
		if(fade_ms > 0)
			Mix_FadeOutChannel(track->channel, fade_ms);
		else
			Mix_HaltChannel(track->channel);
	}
	SDL_UnlockMutex(mutex);
}

/* Audio track destructor */
void kiavc_audio_destroy(kiavc_audio *track) {
	SDL_LockMutex(mutex);
	if(track) {
		if(track->channel > -1) {
			char channel_str[10];
			channel_str[0] = '\0';
			SDL_snprintf(channel_str, sizeof(channel_str)-1, "%d", track->channel);
			kiavc_map_remove(tracks_by_channel, channel_str);
		}
		SDL_free(track->id);
		SDL_free(track->path);
		kiavc_audio_unload(track);
		SDL_free(track);
	}
	SDL_UnlockMutex(mutex);
}
