#include "logic.h"
#include "sfx.h"
#include "etable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define logic_tostr(name) #name

#define logic_addKey(L, key) lua_pushnumber(L, SDL_SCANCODE_##key); lua_setglobal(L, logic_tostr(KEY_##key))

/* setting functions */
static int l_entity_setPosition(lua_State* L);
static int l_entity_getPosition(lua_State* L);
static int l_entity_setVelocity(lua_State* L);
static int l_entity_getVelocity(lua_State* L);
static int l_entity_setAcceleration(lua_State* L);
static int l_entity_getAcceleration(lua_State* L);
static int l_entity_setMaxVelocityAbs(lua_State* L);
static int l_entity_setMaxVelocity(lua_State* L);
static int l_entity_setGroundAcceleration(lua_State* L);
static int l_entity_setAirAcceleration(lua_State* L);
static int l_entity_setFriction(lua_State* L);
static int l_entity_setGravity(lua_State* L);
static int l_entity_setDirection(lua_State* L);
static int l_entity_getDirection(lua_State* L);
static int l_entity_getProperties(lua_State* L);
static int l_entity_setProperties(lua_State* L);
static int l_entity_setDirectionToEntity(lua_State* L);
static int l_entity_getType(lua_State* L);
static int l_entity_playAnimation(lua_State* L);
static int l_entity_hasCollidedWithWorld(lua_State* L);
static int l_entity_solveCollisionWithEntity(lua_State* L);
static int l_entity_isLookingAtEntity(lua_State* L);
static int l_entity_getDistanceSqrToEntity(lua_State* L);

/* input */
static int l_input_getKeyDown(lua_State* L);
static int l_input_getMouseDown(lua_State* L);

/* collision and working with entities */
static int l_entity_getCollisionWithType(lua_State* L);
static int l_entity_getEntityCollided(lua_State* L);
static int l_entity_testLineOfSight(lua_State* L);
static int l_entity_kill(lua_State* L);

/* making movement */
static int l_entity_jump(lua_State* L);
static int l_entity_setRelativeAcceleration(lua_State* L);

/* context functions */
static int l_context_addEntity(lua_State* L);
static int l_context_searchEntityType(lua_State* L);
static int l_context_getSearchedEntity(lua_State* L);
static int l_context_sortByDistanceToParent(lua_State* L);
static int l_context_getElapsedTime(lua_State* L);
static int l_context_playSfx(lua_State* L);
static int l_context_isMusicPaused(lua_State* L);
static int l_context_pauseMusic(lua_State* L);
static int l_context_resumeMusic(lua_State* L);
static int l_context_stopMusic(lua_State* L);
static int l_context_setWorldTile(lua_State* L);
static int l_context_getWorldTile(lua_State* L);

/* logic own functions */
static void logic_pushKeys(lua_State* L);
static void logic_pushConstants(lua_State* L);

static int l_entity_setPosition(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Entity* entity = lua_touserdata(L, 1);

	lua_getfield(L, 2, "x");
	lua_getfield(L, 2, "y");
	lua_getfield(L, 2, "z");

	entity->position = vec3_create(
		luaL_checknumber(L, -3),
		luaL_checknumber(L, -2),
		luaL_checknumber(L, -1)
	);

	/* num of args */
	lua_pop(L, 3);

	return 0;
}

static int l_entity_getPosition(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_newtable(L);

	lua_pushnumber(L, entity->position.x);
	lua_setfield(L, -2, "x");

	lua_pushnumber(L, entity->position.y);
	lua_setfield(L, -2, "y");

	lua_pushnumber(L, entity->position.z);
	lua_setfield(L, -2, "z");

	return 1;
}

static int l_entity_setVelocity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_getfield(L, 2, "x");
	lua_getfield(L, 2, "y");
	lua_getfield(L, 2, "z");

	entity->velocity = vec3_create(
		luaL_checknumber(L, -3),
		luaL_checknumber(L, -2),
		luaL_checknumber(L, -1)
	);

	/* num of args */
	lua_pop(L, 3);

	return 0;
}

