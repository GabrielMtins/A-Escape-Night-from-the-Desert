#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Stack* stack_createWithSize(size_t memb_size){
	Stack* stack = malloc(sizeof(Stack));

	stack->memb_size = memb_size;
	stack->stack = NULL;
	stack->size = 0;

	return stack;
}

void stack_push(Stack* stack, void* member){
	if(stack == NULL || member == NULL) return;

	stack->size++;
	stack->stack = realloc(stack->stack, stack->size * stack->memb_size);

	memcpy(
			stack->stack + (stack->size - 1) * stack->memb_size,
			member,
			stack->memb_size
			);
}

void stack_pop(Stack* stack){
	if(stack == NULL) return;
	if(stack->size == 0) return;

	stack->size--;

	if(stack->size == 0){
		free(stack->stack);
		stack->stack = NULL;
	}
	else stack->stack = realloc(stack->stack, stack->size * stack->memb_size);
}

void* stack_get(Stack* stack, size_t index){
	if(stack == NULL) return NULL;
	if(index >= stack->size) return NULL;

	return stack->stack + index * stack->memb_size;
}

void stack_clean(Stack* stack){
	if(stack == NULL) return;
	if(stack->size == 0) return;

	stack->size = 0;

	free(stack->stack);
	stack->stack = NULL;
}

void stack_remove(Stack* stack, size_t index){
	if(stack == NULL) return;
	if(index >= stack->size) return;

	if(stack->size != index + 1)
		memmove(
				stack->stack + index * stack->memb_size,
				stack->stack + (index + 1) * stack->memb_size,
				(stack->size - index - 1) * stack->memb_size
		  	);

	stack->size--;

	if(stack->size == 0){
		free(stack->stack);
		stack->stack = NULL;
	}
	else stack->stack = realloc(stack->stack, stack->size * stack->memb_size);
}

void* stack_top(Stack* stack){
	if(stack == NULL) return NULL;
	if(stack->size == 0) return NULL;

	return stack_get(stack, stack->size - 1);
}

void stack_destroy(Stack* stack){
	free(stack->stack);
	free(stack);
}
