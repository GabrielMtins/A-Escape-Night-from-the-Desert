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

#include "dui.h"
#include "dui_renderer.h"

#include <string.h>

#ifndef max
#define max(x, y) (x > y ? x : y)
#endif

#ifndef min
#define min(x, y) (x < y ? x : y)
#endif

#define MINIMUM(x) (x = x < 0.001 ? 0 : x)
#define MAXIMUM(x) (x = x > 0.999 ? 1 : x)
#define CHECK_ERROR(window) \
	if(window->widget_counter >= window->num_widget_x * window->num_widget_y && \
			(window->num_widget_x != 0  && window->num_widget_y != 0)) \
		return 

static const int text_padding = 2;
static const int padding = 8;
static const int title_bar_size = 24;
static const int slider_width = 10;
static const int focus_size = 2;

static const DUI_Color button_color = {0x88, 0x88, 0x88, 0xFF};
static const DUI_Color button_pressed_color = {0x30, 0x30, 0x30, 0xFF};
static const DUI_Color highlight_color = {0xAA, 0xAA, 0xAA, 0xFF};
static const DUI_Color text_color = {0xFF, 0xFF, 0xFF, 0xFF};
static const DUI_Color window_bg_color = {0x44, 0x44, 0x44, 0xFF};
static const DUI_Color window_fg_color = {0x11, 0x11, 0x11, 0xFF};

static void DUI_SetContextFocus(DUI_Window* window);
static int DUI_IsMouseInsideRect(DUI_Window* window, DUI_Rect rect);
static int DUI_IsWindowOnFocus(DUI_Window* window);
static int DUI_IsMouseClickingOnWidget(DUI_Window* window, DUI_Rect rect);
static int DUI_IsMouseClickingOnWindow(DUI_Window* window);
static void DUI_GetMouseRelativeToRect(DUI_Window* window, DUI_Rect rect, double* x, double* y);
static void DUI_RenderTextOnRectCenter(DUI_Rect rect, const char* text);
static void DUI_RenderTextOnRectLeft(DUI_Rect rect, const char* text);
static DUI_Rect DUI_GetWidgetRect(DUI_Window* window);

static void DUI_SetContextFocus(DUI_Window* window){
	window->context->focus = window->id;
	window->context->rect_focus = &window->box;
	window->context->rect_focus_has_title = window->have_title_bar;
}

static int DUI_IsMouseInsideRect(DUI_Window* window, DUI_Rect rect){
	if(window == NULL) return 0;

	if(window->context->mouse_x < rect.x || window->context->mouse_x >= rect.x + rect.w ||
			window->context->mouse_y < rect.y || window->context->mouse_y >= rect.y + rect.h)
		return 0;

	return 1;
}

static int DUI_IsWindowOnFocus(DUI_Window* window){
	return window->id == window->context->focus;
}

static int DUI_IsMouseClickingOnWidget(DUI_Window* window, DUI_Rect rect){
	return
		window->context->mouse_pressed
		&& DUI_IsMouseInsideRect(window, rect)
		&& DUI_IsWindowOnFocus(window)
		;
}

static int DUI_IsMouseClickingOnWindow(DUI_Window* window){
	DUI_Rect window_rect = {
		window->box.x,
		window->box.y - window->have_title_bar * title_bar_size,
		window->box.w,
		window->box.h + window->have_title_bar * title_bar_size
	};

	int is_inside_window = 
		window->context->mouse_pressed
		&& DUI_IsMouseInsideRect(window, window_rect);

	if(window->context->rect_focus == NULL)
		return is_inside_window;

	DUI_Rect focus_rect = {
		window->context->rect_focus->x,
		window->context->rect_focus->y - window->context->rect_focus_has_title * title_bar_size,
		window->context->rect_focus->w,
		window->context->rect_focus->h + window->context->rect_focus_has_title * title_bar_size
	};

	return is_inside_window
		&& !DUI_IsMouseInsideRect(window, focus_rect)
		;
}

static void DUI_GetMouseRelativeToRect(DUI_Window* window, DUI_Rect rect, double* x, double* y){
	if(x != NULL) *x = (double) (window->context->mouse_x - rect.x) / rect.w;
	if(y != NULL) *y = (double) (window->context->mouse_y - rect.y) / rect.h;
}

static void DUI_RenderTextOnRectCenter(DUI_Rect rect, const char* text){
	if(text == NULL) return;

	dui_renderer_setClipRect(rect);

	int text_width = dui_renderer_getTextWidth(text);
	int text_height = dui_renderer_getTextHeight();

	dui_renderer_drawText(
			text,
			rect.x + rect.w/2 - text_width/2,
			rect.y + rect.h/2 - text_height/2,
			text_color
			);
}

