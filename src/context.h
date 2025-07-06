#ifndef CONTEXT_H
#define CONTEXT_H

#include "stack.h"
#include "world.h"

typedef struct{
	Stack* entity_stack;
	Stack* adder_stack;
	Stack* profile_stack;
	Stack* search_stack;
	Stack* sfx_stack;
	World* world;
	double delta_time;
} Lcontext;

Lcontext logic_createContext(Stack* entity_stack, Stack* adder_stack, Stack* profile_stack, Stack* sfx_stack, World* world);

void logic_setContextElapsedTime(Lcontext* context, double delta_time);

void logic_setContextSearchStack(Lcontext* context, Stack* search_stack);

#endif
