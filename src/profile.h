#ifndef PROFILE_H
#define PROFILE_H

#include "world.h"

typedef struct{
	Wtexture* texture;
	void* script;
	char* type;
	double width, height;
} Profile;

Profile* profile_create(void);

void profile_setType(Profile* profile, const char* type);

void profile_loadScript(Profile* profile, const char* filename);

void profile_destroy(Profile* profile);

#endif