static int l_entity_getVelocity(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_newtable(L);

	lua_pushnumber(L, entity->velocity.x);
	lua_setfield(L, -2, "x");

	lua_pushnumber(L, entity->velocity.y);
	lua_setfield(L, -2, "y");

	lua_pushnumber(L, entity->velocity.z);
	lua_setfield(L, -2, "z");

	return 1;
}

static int l_entity_setAcceleration(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_getfield(L, 2, "x");
	lua_getfield(L, 2, "y");
	lua_getfield(L, 2, "z");

	entity->acceleration = vec3_create(
		luaL_checknumber(L, -3),
		luaL_checknumber(L, -2),
		luaL_checknumber(L, -1)
	);

	/* num of args */
	lua_pop(L, 3);

	return 0;
}

static int l_entity_getAcceleration(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_newtable(L);

	lua_pushnumber(L, entity->acceleration.x);
	lua_setfield(L, -2, "x");

	lua_pushnumber(L, entity->acceleration.y);
	lua_setfield(L, -2, "y");

	lua_pushnumber(L, entity->acceleration.z);
	lua_setfield(L, -2, "z");

	return 1;
}

static int l_entity_setMaxVelocityAbs(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->max_velocity_abs = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_setMaxVelocity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->max_velocity = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_setGroundAcceleration(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->ground_acceleration = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_setAirAcceleration(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->air_acceleration = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_setFriction(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->friction = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_setGravity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->gravity = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_setDirection(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->direction = luaL_checknumber(L, 2);

	return 0;
}

static int l_entity_getDirection(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);
	lua_pushnumber(L, entity->direction);

	return 1;
}

static int l_entity_getProperties(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	if(entity->properties == NULL)
		lua_newtable(L);
	else
		etable_writeToScript(entity->properties, L);

	return 1;
}

static int l_entity_setProperties(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);
	
	if(!lua_istable(L, 2)) return -1;

	etable_destroy(entity->properties);

	entity->properties = etable_readFromScript(L, 2);

	return 0;
}

static int l_entity_setDirectionToEntity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Entity* entity_a = lua_touserdata(L, 1);
	Entity* entity_b = lua_touserdata(L, 2);

	entity_a->direction = atan2(
			entity_b->position.x - entity_a->position.x,
			entity_b->position.y - entity_a->position.y
			);

	return 0;
}

static int l_entity_getType(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_pushstring(L, entity->profile->type);

	return 1;
}

static int l_entity_playAnimation(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);
	entity->current_animation.fps = luaL_checknumber(L, 3);
	size_t num_frames = lua_rawlen(L, 2);

	stack_clean(entity->current_animation.frames);

	for(size_t i = 1; i <= num_frames; i++){
		lua_geti(L, 2, i);
		size_t cell = lua_tonumber(L, -1);
		stack_push(entity->current_animation.frames, &cell);

		lua_pop(L, 1);
	}

	return 1;
}

static int l_entity_hasCollidedWithWorld(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	lua_pushnumber(L, entity->has_collided_with_world);

	return 1;
}

static int l_entity_solveCollisionWithEntity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 2, "userdata");
		return -1;
	}

	Entity* entity = lua_touserdata(L, 1);
	Entity* fixer = lua_touserdata(L, 2);

	entity_solveCollisionForEntities(entity, fixer);

	return 0;
}

static int l_entity_isLookingAtEntity(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 2, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 3)){
		luaL_typeerror(L, 3, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);
	Entity* viewer = lua_touserdata(L, 2);
	Entity* enemy = lua_touserdata(L, 3);

	int is_looking_at = entity_isLookingAt(viewer, enemy, context->world);
	lua_pushboolean(L, is_looking_at);

	return 1;
}

static int l_entity_getDistanceSqrToEntity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 2, "userdata");
		return -1;
	}

	Entity* viewer = lua_touserdata(L, 1);
	Entity* enemy = lua_touserdata(L, 2);

	lua_pushnumber(L, vec3_sizeSqr(vec3_sub(viewer->position, enemy->position)));

	return 1;
}

static int l_input_getKeyDown(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	const uint8_t* keys = SDL_GetKeyboardState(NULL);
	int number = luaL_checknumber(L, 1);

	if(keys[number])
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);


	return 1;
}

