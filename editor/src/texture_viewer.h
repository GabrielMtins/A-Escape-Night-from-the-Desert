#ifndef TEXTURE_VIEWER_H
#define TEXTURE_VIEWER_H

#include <dui.h>

#include "world.h"

void textureviewer_init(DUI_Context* context);

void textureviewer_render(World* world);

void textureviewer_quit(void);

#endif
