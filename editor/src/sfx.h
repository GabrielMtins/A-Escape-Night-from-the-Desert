#ifndef SFX_H
#define SFX_H

#include <stdint.h>

enum SFX_TYPE{
	SFX_CHUNK = 0,
	SFX_MUSIC
};

typedef struct{
	uint8_t type;
	void* audio;
} Sfx;

Sfx* sfx_load(const char* filename, uint8_t type);

void sfx_play(Sfx* sfx);

void sfx_destroy(Sfx* sfx);

#endif