static void DUI_RenderTextOnRectLeft(DUI_Rect rect, const char* text){
	if(text == NULL) return;

	dui_renderer_setClipRect(rect);

	int text_height = dui_renderer_getTextHeight();

	dui_renderer_drawText(
			text,
			rect.x + padding,
			rect.y + rect.h/2 - text_height/2,
			text_color
			);
}

static DUI_Rect DUI_GetWidgetRect(DUI_Window* window){
	if(window->num_widget_x == 0 || window->num_widget_y == 0)
		return window->box;

	int x_pos = window->widget_counter % window->num_widget_x;
	int y_pos = window->widget_counter / window->num_widget_x;

	int widget_width = (window->box.w - (window->num_widget_x + 1) * padding) / 
							window->num_widget_x;

	int widget_height = (window->box.h - (window->num_widget_y + 1) * padding) / 
							window->num_widget_y;

	DUI_Rect rect = {
		padding + window->box.x + (padding + widget_width) * x_pos,
		padding + window->box.y + (padding + widget_height) * y_pos,
		widget_width, widget_height
	};

	return rect;
}

DUI_Context DUI_CreateContext(void){
	DUI_Context context;
	context.num_windows = 0;
	context.character_typed = '\0';
	context.focus = 999999;

	context.rect_focus = NULL;
	context.current_textbox = NULL;

	return context;
}

DUI_Window DUI_CreateWindow(DUI_Context* context, DUI_Rect widget, size_t num_widget_x, size_t num_widget_y){
	DUI_Window window;

	window.box = widget;
	window.num_widget_x = num_widget_x;
	window.num_widget_y = num_widget_y;
	window.widget_counter = 0;
	window.id = context->num_windows;

	window.context = context;

	context->num_windows++;

	window.focus_on_title_bar = 0;
	window.have_title_bar = 1;

	return window;
}

void DUI_UpdateContext(DUI_Context* context, int mouse_pressed, int mouse_x, int mouse_y){
	if(context == NULL) return;

	context->mouse_pressed = mouse_pressed;

	context->mouse_xrel = mouse_x - context->mouse_x;
	context->mouse_yrel = mouse_y - context->mouse_y;

	context->mouse_x = mouse_x;
	context->mouse_y = mouse_y;
}

void DUI_UpdateCharTyped(DUI_Context* context, char c){
	if(context->current_textbox == NULL) return;

	context->character_typed = c;
}

void DUI_PopCharTyped(DUI_Context* context){
	if(context->current_textbox == NULL) return;

	context->pop_char = 1;
}

void DUI_PutWindow(DUI_Window* window, const char* title){
	if(window == NULL) return;

	window->widget_counter = 0;

	if(DUI_IsMouseClickingOnWindow(window))
		DUI_SetContextFocus(window);

	if(DUI_IsWindowOnFocus(window)) dui_renderer_setRenderTop();
	else dui_renderer_setRenderBottom();

	if(window->have_title_bar){
		DUI_Rect title_bar = window->box;
	
		title_bar.y -= title_bar_size;
		title_bar.h = title_bar_size;

		if(window->context->mouse_pressed && DUI_IsMouseInsideRect(window, title_bar)){
			window->focus_on_title_bar = 1;
		}

		if(!window->context->mouse_pressed)
			window->focus_on_title_bar = 0;

		/* handling input */
		if(window->focus_on_title_bar && DUI_IsWindowOnFocus(window)){
			window->box.x += window->context->mouse_xrel;
			window->box.y += window->context->mouse_yrel;
			title_bar.x += window->context->mouse_xrel;
			title_bar.y += window->context->mouse_yrel;
		}

		if(DUI_IsWindowOnFocus(window)){
			title_bar.x -= focus_size/2;
			title_bar.y -= focus_size/2;
			title_bar.w += focus_size;
			title_bar.h += focus_size;
		}

		dui_renderer_drawRect(title_bar, window_fg_color);
		DUI_RenderTextOnRectCenter(title_bar, title);
	}

	if(DUI_IsWindowOnFocus(window)){
		DUI_Rect outline_rect = window->box;
		outline_rect.x -= focus_size/2;
		outline_rect.y -= focus_size/2;
		outline_rect.w += focus_size;
		outline_rect.h += focus_size;
		dui_renderer_drawRect(outline_rect, window_fg_color);
	}

	dui_renderer_drawRect(window->box, window_bg_color);
}

void DUI_PutChildWindow(DUI_Window* parent_window, DUI_Window* child_window){
	if(parent_window == NULL || child_window == NULL) return;
	CHECK_ERROR(parent_window);

	child_window->id = parent_window->id;
	child_window->box = DUI_GetWidgetRect(parent_window);
	child_window->widget_counter = 0;

	dui_renderer_drawRect(child_window->box, window_bg_color);

	parent_window->widget_counter++;
}

