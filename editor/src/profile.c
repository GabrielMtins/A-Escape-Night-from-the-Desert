#include "profile.h"
#include "logic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

Profile* profile_create(void){
	Profile* profile = malloc(sizeof(Profile));
	profile->type = NULL;
	profile->texture = NULL;
	profile->width = 0;
	profile->height = 0;
	profile->script = NULL;
	 
	return profile;
}

void profile_setType(Profile* profile, const char* type){
	if(profile == NULL) return;
	if(profile->type != NULL) free(profile->type);

	profile->type = malloc(strlen(type) + 1);
	memcpy(profile->type, type, strlen(type) + 1);
}

void profile_loadScript(Profile* profile, const char* filename){
	if(profile == NULL) return;
	if(profile->script != NULL) lua_close(profile->script);

	profile->script = luaL_newstate();
	luaL_openlibs(profile->script);

	if(luaL_dofile(profile->script, filename) != LUA_OK){
		fprintf(stderr, "Profile: Failed to script for profile: %s. Script: %s\n", profile->type, filename);
		fprintf(stderr, "Error: %s\n", lua_tostring(profile->script, -1));
	}

	logic_setScriptingAPI(profile);
}

void profile_destroy(Profile* profile){
	if(profile == NULL) return;
	if(profile->type != NULL) free(profile->type);
	if(profile->texture != NULL) wtexture_destroy(profile->texture);
	if(profile->script != NULL) lua_close(profile->script);

	free(profile);
}
