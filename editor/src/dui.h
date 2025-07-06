/*
	Copyright (C) 2023 Gabriel Martins
  
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
  	
	1. The origin of this software must not be misrepresented; you must not
   	claim that you wrote the original software. If you use this software
   	in a product, an acknowledgment in the product documentation would be
   	appreciated but is not required. 
	2. Altered source versions must be plainly marked as such, and must not be
   	misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

#ifndef DUI_H
#define DUI_H

#include <stdint.h>
#include <stddef.h>

#define MAX_SIZE_STRING 256

typedef struct{
	int x, y, w, h;
} DUI_Rect;

typedef struct{
	uint8_t r, g, b, a;
} DUI_Color;

typedef struct{
	uint8_t mouse_pressed;
	char character_typed;
	uint8_t pop_char;

	int mouse_xrel, mouse_yrel;
	int mouse_x, mouse_y;

	size_t num_windows;
	size_t focus;

	DUI_Rect* rect_focus;
	int rect_focus_has_title;

	char* current_textbox;
} DUI_Context;

typedef struct{
	DUI_Rect box;
	size_t num_widget_x;
	size_t num_widget_y;
	size_t widget_counter;

	DUI_Context* context;

	uint8_t focus_on_title_bar;
	uint8_t have_title_bar;
	size_t id;
} DUI_Window;

DUI_Context DUI_CreateContext(void);

DUI_Window DUI_CreateWindow(DUI_Context* context, DUI_Rect widget, size_t num_widget_x, size_t num_widget_y);

void DUI_UpdateContext(DUI_Context* context, int mouse_pressed, int mouse_x, int mouse_y);

void DUI_UpdateCharTyped(DUI_Context* context, char c);

void DUI_PopCharTyped(DUI_Context* context);

void DUI_PutWindow(DUI_Window* window, const char* title);

void DUI_PutChildWindow(DUI_Window* parent_window, DUI_Window* child_window);

void DUI_PutLabel(DUI_Window* window, const char* text);

int DUI_PutButton(DUI_Window* window, const char* text);

void DUI_PutSliderHorizontal(DUI_Window* window, double* value);

void DUI_PutSliderVertical(DUI_Window* window, double* value);

void DUI_PutTexture(DUI_Window* window, void* texture, double* x, double* y, int* mouse_pressed);

void DUI_PutTextureAspectRatio(DUI_Window* window, void* texture, double aspect_ratio, double* x, double* y, int* mouse_pressed);

int DUI_PutListVertical(DUI_Window* window, const char** list, size_t list_size);

int DUI_PutListHorizontal(DUI_Window* window, const char** list, size_t list_size);

int DUI_PutListVerticalWSlider(DUI_Window* window, const char** list, size_t list_size, int* offset);

void DUI_PutTextBox(DUI_Window* window, char* typed_string, size_t max_len);

void DUI_Space(DUI_Window* window, const DUI_Color* color);

#endif