static int l_input_getMouseDown(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	int mouse_state = SDL_GetMouseState(NULL, NULL);

	const char* type = luaL_checkstring(L, 1);

	if(!strcmp(type, "left"))
		lua_pushboolean(L, mouse_state & SDL_BUTTON_LMASK);
	else if(!strcmp(type, "right"))
		lua_pushboolean(L, mouse_state & SDL_BUTTON_RMASK);
	else
		lua_pushboolean(L, 0);

	return 1;
}

static int l_entity_getCollisionWithType(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);
	Entity* entity = lua_touserdata(L, 2);
	Stack* type_stack = stack_create(char*);
	
	for(int i = 3; i <= lua_gettop(L); i++){
		const char* current_type = lua_tostring(L, i);
		stack_push(type_stack, &current_type);
	}

	stack_clean(entity->working_stack);
	entity_findCollisionOnStack(entity, context->entity_stack, type_stack);

	stack_destroy(type_stack);

	if(entity->working_stack->size > 0)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	return 1;
}

static int l_entity_getEntityCollided(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	if(entity->working_stack->size == 0)
		lua_pushnil(L);
	else{
		Entity* collided = *((Entity**) stack_top(entity->working_stack));
		lua_pushlightuserdata(L, collided);
		stack_pop(entity->working_stack);
	}

	return 1;
}

static int l_entity_testLineOfSight(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 2, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 3)){
		luaL_typeerror(L, 3, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);
	Entity* viewer = lua_touserdata(L, 2);
	Entity* enemy = lua_touserdata(L, 3);

	if(entity_testLineOfSight(viewer, enemy, context->world))
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	return 1;
}

static int l_entity_kill(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);

	entity->kill = 1;

	return 0;
}

static int l_entity_jump(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);
	double jump_velocity = luaL_checknumber(L, 2);

	entity_jump(entity, jump_velocity);

	return 0;
}

static int l_entity_setRelativeAcceleration(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	Entity* entity = lua_touserdata(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);

	entity_setRelativeAcceleration(entity, x, y);

	return 0;
}

static int l_context_addEntity(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Entity* new_entity = NULL;
	Profile* new_profile = NULL;
	Lcontext* context = lua_touserdata(L, 1);
	const char* type = lua_tostring(L, 2);

	for(size_t i = 0; i < context->profile_stack->size; i++){
		Profile* current_profile = *((Profile**) stack_get(context->profile_stack, i));
		if(strcmp(current_profile->type, type)) continue;

		new_profile = current_profile;
	}

	if(new_profile == NULL) return -1;

	new_entity = entity_create(new_profile);

	stack_push(context->adder_stack, &new_entity);

	lua_pushlightuserdata(L, new_entity);

	return 1;
}

static int l_context_searchEntityType(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);
	Stack* type_stack = stack_create(char*);

	stack_clean(context->search_stack);
	
	for(int i = 2; i <= lua_gettop(L); i++){
		const char* current_type = lua_tostring(L, i);
		stack_push(type_stack, &current_type);
	}

	entity_searchForEntity(context->search_stack, context->entity_stack, type_stack);

	stack_destroy(type_stack);

	if(context->search_stack->size > 0)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	return 1;
}

static int l_context_getSearchedEntity(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);

	if(context->search_stack->size == 0) lua_pushnil(L);
	else{
		Entity* current_entity = *((Entity**) stack_top(context->search_stack));
		lua_pushlightuserdata(L, current_entity);
		stack_pop(context->search_stack);
	}

	return 1;
}

static int l_context_sortByDistanceToParent(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_isuserdata(L, 2)){
		luaL_typeerror(L, 2, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);
	Entity* parent = lua_touserdata(L, 2);

	entity_sortByDistanceFromParent(context->search_stack, parent);

	return 0;
}

static int l_context_getElapsedTime(lua_State* L){
	if(lua_gettop(L) < 1) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);

	lua_pushnumber(L, context->delta_time);

	return 1;
}

static int l_context_playSfx(lua_State* L){
	if(lua_gettop(L) < 2) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);
	size_t id = luaL_checknumber(L, 2);

	Sfx* sfx = *((Sfx**) stack_get(context->sfx_stack, id));

	sfx_play(sfx);

	return 0;
}

static int l_context_isMusicPaused(lua_State* L){
	if(lua_gettop(L) != 0) return -1;

	lua_pushboolean(L, Mix_PausedMusic() == 1);

	return 1;
}

