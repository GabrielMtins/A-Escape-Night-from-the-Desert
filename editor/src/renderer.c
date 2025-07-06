#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592

static void renderer_putPixel(Renderer3D* renderer, int x, int y, Wcolor color);
static void renderer_setDepthBuffer(Renderer3D* renderer, int x, int y, double depth);
static double renderer_getDepthBuffer(Renderer3D* renderer, int x, int y);
static Vec3 renderer_rotate(Vec3 dir, double cos_ang, double sin_ang);
static double renderer_getDepth(Vec3 camera, Vec3 wall, double cos_ang, double sin_ang);
static Vec3 renderer_nextClosestTile(Vec3 camera, Vec3 tile, Vec3 dir, int* hor_intersection);
static void renderer_renderWall(Renderer3D* renderer, int i, int id, double offset_texture, int start_px, int end_px);
static void renderer_renderFloor(Renderer3D* renderer, int i, int id, int start_wall, int end_wall, int start_px, int end_px);
static void renderer_renderSprites(Renderer3D* renderer, Vec3 camera, double direction, double cos_ang, double sin_ang);
static double renderer_projectHeight(Renderer3D* renderer, double height, double depth);

static void renderer_putPixel(Renderer3D* renderer, int x, int y, Wcolor color){
	y = renderer->height - y - 1;

	if(x < 0 || y < 0 || x >= renderer->width || y >= renderer->height) return;

	renderer->pixels[x + y * renderer->width] = 
		(color.r << 24) | (color.g << 16) | (color.b << 8) | (color.a);
}

static void renderer_setDepthBuffer(Renderer3D* renderer, int x, int y, double depth){
	y = renderer->height - y - 1;

	if(x < 0 || y < 0 || x >= renderer->width || y >= renderer->height) return;

	renderer->depth_buffer[x + y * renderer->width] = depth;
}

static double renderer_getDepthBuffer(Renderer3D* renderer, int x, int y){
	y = renderer->height - y - 1;

	if(x < 0 || y < 0 || x >= renderer->width || y >= renderer->height) return renderer->max_dist;

	return renderer->depth_buffer[x + y * renderer->width];
}

static Vec3 renderer_rotate(Vec3 dir, double cos_ang, double sin_ang){
	Vec3 new_dir = {
		dir.x * cos_ang - dir.y * sin_ang,
		dir.x * sin_ang + dir.y * cos_ang,
		dir.z
	};

	return new_dir;
}

static double renderer_getDepth(Vec3 camera, Vec3 wall, double cos_ang, double sin_ang){
	wall = vec3_sub(wall, camera);

	return -wall.x * sin_ang + wall.y * cos_ang;
}

static Vec3 renderer_nextClosestTile(Vec3 camera, Vec3 tile, Vec3 dir, int* hor_intersection){
	double step_x = dir.x > 0 ? 1 : -0.001;
	double step_y = dir.y > 0 ? 1 : -0.001;

	/*
	Vec3 tile_x = {10 * WORLD_SIZE+1, 10 * WORLD_SIZE+1, tile.z};
	Vec3 tile_y = {10 * WORLD_SIZE+1, 10 * WORLD_SIZE+1, tile.z};
	*/

	Vec3 tile_x, tile_y;

	if(dir.x != 0){
		tile_x.x = floor(tile.x) + step_x;
		tile_x.y = (tile_x.x - camera.x) * dir.y / dir.x + camera.y;
		tile_x.z = tile.z;
	}

	if(dir.y != 0){
		tile_y.y = floor(tile.y) + step_y;
		tile_y.x = (tile_y.y - camera.y) * dir.x / dir.y + camera.x;
		tile_y.z = tile.z;
	}

	if(dir.x == 0) return tile_y;
	if(dir.y == 0) return tile_x;

	if(vec3_sizeSqr(vec3_sub(tile_x, camera)) < vec3_sizeSqr(vec3_sub(tile_y, camera))){
		if(hor_intersection != NULL) *hor_intersection = 1;
		return tile_x;
	}
	else{
		if(hor_intersection != NULL) *hor_intersection = 0;
		return tile_y;
	}
}

