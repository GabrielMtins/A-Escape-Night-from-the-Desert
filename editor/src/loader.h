#ifndef LOADER_H
#define LOADER_H

#include "world.h"
#include "profile.h"
#include "stack.h"

void loader_create(Stack** entity_stack);

void loader_free(Stack* profile_stack, Stack* entity_stack, Stack* sfx_stack);

void loader_loadWorldTiles(World* world, Stack* profile_stack, Stack* entity_stack, const char* filename);

void loader_saveWorldTiles(World* world, Stack* entity_stack, const char* filename);

void loader_loadWorldTextures(World* world, const char* filename);

Stack* loader_loadProfiles(const char* filename);

Stack* loader_loadSfx(const char* filename);

#endif
