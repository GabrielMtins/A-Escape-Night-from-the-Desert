#ifndef PROFILE_VIEWER_H
#define PROFILE_VIEWER_H

#include <dui.h>
#include "stack.h"

void profileviewer_init(DUI_Context* context);

void profileviewer_render(Stack* profile_stack);

void profileviewer_quit(void);

#endif