static void renderer_renderWall(Renderer3D* renderer, int i, int id, double offset_texture, int start_px, int end_px){
	if(id < 0) return;

	double x_text = 0, y_text = 0;

	/* using this to correct mirrored walls */
	if(renderer->var.horizontal_intersection){
		if(renderer->var.dir.x < 0)
			x_text = (renderer->var.wall.y - floor(renderer->var.wall.y));
		else
			x_text = 1.0 - (renderer->var.wall.y - floor(renderer->var.wall.y));
	}
	else{
		if(renderer->var.dir.y > 0)
			x_text = (renderer->var.wall.x - floor(renderer->var.wall.x));
		else
			x_text = 1.0 - (renderer->var.wall.x - floor(renderer->var.wall.x));
	}

	for(int j = start_px; j < end_px; j++){
		if(j > renderer->height) break;

		y_text = ((double)((j - renderer->var.bottom_start_wall)) / renderer->var.max_height) + offset_texture;

		while(y_text < 0) y_text += 1;
		while(y_text >= 1) y_text -= 1;

		Wcolor color = world_getPixelTexture(renderer->world, id, x_text, y_text);
		color.a = renderer->var.alpha_start;

		renderer_putPixel(renderer, i, j, color);
		renderer_setDepthBuffer(renderer, i, j, renderer->var.first_depth);
	}
}

static void renderer_renderFloor(Renderer3D* renderer, int i, int id, int start_wall, int end_wall, int start_px, int end_px){
	if(id < 0) return;

	double inv_h1, inv_h2;

	if(renderer->var.first_depth == 0) inv_h1 = 0;
	else inv_h1 = 1.0 / (start_wall - renderer->height/2);

	inv_h2 = 1.0 / (end_wall - renderer->height/2);

	double inv_sum = 1.0 / (inv_h2 - inv_h1);
	double second_part = inv_h1 * inv_sum;

	/* drawing bottom floor */
	for(int j = start_px; j < end_px; j++){
		double scale = inv_sum / (j - renderer->height/2) - second_part;
		double depth = renderer->var.first_depth + (renderer->var.second_depth - renderer->var.first_depth) * scale;

		Vec3 tile = vec3_add(vec3_mul(vec3_sub(renderer->var.next_wall, renderer->var.wall), scale), renderer->var.wall);

		double x_text = fabs(tile.x - floor(tile.x));
		double y_text = fabs(tile.y - floor(tile.y));

		Wcolor color = world_getPixelTexture(renderer->world, id, x_text, y_text);
				
		color.a = renderer->var.alpha_start + (renderer->var.alpha_end - renderer->var.alpha_start) * scale;

		renderer_putPixel(renderer, i, j, color);
		renderer_setDepthBuffer(renderer, i, j, depth);
	}
}

static void renderer_renderSprites(Renderer3D* renderer, Vec3 camera, double direction, double cos_ang, double sin_ang){
	for(size_t i = 0; i < renderer->world->prop_stack->size; i++){
		Wprop* current_prop = stack_get(renderer->world->prop_stack, i);

		Vec3 character = current_prop->position;
		double w = current_prop->width;
		double h = current_prop->height;

		double abs_direction = current_prop->direction - direction;

		while(abs_direction < 0) abs_direction += 2 * PI;
		abs_direction = fmod(abs_direction, 2 * PI);

		int cell_y = 0;

		if(current_prop->texture->cell_height != current_prop->texture->height)
			cell_y = abs_direction / (2 * PI) * current_prop->texture->height / current_prop->texture->cell_height + 0.5;

		Wtexture* id = current_prop->texture;

		character = vec3_sub(character, camera);
		character = renderer_rotate(character, cos_ang, -sin_ang);

		if(character.y <= 0.1) continue;
		if(vec3_sizeSqr(character) >= renderer->max_dist_sqr) continue;

		int x = ((character.x / character.y) * renderer->height)/renderer->scale_width + renderer->width/2;
		int y = (character.z / character.y) * renderer->height + renderer->height/2;

		w = (double) renderer->height * w / character.y / renderer->scale_width;
		h = (double) renderer->height * h / character.y;

		for(int j = y; j < y + h; j++){
			if(j < 0 || j >= renderer->height) continue;
			for(int i = x - w/2; i < x + w/2; i++){
				if(i < 0 || i >= renderer->width) continue;

				if(character.y >= renderer_getDepthBuffer(renderer, i, j)) continue;

				double x_text = (double) (i - x - w/2) / w;
				double y_text = (double) (j - y) / h;
				x_text = fabs(x_text);
				y_text = fabs(y_text);

				x_text = 1.0 - x_text;

				Wcolor color = wtexture_getPixelFromCell(id, x_text, y_text, current_prop->cell, cell_y);

				if(color.a == 0) continue;
				renderer_setDepthBuffer(renderer, i, j, character.y);
				renderer_putPixel(renderer, i, j, color);
			}
		}
	}
}

