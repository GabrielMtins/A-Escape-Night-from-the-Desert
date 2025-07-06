#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>

typedef struct{
	uint8_t* stack;
	size_t size;
	size_t memb_size;
} Stack;

#define stack_create(type) stack_createWithSize(sizeof(type))

Stack* stack_createWithSize(size_t memb_size);

void stack_push(Stack* stack, void* member);

void stack_pop(Stack* stack);

void* stack_get(Stack* stack, size_t index);

void stack_clean(Stack* stack);

void stack_remove(Stack* stack, size_t index);

void* stack_top(Stack* stack);

void stack_destroy(Stack* stack);

#endif