static int l_context_pauseMusic(lua_State* L){
	if(lua_gettop(L) != 0) return -1;

	Mix_PauseMusic();

	return 0;
}

static int l_context_resumeMusic(lua_State* L){
	if(lua_gettop(L) != 0) return -1;

	Mix_ResumeMusic();

	return 0;
}

static int l_context_stopMusic(lua_State* L){
	if(lua_gettop(L) != 0) return -1;

	Mix_HaltMusic();

	return 0;
}

static int l_context_setWorldTile(lua_State* L){
	if(lua_gettop(L) < 4) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}
	if(!lua_istable(L, 4)){
		luaL_typeerror(L, 4, "table");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);

	int x = luaL_checknumber(L, 2);
	int y = luaL_checknumber(L, 3);

	if(x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE) return -1;

	Tile tile;

	lua_getfield(L, 4, "top_id");
	tile.top_id = luaL_checknumber(L, -1);
	lua_getfield(L, 4, "bottom_id");
	tile.bottom_id = luaL_checknumber(L, -1);
	lua_getfield(L, 4, "top_wall_id");
	tile.top_wall_id = luaL_checknumber(L, -1);
	lua_getfield(L, 4, "bottom_wall_id");
	tile.bottom_wall_id = luaL_checknumber(L, -1);
	lua_getfield(L, 4, "bottom_height");
	tile.bottom_height = luaL_checknumber(L, -1);
	lua_getfield(L, 4, "top_height");
	tile.top_height = luaL_checknumber(L, -1);
	lua_getfield(L, 4, "offset_texture");
	tile.offset_texture = luaL_checknumber(L, -1);

	context->world->world[x][y] = tile;

	lua_pop(L, 7);

	return 0;
}

static int l_context_getWorldTile(lua_State* L){
	if(lua_gettop(L) < 3) return -1;

	if(!lua_isuserdata(L, 1)){
		luaL_typeerror(L, 1, "userdata");
		return -1;
	}

	Lcontext* context = lua_touserdata(L, 1);

	int x = luaL_checknumber(L, 2);
	int y = luaL_checknumber(L, 3);

	if(x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE) return -1;

	lua_newtable(L);

	Tile tile = context->world->world[x][y];

	lua_pushnumber(L, tile.bottom_height);
	lua_setfield(L, -2, "bottom_height");

	lua_pushnumber(L, tile.top_height);
	lua_setfield(L, -2, "top_height");

	lua_pushnumber(L, tile.top_id);
	lua_setfield(L, -2, "top_id");

	lua_pushnumber(L, tile.bottom_id);
	lua_setfield(L, -2, "bottom_id");

	lua_pushnumber(L, tile.bottom_wall_id);
	lua_setfield(L, -2, "bottom_wall_id");

	lua_pushnumber(L, tile.top_wall_id);
	lua_setfield(L, -2, "top_wall_id");

	lua_pushnumber(L, tile.offset_texture);
	lua_setfield(L, -2, "offset_texture");

	return 1;
}

static void logic_pushKeys(lua_State* L){
	logic_addKey(L, A);
	logic_addKey(L, B);
	logic_addKey(L, C);
	logic_addKey(L, D);
	logic_addKey(L, E);
	logic_addKey(L, F);
	logic_addKey(L, G);
	logic_addKey(L, H);
	logic_addKey(L, I);
	logic_addKey(L, J);
	logic_addKey(L, K);
	logic_addKey(L, L);
	logic_addKey(L, M);
	logic_addKey(L, N);
	logic_addKey(L, O);
	logic_addKey(L, P);
	logic_addKey(L, Q);
	logic_addKey(L, R);
	logic_addKey(L, S);
	logic_addKey(L, T);
	logic_addKey(L, U);
	logic_addKey(L, V);
	logic_addKey(L, W);
	logic_addKey(L, X);
	logic_addKey(L, Y);
	logic_addKey(L, Z);
	logic_addKey(L, SPACE);

	logic_addKey(L, 1);
	logic_addKey(L, 2);
	logic_addKey(L, 3);
	logic_addKey(L, 4);
	logic_addKey(L, 5);
	logic_addKey(L, 6);
	logic_addKey(L, 7);
	logic_addKey(L, 8);
	logic_addKey(L, 9);
	logic_addKey(L, 0);
}

