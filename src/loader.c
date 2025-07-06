#include "loader.h"
#include "entity.h"
#include "profile.h"
#include "sfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct{
	Entity** entity_list;
	Profile** profile_list;
	int num_list;
} EntityList;

static const char* prefix_texture = "texture_id_";
static const char* prefix_profile = "profile_id_";

static int l_setTile(lua_State* L);
static int l_loadTexture(lua_State* L);
static int l_addProfile(lua_State* L);
static int l_addEntity(lua_State* L);
static int l_setFogColor(lua_State* L);
static int l_setFogDistance(lua_State* L);

static int l_loadMusic(lua_State* L);
static int l_loadChunk(lua_State* L);

static int l_setTile(lua_State* L){
	World* world = lua_touserdata(L, 1);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	double bottom_height = luaL_checknumber(L, 4);
	double top_height = luaL_checknumber(L, 5);
	int top_id = luaL_checkinteger(L, 6);
	int top_wall_id = luaL_checkinteger(L, 7);
	int bottom_id = luaL_checkinteger(L, 8);
	int bottom_wall_id = luaL_checkinteger(L, 9);

	if(x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE){
		fprintf(stderr,"Loader: x or y are out of bounds\n");
		return -1;
	}

	world->world[x][y] = tile_create(
			bottom_height,
			top_height,
			top_id,
			top_wall_id,
			bottom_id,
			bottom_wall_id
			);

	return 0;
}

static int l_loadTexture(lua_State* L){
	World* world = lua_touserdata(L, 1);
	const char* filename = luaL_checkstring(L, 2);
	const char* name = luaL_checkstring(L, 3);

	world->texture_list[world->num_texture] = wtexture_load(filename, name);
	world->num_texture++;

	return 0;
}

static int l_addProfile(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	Stack* profile_stack = lua_touserdata(L, 1);
	Profile* profile_top = profile_create();
	stack_push(profile_stack, &profile_top);

	lua_getfield(L, 2, "type");

	const char* type = lua_tolstring(L, -1, NULL);
	profile_setType(profile_top, type);

	lua_getfield(L, 2, "texture");
	const char* filename = lua_tolstring(L, -1, NULL);
	if(filename != NULL)
		profile_top->texture = wtexture_load(filename, NULL);

	lua_getfield(L, 2, "width");
	if(lua_isnumber(L, -1))
		profile_top->width = lua_tonumber(L, -1);

	lua_getfield(L, 2, "height");
	if(lua_isnumber(L, -1))
		profile_top->height = lua_tonumber(L, -1);

	lua_getfield(L, 2, "cell_width");
	if(lua_isnumber(L, -1))
		profile_top->texture->cell_width = lua_tonumber(L, -1);

	lua_getfield(L, 2, "cell_height");
	if(lua_isnumber(L, -1))
		profile_top->texture->cell_height = lua_tonumber(L, -1);

	lua_getfield(L, 2, "script");
	if(lua_isstring(L, -1))
		profile_loadScript(profile_top, lua_tostring(L, -1));

	/* pop number of elements */
	lua_pop(L, 7);

	return 0;
}

static int l_addEntity(lua_State* L){
	if(lua_gettop(L) < 4) return -1;
	Stack* entity_stack = lua_touserdata(L, 1);
	Stack* profile_stack = lua_touserdata(L, 2);

	int index = lua_tonumber(L, 4);

	Profile** current_profile = stack_get(profile_stack, index);

	Entity* entity_top = entity_create(*current_profile);

	lua_getfield(L, 3, "x");
	entity_top->position.x = luaL_checknumber(L, -1);
	lua_getfield(L, 3, "y");
	entity_top->position.y = luaL_checknumber(L, -1);
	lua_getfield(L, 3, "z");
	entity_top->position.z = luaL_checknumber(L, -1);

	/* pop number of elements */
	lua_pop(L, 3);

	stack_push(entity_stack, &entity_top);

	return 0;
}

