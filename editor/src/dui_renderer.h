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

#ifndef DUI_RENDERER
#define DUI_RENDERER

#define USE_SDL

#include "dui.h"

void dui_renderer_init(void);

void* dui_renderer_getRenderer(void);

void dui_renderer_resize(void);

void dui_renderer_getSize(int* w, int* h);

void dui_renderer_setRenderTop(void);

void dui_renderer_setRenderBottom(void);

void dui_renderer_drawRect(DUI_Rect rect, DUI_Color color);

void dui_renderer_drawTexture(void* texture, DUI_Rect rect);

int dui_renderer_getTextWidth(const char* text);

int dui_renderer_getTextHeight(void);

void dui_renderer_drawText(const char* text, int pos_x, int pos_y, DUI_Color color);

void dui_renderer_setClipRect(DUI_Rect rect);

void dui_renderer_clear(DUI_Color color);

void dui_renderer_present(void);

void dui_renderer_end(void);

#endif
