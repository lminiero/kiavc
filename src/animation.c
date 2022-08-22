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

#include <SDL2/SDL_image.h>

#include "engine.h"
#include "animation.h"

/* Animation constructor */
kiavc_animation *kiavc_animation_create(const char *id, const char *path,
		int frames, SDL_Color *transparency) {
	if(!id || !path || frames < 1)
		return NULL;
	kiavc_animation *anim = SDL_calloc(1, sizeof(kiavc_animation));
	anim->id = SDL_strdup(id);
	anim->path = SDL_strdup(path);
	anim->frames = frames;
	if(transparency) {
		anim->transparency = 1;
		anim->t_r = transparency->r;
		anim->t_g = transparency->g;
		anim->t_b = transparency->b;
	}
	return anim;
}

/* Animation image initialization */
int kiavc_animation_load(kiavc_animation *anim, SDL_Renderer *renderer) {
	if(!anim || !renderer)
		return -1;
	/* If we have a texture already, do nothing */
	if(anim->texture)
		return 0;
	/* Load the image */
	SDL_Surface *loaded = IMG_Load_RW(kiavc_engine_open_file(anim->path), 1);
	if(!loaded) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading image: %s\n", IMG_GetError());
		return -2;
	}
	if(anim->transparency)
		SDL_SetColorKey(loaded, SDL_TRUE, SDL_MapRGB(loaded->format, anim->t_r, anim->t_g, anim->t_b));
	anim->texture = SDL_CreateTextureFromSurface(renderer, loaded);
	anim->w = loaded->w/anim->frames;
	anim->h = loaded->h;
	SDL_FreeSurface(loaded);
	if(!anim->texture) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating texture: %s\n", SDL_GetError());
		kiavc_animation_unload(anim);
		return -3;
	}
	return 0;
}

/* Animation image de-initialization */
void kiavc_animation_unload(kiavc_animation *anim) {
	if(!anim)
		return;
	if(anim->texture)
		SDL_DestroyTexture(anim->texture);
	anim->texture = NULL;
	anim->w = 0;
	anim->h = 0;
}

/* Animation destructor */
void kiavc_animation_destroy(kiavc_animation *anim) {
	if(anim) {
		SDL_free(anim->id);
		SDL_free(anim->path);
		kiavc_animation_unload(anim);
		free(anim);
	}
}
