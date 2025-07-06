#include "world.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define OUTOFBOUNDS(x, y) \
{ \
	if(x < 0) x = 0; \
	if(y < 0) y = 0; \
	if(x >= WORLD_SIZE) x = WORLD_SIZE - 1; \
	if(y >= WORLD_SIZE) y = WORLD_SIZE - 1; \
}
//(x < 0 || y < 0 || x >= WORLD_SIZE || y >= WORLD_SIZE)

Tile tile_create(double bottom_height, double top_height, int top_id, int top_wall_id, int bottom_id, int bottom_wall_id){
	Tile tile;

	tile.bottom_height = bottom_height;
	tile.top_height = top_height;
	tile.bottom_wall_id = bottom_wall_id;
	tile.bottom_id = bottom_id;
	tile.offset_texture = 0;
	tile.top_wall_id = top_wall_id;
	tile.top_id = top_id;

	return tile;
}

Wtexture* wtexture_load(const char* filename, const char* name){
	if(filename == NULL) return NULL;

	SDL_Surface* surface = IMG_Load(filename);

	if(surface == NULL) return NULL;

	Wtexture* texture = malloc(sizeof(Wtexture));

	texture->width = surface->w;
	texture->height = surface->h;

	texture->cell_width = surface->w;
	texture->cell_height = surface->h;
	texture->texture = malloc(texture->width * texture->height * sizeof(Wcolor));

	if(name != NULL){
		texture->name = malloc(strlen(name) + 1);
		strcpy(texture->name, name);
	}
	else texture->name = NULL;

	for(size_t i = 0; i < texture->width; i++){
		for(size_t j = 0; j < texture->height; j++){
			uint32_t* pixels = surface->pixels;
			Wcolor color;

			SDL_GetRGBA(
					pixels[i + j * texture->width],
					surface->format,
					&color.r,
					&color.g,
					&color.b,
					&color.a
					);

			texture->texture[i + j * texture->width] = color;
		}
	}

	SDL_FreeSurface(surface);

	return texture;
}

void wtexture_setCellSize(Wtexture* texture, size_t cell_width, size_t cell_height){
	if(texture == NULL) return;

	texture->cell_width = cell_width;
	texture->cell_height = cell_height;
}

void wtexture_destroy(Wtexture* texture){
	if(texture == NULL) return;
	if(texture->texture != NULL) free(texture->texture);
	if(texture->name != NULL) free(texture->name);

	free(texture);
}

Wcolor wtexture_getPixel(const Wtexture* texture, double x, double y){
	Wcolor black = {0, 0, 0, 0};
	y = 1 - y;

	if(texture == NULL) return black;
	if(x < 0 || x > 1 || y < 0 || y > 1) return black;

	if(y >= 1) y = 0.99;
	if(x >= 1) x = 0.99;

	size_t x_pixel = x * texture->width;
	size_t y_pixel = y * texture->height;
	size_t index = x_pixel + y_pixel * texture->width;

	return texture->texture[index];
}

Wcolor wtexture_getPixelFromCell(const Wtexture* texture, double x, double y, size_t cell_x, size_t cell_y){
	Wcolor black = {0, 0, 0, 0};
	y = 1 - y;

	if(texture == NULL) return black;
	if(x < 0 || x > 1 || y < 0 || y > 1) return black;

	if(y >= 1) y = 0.99;
	if(x >= 1) x = 0.99;

	size_t x_pixel = (x + cell_x) * texture->cell_width;
	size_t y_pixel = (y + cell_y) * texture->cell_height;

	if(x_pixel >= texture->width) x_pixel %= texture->width;
	if(y_pixel >= texture->height) y_pixel %= texture->height;

	size_t index = x_pixel + y_pixel * texture->width;

	return texture->texture[index];
}

World* world_create(void){
	World* world = malloc(sizeof(World));

	world->num_texture = 0;
	world->prop_stack = stack_create(Wprop);

	world_clean(world);

	return world;
}

void world_clean(World* world){
	world->fog.r = 112;
	world->fog.g = 181;
	world->fog.b = 204;
	world->fog.a = 255;
	world->fog_distance = 32;

	for(int i = 0; i < WORLD_SIZE; i++){
		for(int j = 0; j < WORLD_SIZE; j++){
			world->world[i][j] = tile_create(0, 6, 0, 1, 0, 1);

			if(i == 0 || j == 0 || i == WORLD_SIZE-1 || j == WORLD_SIZE-1){
				world->world[i][j] = tile_create(7, 100, -1, -1, 0, 1);
			}
		}
	}
}

void world_addProp(World* world, Wprop prop){
	if(world == NULL) return;

	stack_push(world->prop_stack, &prop);
}

void world_clearProp(World* world){
	if(world == NULL) return;

	stack_clean(world->prop_stack);
}

const char* world_getTextureName(const World* world, int id){
	if(id == -1) return "skybox";
	else return world->texture_list[id]->name;
}

Wcolor world_getPixelTexture(const World* world, int id, double x, double y){
	if(world == NULL) return world->fog;
	if(id < 0 || id >= world->num_texture) return world->fog;
	if(x < 0 || y < 0 || x >= 1 || y >= 1) return world->fog;

	return wtexture_getPixel(world->texture_list[id], x, y);
}

double world_getTopHeight(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].top_height;
}

double world_getBottomHeight(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].bottom_height;
}

int world_getTopId(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].top_id;
}

int world_getTopWallId(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].top_wall_id;
}

int world_getBottomId(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].bottom_id;
}

int world_getBottomWallId(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].bottom_wall_id;
}

double world_getOffsetTexture(const World* world, int x, int y){
	if(world == NULL) return -1;
	OUTOFBOUNDS(x, y);

	return world->world[x][y].offset_texture;
}

void world_destroy(World* world){
	if(world == NULL) return;

	for(int i = 0; i < world->num_texture; i++)
		wtexture_destroy(world->texture_list[i]);

	stack_destroy(world->prop_stack);

	free(world);
}
