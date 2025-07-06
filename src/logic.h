#ifndef LOGIC_H
#define LOGIC_H

#include "entity.h"
#include "world.h"
#include "profile.h"
#include "context.h"

#define logic_addFunction(L, name) lua_pushcfunction(L, l_##name); lua_setglobal(L, #name)

void logic_setScriptingAPI(Profile* profile);

void logic_callCreateFunction(Entity* entity);

void logic_callUpdateFunction(Entity* entity, Lcontext* context);

#endif
