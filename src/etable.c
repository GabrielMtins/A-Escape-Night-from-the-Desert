#include "etable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

Property* property_create(const char* name, int index, uint8_t type, void* data){
	if(data == NULL) return NULL;

	Property* property = malloc(sizeof(Property));

	if(name != NULL){
		property->is_name_index = 0;
		property->name = malloc(strlen(name) + 1);
		strcpy(property->name, name);
	}
	else{
		property->name = NULL;
		property->is_name_index = 1;
		property->index = index;
	}

	property->type = type;

	switch(type){
		case TYPE_USERDATA:
			property->data.d_userdata = data;
		break;

		case TYPE_NUMBER:
		case TYPE_BOOLEAN:
			property->data.d_number = *((double*) data);
		break;

		case TYPE_STRING:
			property->data.d_string = malloc(strlen(data) + 1);
			strcpy(property->data.d_string, data);
		break;

		case TYPE_TABLE:
			property->data.d_table = data;
		break;
	}

	return property;
}

void property_destroy(Property* property){
	if(property == NULL) return;

	if(property->name != NULL)
		free(property->name);

	if(property->type == TYPE_STRING)
		free(property->data.d_string);
	
	if(property->type == TYPE_TABLE)
		etable_destroy(property->data.d_table);

	free(property);
}

Stack* etable_readFromScript(void* script, int table_index){
	if(script == NULL) return NULL;

	Stack* etable = stack_create(Property*);

	lua_State* L = script;

	/* searching for the elements */
	lua_pushnil(L);

	while(lua_next(L, table_index) != 0){
		int index = 0;
		const char* name = NULL;

		if(lua_type(L, -2) != LUA_TNUMBER){
			name = lua_tostring(L, -2);
		}
		else{
			index =  (int) lua_tonumber(L, -2);
		}

		Property* property = NULL;

		int type = lua_type(L, -1);
		double data;

		switch(type){
			case LUA_TNUMBER:
				data = lua_tonumber(L, -1);
				property = property_create(name, index, TYPE_NUMBER, &data);
			break;

			case LUA_TBOOLEAN:
				data = lua_toboolean(L, -1);
				property = property_create(name, index, TYPE_BOOLEAN, &data);
			break;

			case LUA_TSTRING:
				property = property_create(name, index, TYPE_STRING, (void*) lua_tostring(L, -1));
			break;

			case LUA_TUSERDATA:
			case LUA_TLIGHTUSERDATA:
				property = property_create(name, index, TYPE_USERDATA, lua_touserdata(L, -1));
			break;

			case LUA_TTABLE:
				property = property_create(name, index, TYPE_TABLE, etable_readFromScript(L, lua_gettop(L)));
			break;
		}

		if(property != NULL)
			stack_push(etable, &property);
		
		lua_pop(L, 1);
	}

	return etable;
}

void etable_writeToScript(Stack* etable, void* script){
	if(script == NULL) return;
	if(etable == NULL) return;

	lua_State* L = script;
	int table_index;

	lua_newtable(L);
	table_index = lua_gettop(L);

	for(size_t i = 0; i < etable->size; i++){
		Property* property = *((Property**) stack_get(etable, i));

		switch(property->type){
			case TYPE_STRING:
				lua_pushstring(L, property->data.d_string);
			break;

			case TYPE_USERDATA:
				lua_pushlightuserdata(L, property->data.d_userdata);
			break;

			case TYPE_NUMBER:
				lua_pushnumber(L, property->data.d_number);
			break;

			case TYPE_BOOLEAN:
				lua_pushboolean(L, property->data.d_number);
			break;

			case TYPE_TABLE:
				etable_writeToScript(property->data.d_table, L);
			break;
		}

		if(!property->is_name_index)
			lua_setfield(L, table_index, property->name);
		else
			lua_seti(L, table_index, property->index);
	}
}

void etable_destroy(Stack* etable){
	if(etable == NULL) return;

	for(size_t i = 0; i < etable->size; i++){
		Property** property = stack_get(etable, i);

		property_destroy(*property);
	}

	stack_destroy(etable);
}