Renderer3D* renderer_init(SDL_Renderer* renderer_sdl, World* world){
	Renderer3D* renderer = malloc(sizeof(Renderer3D));
	renderer->renderer = renderer_sdl;
	renderer->pixels = NULL;
	renderer->depth_buffer = NULL;
	renderer->texture = NULL;
	renderer->world = world;

	renderer_setRes(renderer, 1280, 720, 4);

	return renderer;
}

void renderer_setRes(Renderer3D* renderer, int width, int height, int scale_width){
	if(renderer->texture != NULL) SDL_DestroyTexture(renderer->texture);
	if(renderer->pixels != NULL) free(renderer->pixels);
	if(renderer->depth_buffer != NULL) free(renderer->depth_buffer);

	renderer->width = width / scale_width;
	renderer->height = height;
	renderer->scale_width = scale_width;

	renderer->pixels = malloc(renderer->width * renderer->height * sizeof(uint32_t));
	renderer->depth_buffer = malloc(renderer->width * renderer->height * sizeof(double));

	renderer->texture = SDL_CreateTexture(
			renderer->renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			renderer->width,
			renderer->height
			);

	SDL_SetTextureBlendMode(renderer->texture, SDL_BLENDMODE_BLEND);

}

static double renderer_projectHeight(Renderer3D* renderer, double height, double depth){
	return ((height) / depth + 0.5) * renderer->height;
}

