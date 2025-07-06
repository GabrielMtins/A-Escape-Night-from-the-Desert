#include "context.h"

Lcontext logic_createContext(Stack* entity_stack, Stack* adder_stack, Stack* profile_stack, Stack* sfx_stack, World* world){
	Lcontext context = {
		entity_stack,
		adder_stack,
		profile_stack,
		NULL,
		sfx_stack,
		world,
		0
	};

	return context;
}

void logic_setContextElapsedTime(Lcontext* context, double delta_time){
	context->delta_time = delta_time;
}

void logic_setContextSearchStack(Lcontext* context, Stack* search_stack){
	context->search_stack = search_stack;
}
