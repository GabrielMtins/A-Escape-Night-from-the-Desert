#ifndef WORLD_H
#define WORLD_H

#define WORLD_SIZE 256
#define MAX_TEXTURE 256
#define MAX_PROP 512

#include <stddef.h>
#include <stdint.h>
#include "vec3.h"
#include "stack.h"

typedef struct{
	uint8_t r, g, b, a;
} Wcolor;

typedef struct{
	Wcolor* texture;
	char* name;
	size_t width;
	size_t height;
	size_t cell_width;
	size_t cell_height;
} Wtexture;

typedef struct{
	Vec3 position;
	double width, height;
	size_t cell;
	double direction;
	Wtexture* texture;
} Wprop;

typedef struct{
	double bottom_height;
	double top_height;
	double offset_texture;
	uint8_t id;
	int top_id;
	int top_wall_id;
	int bottom_id;
	int bottom_wall_id;
} Tile;

typedef struct{
	Tile world[WORLD_SIZE][WORLD_SIZE];
	Wtexture* texture_list[MAX_TEXTURE];
	Stack* prop_stack;
	int num_texture;
	Wcolor fog;
	double fog_distance;
} World;

Tile tile_create(double bottom_height, double top_height, int top_id, int top_wall_id, int bottom_id, int bottom_wall_id);

Wtexture* wtexture_load(const char* filename, const char* name);

void wtexture_setCellSize(Wtexture* texture, size_t cell_width, size_t cell_height);

void wtexture_destroy(Wtexture* texture);

/* both x and y need to be normalized */
Wcolor wtexture_getPixel(const Wtexture* texture, double x, double y);

Wcolor wtexture_getPixelFromCell(const Wtexture* texture, double x, double y, size_t cell_x, size_t cell_y);

World* world_create(void);

void world_clean(World* world);

void world_addProp(World* world, Wprop prop);

void world_clearProp(World* world);

const char* world_getTextureName(const World* world, int id);

Wcolor world_getPixelTexture(const World* world, int id, double x, double y);

double world_getTopHeight(const World* world, int x, int y);

double world_getBottomHeight(const World* world, int x, int y);

int world_getTopId(const World* world, int x, int y);

int world_getTopWallId(const World* world, int x, int y);

int world_getBottomId(const World* world, int x, int y);

int world_getBottomWallId(const World* world, int x, int y);

double world_getOffsetTexture(const World* world, int x, int y);

void world_destroy(World* world);

#endif