void renderer_render(Renderer3D* renderer, Vec3 camera, double direction){
	renderer->max_dist = renderer->world->fog_distance;
	renderer->max_dist_sqr = renderer->max_dist * renderer->max_dist;

	for(int i = 0; i < renderer->width * renderer->height; i++){
		renderer->pixels[i] = 0;
		renderer->depth_buffer[i] = renderer->max_dist;
	}

	double cos_ang = cos(-direction);
	double sin_ang = sin(-direction);

	for(int i = 0; i < renderer->width; i++){
		Vec3 dir = {
			(double) (i - renderer->width/2) / renderer->height * renderer->scale_width,
			1,
			0
		};

		dir = renderer_rotate(dir, cos_ang, sin_ang);
		
		Vec3 wall = camera;

		int bottom_pixel = 0;
		int top_pixel = renderer->height;
		int horizontal_intersection = 0;

		while(1)
		{
			if(top_pixel <= bottom_pixel) break;
			if(vec3_sizeSqr(vec3_sub(wall, camera)) > renderer->max_dist_sqr) break;

			Vec3 next_wall = renderer_nextClosestTile(camera, wall, dir, NULL);

			int bottom_min_height, bottom_max_height, top_min_height, top_max_height;
			double bot, top, offset_texture;

			/* computing the alpha values for fog */
			renderer->var.alpha_start = 255 * (1.0 - vec3_sizeSqr(vec3_sub(wall, camera)) / renderer->max_dist_sqr);
			renderer->var.alpha_end = 255 * (1.0 - vec3_sizeSqr(vec3_sub(next_wall, camera)) / renderer->max_dist_sqr);

			if(renderer->var.alpha_start < 0) renderer->var.alpha_start = 0;
			if(renderer->var.alpha_start > 255) renderer->var.alpha_start = 255;

			if(renderer->var.alpha_end < 0) renderer->var.alpha_end = 0;
			if(renderer->var.alpha_end > 255) renderer->var.alpha_end = 255;

			/* getting the bottom and the top of each wall */
			bot = world_getBottomHeight(renderer->world, wall.x, wall.y);
			top = world_getTopHeight(renderer->world, wall.x, wall.y);
			offset_texture = world_getOffsetTexture(renderer->world, wall.x, wall.y);

			renderer->var.first_depth = renderer_getDepth(camera, wall, cos_ang, sin_ang);
			renderer->var.second_depth = renderer_getDepth(camera, next_wall, cos_ang, sin_ang);

			/* projecting to camera */
			if(renderer->var.first_depth != 0){
				bottom_min_height = renderer_projectHeight(renderer, bot - camera.z, renderer->var.first_depth);
				top_max_height = renderer_projectHeight(renderer, top - camera.z, renderer->var.first_depth);
				renderer->var.max_height = ((1.0) / renderer->var.first_depth) * renderer->height;
			}
			else{
				bottom_min_height = -1;
				top_max_height = renderer->height+1;
				renderer->var.max_height = renderer->height;
			}

			bottom_max_height = renderer_projectHeight(renderer, bot - camera.z, renderer->var.second_depth);
			top_min_height = renderer_projectHeight(renderer, top - camera.z, renderer->var.second_depth);

			if(renderer->var.first_depth == 0)
				renderer->var.bottom_start_wall = bottom_pixel;
			else 
				renderer->var.bottom_start_wall = 
					renderer_projectHeight(renderer, -camera.z + floor(camera.z), renderer->var.first_depth);

			renderer->var.wall = wall;
			renderer->var.next_wall = next_wall;
			renderer->var.horizontal_intersection = horizontal_intersection;
			renderer->var.dir = dir;

			/* drawing bottom wall */
			renderer_renderWall(
					renderer,
					i,
					world_getBottomWallId(renderer->world, wall.x, wall.y),
					offset_texture,
					bottom_pixel,
					fmin(bottom_min_height, top_pixel)
					);
			bottom_pixel = fmax(bottom_pixel, bottom_min_height);

			/* drawing top wall */
			renderer_renderWall(
					renderer,
					i,
					world_getTopWallId(renderer->world, wall.x, wall.y),
					offset_texture,
					fmax(top_max_height, bottom_pixel),
					top_pixel);

			top_pixel = fmin(top_pixel, top_max_height);

			/* drawing bottom floor */
			renderer_renderFloor(
					renderer,
					i,
					world_getBottomId(renderer->world, wall.x, wall.y),
					bottom_min_height,
					bottom_max_height,
					bottom_pixel,
					fmin(bottom_max_height, top_pixel)
					);
			bottom_pixel = fmax(bottom_pixel, bottom_max_height);

			/* drawing top ceiling */
			renderer_renderFloor(
					renderer,
					i,
					world_getTopId(renderer->world, wall.x, wall.y),
					top_max_height,
					top_min_height,
					fmax(bottom_pixel, top_min_height),
					top_pixel);

			top_pixel = fmin(top_pixel, top_min_height);

			wall = renderer_nextClosestTile(camera, wall, dir, &horizontal_intersection);
		}
	}

	renderer_renderSprites(renderer, camera, direction, cos_ang, sin_ang);

	SDL_UpdateTexture(renderer->texture, NULL, renderer->pixels, renderer->width * sizeof(uint32_t));
}

void renderer_destroy(Renderer3D* renderer){
	if(renderer->texture != NULL) SDL_DestroyTexture(renderer->texture);
	if(renderer->pixels != NULL) free(renderer->pixels);
	if(renderer->depth_buffer != NULL) free(renderer->depth_buffer);
	free(renderer);
	
}
