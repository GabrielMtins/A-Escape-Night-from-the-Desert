#ifndef TILEED_H
#define TILEED_H

#include <dui.h>

#include "world.h"

void tileed_init(DUI_Context* context);

void tileed_render(World* world);

void tileed_quit(void);

#endif