void DUI_PutLabel(DUI_Window* window, const char* text){
	if(window == NULL) return;
	CHECK_ERROR(window);

	DUI_Rect rect = DUI_GetWidgetRect(window);

	/* rendering */
	DUI_RenderTextOnRectCenter(rect, text);

	window->widget_counter++;
}

int DUI_PutButton(DUI_Window* window, const char* text){
	if(window == NULL) return 0;
	CHECK_ERROR(window) 0;

	int was_pressed = 0;

	DUI_Rect rect = DUI_GetWidgetRect(window);

	/* rendering */
	DUI_Color rendering_color = button_color;
	if(DUI_IsWindowOnFocus(window) && DUI_IsMouseInsideRect(window, rect)){
		rendering_color = highlight_color;

		if(window->context->mouse_pressed){
			rendering_color = button_pressed_color;
			was_pressed = 1;
		}
	}

	dui_renderer_drawRect(rect, rendering_color);

	DUI_RenderTextOnRectCenter(rect, text);

	window->widget_counter++;

	return was_pressed;
}

void DUI_PutSliderHorizontal(DUI_Window* window, double* value){
	if(window == NULL) return;
	if(value == NULL) return;
	CHECK_ERROR(window);

	DUI_Rect rect = DUI_GetWidgetRect(window);

	DUI_Rect slider = rect;
	slider.w = slider_width;

	slider.x = (*value) * (rect.w - slider.w) + rect.x;

	if(DUI_IsMouseClickingOnWidget(window, rect)){
		DUI_GetMouseRelativeToRect(window, rect, value, NULL);
		MINIMUM(*value);
		MAXIMUM(*value);
		slider.x = (*value) * (rect.w - slider.w) + rect.x;
	}

	dui_renderer_drawRect(rect, window_fg_color);
	dui_renderer_drawRect(slider, text_color);

	window->widget_counter++;
}

void DUI_PutSliderVertical(DUI_Window* window, double* value){
	if(window == NULL) return;
	if(value == NULL) return;
	CHECK_ERROR(window);

	DUI_Rect rect = DUI_GetWidgetRect(window);

	DUI_Rect slider = rect;
	slider.h = slider_width;

	slider.y = (*value) * (rect.h - slider.h) + rect.y;

	if(DUI_IsMouseClickingOnWidget(window, rect)){
		DUI_GetMouseRelativeToRect(window, rect, NULL, value);
		MINIMUM(*value);
		MAXIMUM(*value);
		slider.y = (*value) * (rect.h - slider.h) + rect.y;
	}

	dui_renderer_drawRect(rect, window_fg_color);
	dui_renderer_drawRect(slider, text_color);

	window->widget_counter++;
}

void DUI_PutTexture(DUI_Window* window, void* texture, double* x, double* y, int* mouse_pressed){
	if(window == NULL) return;
	CHECK_ERROR(window);

	DUI_Rect rect = DUI_GetWidgetRect(window);

	if(mouse_pressed != NULL) *mouse_pressed = 0;

	DUI_GetMouseRelativeToRect(window, rect, x, y);

	if(DUI_IsMouseClickingOnWidget(window, rect)){
		if(mouse_pressed != NULL) *mouse_pressed = 1;
	}

	dui_renderer_drawTexture(texture, rect);

	window->widget_counter++;
}

void DUI_PutTextureAspectRatio(DUI_Window* window, void* texture, double aspect_ratio, double* x, double* y, int* mouse_pressed){
	if(window == NULL) return;
	CHECK_ERROR(window);

	DUI_Rect rect = DUI_GetWidgetRect(window);

	if(rect.h * aspect_ratio < rect.w){
		int center_x = rect.x + rect.w / 2;
		rect.w = rect.h * aspect_ratio;
		rect.x = center_x - rect.w / 2;
	}
	else{
		int center_y = rect.y + rect.h / 2;
		rect.h = rect.w / aspect_ratio;
		rect.y = center_y - rect.h / 2;
	}

	if(mouse_pressed != NULL) *mouse_pressed = 0;

	DUI_GetMouseRelativeToRect(window, rect, x, y);
	if(DUI_IsMouseClickingOnWidget(window, rect)){

		if(mouse_pressed != NULL) *mouse_pressed = 1;
	}

	dui_renderer_drawTexture(texture, rect);

	window->widget_counter++;
}