static void logic_pushConstants(lua_State* L){
	lua_pushnumber(L, X_AXIS);
	lua_setglobal(L, "X_AXIS");
	lua_pushnumber(L, Y_AXIS);
	lua_setglobal(L, "Y_AXIS");
	lua_pushnumber(L, NO_AXIS);
	lua_setglobal(L, "NO_AXIS");
}

void logic_setScriptingAPI(Profile* profile){
	if(profile->script == NULL) return;

	lua_State* L = profile->script;

	/* setting entities */
	logic_addFunction(L, entity_setPosition);
	logic_addFunction(L, entity_getPosition);
	logic_addFunction(L, entity_setVelocity);
	logic_addFunction(L, entity_getVelocity);
	logic_addFunction(L, entity_setAcceleration);
	logic_addFunction(L, entity_getAcceleration);
	logic_addFunction(L, entity_setAirAcceleration);
	logic_addFunction(L, entity_setAirAcceleration);
	logic_addFunction(L, entity_setGroundAcceleration);
	logic_addFunction(L, entity_setFriction);
	logic_addFunction(L, entity_setMaxVelocityAbs);
	logic_addFunction(L, entity_setMaxVelocity);
	logic_addFunction(L, entity_setGravity);
	logic_addFunction(L, entity_setDirection);
	logic_addFunction(L, entity_getDirection);
	logic_addFunction(L, entity_getProperties);
	logic_addFunction(L, entity_setProperties);
	logic_addFunction(L, entity_setDirectionToEntity);
	logic_addFunction(L, entity_getType);
	logic_addFunction(L, entity_playAnimation);
	logic_addFunction(L, entity_hasCollidedWithWorld);
	logic_addFunction(L, entity_solveCollisionWithEntity);
	logic_addFunction(L, entity_isLookingAtEntity);
	logic_addFunction(L, entity_getDistanceSqrToEntity);

	/* input */
	logic_addFunction(L, input_getKeyDown);
	logic_addFunction(L, input_getMouseDown);

	/* dealing with entities */
	logic_addFunction(L, entity_getCollisionWithType);
	logic_addFunction(L, entity_getEntityCollided);
	logic_addFunction(L, entity_testLineOfSight);
	logic_addFunction(L, entity_kill);
	
	/* context functions */
	logic_addFunction(L, context_addEntity);
	logic_addFunction(L, context_searchEntityType);
	logic_addFunction(L, context_getSearchedEntity);
	logic_addFunction(L, context_sortByDistanceToParent);
	logic_addFunction(L, context_getElapsedTime);
	logic_addFunction(L, context_playSfx);
	logic_addFunction(L, context_isMusicPaused);
	logic_addFunction(L, context_pauseMusic);
	logic_addFunction(L, context_resumeMusic);
	logic_addFunction(L, context_stopMusic);
	logic_addFunction(L, context_getWorldTile);
	logic_addFunction(L, context_setWorldTile);

	/* entity movement */
	logic_addFunction(L, entity_jump);
	logic_addFunction(L, entity_setRelativeAcceleration);

	logic_pushKeys(L);
	logic_pushConstants(L);
}

void logic_callCreateFunction(Entity* entity){
	if(entity->profile->script == NULL) return;

	lua_State* L = entity->profile->script;

	lua_getglobal(L, "OnCreate");
	lua_pushlightuserdata(L, entity);

	if(lua_pcall(L, 1, 1, 0) != LUA_OK)
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
}

void logic_callUpdateFunction(Entity* entity, Lcontext* context){
	if(entity->profile->script == NULL) return;

	lua_State* L = entity->profile->script;
	Stack* search_stack = stack_create(Entity*);

	logic_setContextSearchStack(context, search_stack);

	lua_getglobal(L, "OnUpdate");
	lua_pushlightuserdata(L, context);
	lua_pushlightuserdata(L, entity);

	if(lua_pcall(L, 2, 1, 0) != LUA_OK){
		fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
	}
	
	/* clean stack */
	lua_settop(L, 0);
	stack_destroy(search_stack);
}
