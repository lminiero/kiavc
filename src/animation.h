/*
 *
 * KIAVC animation management implementation. This structure abstracts
 * the process of importing an image and converting it to a set of SDL
 * textures corresponding to an animation, optionally taking into
 * account how to handle transparency. As such, this is the structure
 * all other resources in the engine use and refer to when they need to
 * work with static images. Notice that the implementation makes
 * several assumptions that will need to be revisited in the future:
 * for instance, it assumes an animation will be provided as a single
 * image with a row of frames, where each frame has the same size,
 * meaning the number of frames is implicitly computed from the overall
 * image size. Besides, it currently provides no information related to
 * how long each frame should be displayed when rendering the animation.
 * An animation with a single frame is a static image.
 *
 * Author: Lorenzo Miniero (lorenzo@gmail.com)
 *
 */

#ifndef __KIAVC_ANIMATION_H
#define __KIAVC_ANIMATION_H

#include <SDL2/SDL.h>

/* Abstraction of an animation in the KIAVC engine */
typedef struct kiavc_animation {
	/* Unique ID of the animation */
	char *id;
	/* Path to the image */
	char *path;
	/* Color keying, if required */
	Uint8 transparency, t_r, t_g, t_b;
	/* Texture of the animation */
	SDL_Texture *texture;
	/* Size of each frame */
	int w, h;
	/* Number of frames in the animation */
	int frames;
} kiavc_animation;

/* Animation constructor */
kiavc_animation *kiavc_animation_create(const char *id, const char *path,
	int frames, SDL_Color *transparency);
/* Animation image initialization */
int kiavc_animation_load(kiavc_animation *anim, SDL_Renderer *renderer);
/* Animation image de-initialization */
void kiavc_animation_unload(kiavc_animation *anim);
/* Animation destructor */
void kiavc_animation_destroy(kiavc_animation *anim);

#endif
