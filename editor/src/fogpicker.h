#ifndef FOGPICKER_H
#define FOGPICKER_H

#include <dui.h>
#include "world.h"

void fogpicker_init(DUI_Context* context);

void fogpicker_render(World* world);

void fogpicker_quit(void);

#endif
