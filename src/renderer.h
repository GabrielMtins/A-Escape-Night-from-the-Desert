#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "vec3.h"
#include "world.h"

typedef struct{
	Vec3 wall, next_wall;
	Vec3 dir;
	int horizontal_intersection;
	int bottom_start_wall;
	double first_depth, second_depth;
	int alpha_start, alpha_end;
	int max_height;
} Render_Var;

typedef struct{
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	World* world;
	uint32_t* pixels;
	double* depth_buffer;
	int width, height;
	int scale_width;
	double max_dist;
	double max_dist_sqr;

	Render_Var var;
} Renderer3D;

Renderer3D* renderer_init(SDL_Renderer* renderer_sdl, World* world);

void renderer_setRes(Renderer3D* renderer, int width, int height, int scale_width);

void renderer_render(Renderer3D* renderer, Vec3 camera, double direction);

void renderer_destroy(Renderer3D* renderer);

#endif
