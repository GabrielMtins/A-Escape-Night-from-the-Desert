#include "sfx.h"

#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_mixer.h>

Sfx* sfx_load(const char* filename, uint8_t type){
	if(filename == NULL) return NULL;

	Sfx* sfx = malloc(sizeof(Sfx));
	sfx->type = type;
	sfx->audio = NULL;

	switch(sfx->type){
		case SFX_CHUNK:
			sfx->audio = Mix_LoadWAV(filename);
		break;

		case SFX_MUSIC:
			sfx->audio = Mix_LoadMUS(filename);
		break;
	}

	if(sfx->audio == NULL) 
		fprintf(stderr, "Failed to load audio: %s\n", filename);

	return sfx;
}

void sfx_play(Sfx* sfx){
	if(sfx->type == SFX_CHUNK){
		Mix_PlayChannel(-1, sfx->audio, 0);
	}
	else if(sfx->type == SFX_MUSIC){
		Mix_PlayMusic(sfx->audio, -1);
	}
}

void sfx_destroy(Sfx* sfx){
	if(sfx == NULL) return;

	if(sfx->type == SFX_CHUNK)
		Mix_FreeChunk(sfx->audio);
	else
		Mix_FreeMusic(sfx->audio);
	
	free(sfx);
}