static int l_setFogColor(lua_State* L){
	if(lua_gettop(L) < 4) return -1;

	World* world = lua_touserdata(L, 1);
	Wcolor fog = {
		luaL_checknumber(L, 2),
		luaL_checknumber(L, 3),
		luaL_checknumber(L, 4),
		255
	};

	world->fog = fog;

	return 0;
}

static int l_setFogDistance(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	World* world = lua_touserdata(L, 1);
	world->fog_distance = luaL_checknumber(L, 2);

	return 0;
}

static int l_loadMusic(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	Stack* sfx_stack = lua_touserdata(L, 1);
	Sfx* sfx = sfx_load(lua_tostring(L, 2), SFX_MUSIC); 

	stack_push(sfx_stack, &sfx);

	return 0;
}

static int l_loadChunk(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	Stack* sfx_stack = lua_touserdata(L, 1);
	Sfx* sfx = sfx_load(lua_tostring(L, 2), SFX_CHUNK); 

	stack_push(sfx_stack, &sfx);

	return 0;
}

void loader_create(Stack** entity_stack){
	*entity_stack = stack_create(Entity*);
}

void loader_free(Stack* profile_stack, Stack* entity_stack, Stack* sfx_stack){
	for(size_t i = 0; i < profile_stack->size; i++){
		Profile** profile = stack_get(profile_stack, i);

		profile_destroy(*profile);
	}

	for(size_t i = 0; i < entity_stack->size; i++){
		Entity** entity = stack_get(entity_stack, i);

		entity_destroy(*entity);
	}

	for(size_t i = 0; i < sfx_stack->size; i++){
		Sfx** sfx = stack_get(sfx_stack, i);

		sfx_destroy(*sfx);
	}

	stack_destroy(profile_stack);
	stack_destroy(entity_stack);
}

void loader_loadWorldTiles(World* world, Stack* profile_stack, Stack* entity_stack, const char* filename){
	if(world == NULL) return;
	world_clean(world);
	stack_clean(entity_stack);

	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushnumber(L, -1);
	lua_setglobal(L, "texture_id_skybox");

	/* adding ids to textures */
	for(int i = 0; i < world->num_texture; i++){
		char* name = world->texture_list[i]->name;

		size_t type_prefix_size = strlen(name) + strlen(prefix_texture) + 1;
		char* type_prefix = malloc(type_prefix_size);

		strcpy(type_prefix, prefix_texture);
		strcat(type_prefix, name);

		lua_pushnumber(L, i);
		lua_setglobal(L, type_prefix);

		free(type_prefix);

	}

	/* adding ids to profiles */
	for(size_t i = 0; i < profile_stack->size; i++){
		Profile** current_profile = stack_get(profile_stack, i);

		size_t type_prefix_size = strlen((*current_profile)->type) + strlen(prefix_profile) + 1;
		char* type_prefix = malloc(type_prefix_size);
		strcpy(type_prefix, prefix_profile);
		strcat(type_prefix, (*current_profile)->type);

		lua_pushnumber(L, i);
		lua_setglobal(L, type_prefix);

		free(type_prefix);

		/* now reseting the variable g_player */
		if((*current_profile)->script != NULL){
			lua_pushnil((*current_profile)->script);
			lua_setglobal((*current_profile)->script, "g_player");
		}
	}

	lua_pushcfunction(L, l_setTile);
	lua_setglobal(L, "setTile");

	lua_pushcfunction(L, l_addEntity);
	lua_setglobal(L, "addEntity");

	lua_pushcfunction(L, l_setFogColor);
	lua_setglobal(L, "setFogColor");

	lua_pushcfunction(L, l_setFogDistance);
	lua_setglobal(L, "setFogDistance");

	if(luaL_dofile(L, filename) != LUA_OK)
		fprintf(stderr, "Loader: Failed to load: %s\n", filename);

	lua_getglobal(L, "OnLoadTiles");
	lua_pushlightuserdata(L, world);
	lua_pushlightuserdata(L, profile_stack);
	lua_pushlightuserdata(L, entity_stack);

	if(lua_pcall(L, 3, 1, 0) != LUA_OK)
		fprintf(stderr, "%s\n", lua_tostring(L, -1));

	lua_close(L);
}