int DUI_PutListVertical(DUI_Window* window, const char** list, size_t list_size){
	if(window == NULL) return -1;
	CHECK_ERROR(window) -1;
	if(list_size == 0) return -1;

	DUI_Rect rect = DUI_GetWidgetRect(window);

	DUI_Rect text_rect = rect;
	text_rect.h = dui_renderer_getTextHeight() + text_padding * 2;
	int clicked = -1;

	for(size_t i = 0; i < list_size; i++){
		if(text_rect.y + text_rect.h > rect.y + rect.h){
			clicked = -2;
			break;
		}

		DUI_Color color = button_color;

		if(DUI_IsMouseInsideRect(window, text_rect) && DUI_IsWindowOnFocus(window)){
			color = highlight_color;

			if(window->context->mouse_pressed){
				color = button_pressed_color;
				clicked = i;
			}
		}

		dui_renderer_drawRect(text_rect, color);
		DUI_RenderTextOnRectLeft(text_rect, list[i]);

		text_rect.y += text_rect.h;
	}

	window->widget_counter++;

	return clicked;
}

int DUI_PutListHorizontal(DUI_Window* window, const char** list, size_t list_size){
	if(window == NULL) return -1;
	CHECK_ERROR(window) -1;
	if(list_size == 0) return -1;

	DUI_Rect rect = DUI_GetWidgetRect(window);
	DUI_Rect text_rect = rect;
	int clicked = -1;

	for(size_t i = 0; i < list_size; i++){
		text_rect.w = dui_renderer_getTextWidth(list[i]) * 2;

		if(text_rect.x + text_rect.w > rect.x + rect.w){
			clicked = -2;
			break;
		}

		DUI_Color color = button_color;

		if(DUI_IsMouseInsideRect(window, text_rect) && DUI_IsWindowOnFocus(window)){
			color = highlight_color;

			if(window->context->mouse_pressed){
				color = button_pressed_color;
				clicked = i;
			}
		}

		dui_renderer_drawRect(text_rect, color);
		DUI_RenderTextOnRectCenter(text_rect, list[i]);

		text_rect.x += text_rect.w;
	}

	window->widget_counter++;

	return clicked;
}

int DUI_PutListVerticalWSlider(DUI_Window* window, const char** list, size_t list_size, int* offset){
	if(window == NULL) return -1;
	CHECK_ERROR(window) -1;
	if(list_size == 0) return -1;

	DUI_Rect rect = DUI_GetWidgetRect(window);

	DUI_Rect text_rect = rect;
	DUI_Rect slider_rect = text_rect;

	text_rect.x += text_padding * 12;
	text_rect.w -= text_padding * 12;

	text_rect.h = dui_renderer_getTextHeight() + text_padding * 2;

	slider_rect.w = text_padding * 10;
	slider_rect.x = (window->box.x + text_rect.x - slider_rect.w) / 2;
	int clicked = -1;
	size_t max_text = rect.h / text_rect.h;
	dui_renderer_drawRect(slider_rect, window_fg_color);

	/* handling mouse input for slider */
	if(list_size > max_text){
		double value = 0;

		if(DUI_IsMouseClickingOnWidget(window, slider_rect)){
			DUI_GetMouseRelativeToRect(window, slider_rect, NULL, &value);
			*offset = value * (max(1 + list_size - max_text, 0));
		}

		slider_rect.y = (double) (*offset) / (max(1 + list_size - max_text, 0)) * (slider_rect.h) + slider_rect.y;

		slider_rect.h = slider_width;
		dui_renderer_drawRect(slider_rect, text_color);
	}

	for(size_t i = 0; i < min(max_text, list_size); i++){
		DUI_Color color = button_color;

		if(DUI_IsMouseInsideRect(window, text_rect) && DUI_IsWindowOnFocus(window)){
			color = highlight_color;

			if(window->context->mouse_pressed){
				color = button_pressed_color;
				clicked = i + *offset;
			}
		}

		dui_renderer_drawRect(text_rect, color);
		DUI_RenderTextOnRectLeft(text_rect, list[i + *offset]);

		text_rect.y += text_rect.h;
	}

	window->widget_counter++;

	return clicked;
}

void DUI_PutTextBox(DUI_Window* window, char* typed_string, size_t max_len){
	if(window == NULL) return;
	CHECK_ERROR(window);

	if(window->context->current_textbox == typed_string){
		if(strlen(typed_string) < max_len && window->context->character_typed != '\0'){
			strncat(typed_string, &window->context->character_typed, 1);
			window->context->character_typed = '\0';
		}

		if(window->context->pop_char && strlen(typed_string) > 0){
			typed_string[strlen(typed_string) - 1] = '\0';
			window->context->pop_char = 0;
		}
	}

	if(DUI_PutButton(window, typed_string))
		window->context->current_textbox = typed_string;
}

void DUI_Space(DUI_Window* window, const DUI_Color* color){
	if(window == NULL) return;
	CHECK_ERROR(window);

	if(color != NULL){
		DUI_Rect rect = DUI_GetWidgetRect(window);
		dui_renderer_drawRect(rect, *color);
	}

	window->widget_counter++;
}
