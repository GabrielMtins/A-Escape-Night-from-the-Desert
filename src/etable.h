#ifndef ETABLE_H
#define ETABLE_H

#include <stdint.h>
#include "stack.h"

/* this module dumps a lua table into memory, so that it can be used later */

enum PROPERTY_TYPE{
	TYPE_USERDATA = 0,
	TYPE_NUMBER,
	TYPE_BOOLEAN,
	TYPE_STRING,
	TYPE_TABLE,
};

typedef struct{
	char* name;
	int index;
	uint8_t is_name_index;
	uint8_t type;

	union{
		double d_number;
		char* d_string;
		void* d_userdata;
		void* d_table;
	} data;
} Property;

Property* property_create(const char* name, int index, uint8_t type, void* data);

void property_destroy(Property* property);

/* reads a table from the top of a stack */
Stack* etable_readFromScript(void* script, int table_index);

/* pushes a table into the top of a stack */
void etable_writeToScript(Stack* etable, void* script);

void etable_destroy(Stack* etable);

#endif
