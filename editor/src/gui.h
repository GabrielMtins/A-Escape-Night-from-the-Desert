#ifndef GUI_H
#define GUI_H

#include <dui.h>
#include <dui_renderer.h>
#include "world.h"
#include "stack.h"

DUI_Rect gui_rect(int x, int y, int w, int h);

void gui_init(DUI_Context* context);

void gui_render(World* world, Stack* profile_stack, Stack* entity_stack);

void gui_quit(void);

#endif