void loader_saveWorldTiles(World* world, Stack* entity_stack, const char* filename){
	if(world == NULL) return;

	FILE* file = fopen(filename, "w+");
	freopen(NULL, "w+", file);

	fprintf(file, "function OnLoadTiles(world, profile_stack, entity_stack)\n");

	/* saving fog values */
	fprintf(file, "\tsetFogColor(world, %u, %u, %u)\n",
			world->fog.r, world->fog.g, world->fog.b);

	fprintf(file, "\tsetFogDistance(world, %lf)\n",
			world->fog_distance);

	for(size_t i = 0; i < entity_stack->size; i++){
		Entity* current_entity = *((Entity**) stack_get(entity_stack, i));

		fprintf(file, "\taddEntity(entity_stack, profile_stack, {x = %lf, y = %lf, z = %lf}, profile_id_%s)\n",
				current_entity->position.x,
				current_entity->position.y,
				current_entity->position.z,
				current_entity->profile->type
			   );
	}

	for(int i = 0; i < WORLD_SIZE; i++){
		for(int j = 0; j < WORLD_SIZE; j++){
			fprintf(file, "\tsetTile(world, %i, %i, %0.4lf, %0.4lf, "
				"texture_id_%s, texture_id_%s, texture_id_%s, texture_id_%s)\n",
					i, j,
					world_getBottomHeight(world, i, j),
					world_getTopHeight(world, i, j),
					world_getTextureName(world, world_getTopId(world, i, j)),
					world_getTextureName(world, world_getTopWallId(world, i, j)),
					world_getTextureName(world, world_getBottomId(world, i, j)),
					world_getTextureName(world, world_getBottomWallId(world, i, j))
				   );
		}
	}
	fprintf(file, "end\n");

	fclose(file);
}

void loader_loadWorldTextures(World* world, const char* filename){
	if(world == NULL) return;

	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L, l_loadTexture);
	lua_setglobal(L, "loadTexture");

	if(luaL_dofile(L, filename) != LUA_OK)
		fprintf(stderr, "Loader: Failed to load: %s\n", filename);

	lua_getglobal(L, "OnLoadTextures");
	lua_pushlightuserdata(L, world);

	if(lua_pcall(L, 1, 1, 0) != LUA_OK)
		fprintf(stderr, "Loader: Failed to call OnLoadTextures function from file %s\n", filename);

	lua_close(L);
}

Stack* loader_loadProfiles(const char* filename){
	Stack* profile_stack = stack_create(Profile*);

	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L, l_addProfile);
	lua_setglobal(L, "addProfile");

	if(luaL_dofile(L, filename) != LUA_OK)
		fprintf(stderr, "Loader: Failed to load: %s\n", filename);

	lua_getglobal(L, "OnLoadProfiles");
	lua_pushlightuserdata(L, profile_stack);

	if(lua_pcall(L, 1, 1, 0) != LUA_OK){
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
	}

	lua_close(L);

	return profile_stack;
}

Stack* loader_loadSfx(const char* filename){
	Stack* sfx_stack = stack_create(Sfx*);

	lua_State* L;
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L, l_loadMusic);
	lua_setglobal(L, "loadMusic");

	lua_pushcfunction(L, l_loadChunk);
	lua_setglobal(L, "loadChunk");

	if(luaL_dofile(L, filename) != LUA_OK)
		fprintf(stderr, "Loader: Failed to load: %s\n", filename);

	lua_getglobal(L, "OnLoadSfx");
	lua_pushlightuserdata(L, sfx_stack);

	if(lua_pcall(L, 1, 1, 0) != LUA_OK){
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
	}

	lua_close(L);

	return sfx_stack;
}
