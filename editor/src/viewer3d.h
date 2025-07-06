#ifndef VIEWER3D_H
#define VIEWER3D_H

#include "world.h"
#include "renderer.h"
#include <SDL2/SDL.h>

void viewer3d_init(void);

void viewer3d_input(World* world, Stack* profile_stack, Stack* entity_stack, double delta_time, int mouse_xrel);

void viewer3d_setCurrentTexture(int id);

void viewer3d_setCurrentProfile(int id);

int viewer3d_getCurrentTexture(void);

Vec3 viewer3d_getCurrentTile(void);

void viewer3d_render(Renderer3D* renderer, World* world, int paused);

void viewer3d_quit(void);

#endif
