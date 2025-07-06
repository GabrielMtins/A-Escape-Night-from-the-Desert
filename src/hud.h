#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>
#include "renderer.h"
#include "logic.h"

const char* hud_init(SDL_Renderer* renderer, int* n_pause);

void hud_setInternalRenderer(Renderer3D* renderer_3d);

void hud_render(Lcontext* context);

void hud_quit(void);

#endif
