/*
 *
 * KIAVC audio management implementation. This structure abstracts the
 * functionality of SDL_mixer with respect to how you can import, play,
 * pause, resume and stop the playback of music tracks or sound effects,
 * optionally with fade-in and fade-out. The abstraction also internally
 * assigns different mixer channels to different music tracks as they're
 * imported, meaning multiple audio tracks can be played at the same
 * time if so required. Notice that you can choose whether to loop
 * audio tracks automatically when playing (e.g., for background music)
 * or just play them once (e.g., for sound effects).
 *
 * Author: Lorenzo Miniero (lminiero@gmail.com)
 *
 */

#ifndef __KIAVC_AUDIO_H
#define __KIAVC_AUDIO_H

#include <stdbool.h>

#include <SDL2/SDL.h>

/* Abstraction of an audio track in the KIAVC engine */
typedef struct kiavc_audio {
	/* Unique ID of the audio track instance */
	char *id;
	/* Path to the audio file */
	char *path;
	/* Channel assigned to this audio track */
	int channel;
	/* Chunk instance */
	Mix_Chunk *chunk;
	/* Whether the track is playing or not */
	bool playing;
	/* Whether the track is paused or not */
	bool paused;
} kiavc_audio;

/* Audio track constructor */
kiavc_audio *kiavc_audio_create(const char *id, const char *path);
/* Audio track track initialization */
int kiavc_audio_load(kiavc_audio *track);
/* Audio track track de-initialization */
void kiavc_audio_unload(kiavc_audio *track);
/* Start audio track */
void kiavc_audio_play(kiavc_audio *track, int fade_ms, bool loop);
/* Pause audio track */
void kiavc_audio_pause(kiavc_audio *track);
/* Resume audio track */
void kiavc_audio_resume(kiavc_audio *track);
/* Stop audio track */
void kiavc_audio_stop(kiavc_audio *track, int fade_ms);
/* Audio track destructor */
void kiavc_audio_destroy(kiavc_audio *track);

#endif
