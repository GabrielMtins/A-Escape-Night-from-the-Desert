#include "texture_viewer.h"
#include "viewer3d.h"
#include <SDL2/SDL.h>
#include <math.h>
#include <dui_renderer.h>

#define NUM_TEX 8
#define TEX_SIZE 256
#define TILE_SIZE (TEX_SIZE / NUM_TEX)

static SDL_Texture* textureviewer_texture;
static DUI_Window textureviewer_window;
static int viewer_offset = 0;

void textureviewer_init(DUI_Context* context){
	DUI_Rect window_rect = {100, 80, 400, 400};

	textureviewer_window = DUI_CreateWindow(
			context, window_rect,
			1, 1
			);

	textureviewer_texture = SDL_CreateTexture(
			dui_renderer_getRenderer(),
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			TEX_SIZE,
			TEX_SIZE
			);
}

void textureviewer_render(World* world){
	double x, y;
	int pressed;
	uint32_t pixels[TEX_SIZE * TEX_SIZE];

	for(int i = 0; i < TEX_SIZE; i++){
		for(int j = 0; j < TEX_SIZE; j++){
			int tile_x = i / TILE_SIZE;
			int tile_y = j / TILE_SIZE + viewer_offset;
			int text_index = tile_x + tile_y * NUM_TEX - 1;

			double text_x = fmod((double) i / TILE_SIZE, 1);
			double text_y = 1.0 - fmod((double) j / TILE_SIZE, 1);

			Wcolor color = world_getPixelTexture(world, text_index, text_x, text_y);
			if(color.a == 0) color.a = 255;

			if(text_index == viewer3d_getCurrentTexture()){
				if(i % TILE_SIZE == 0 || j % TILE_SIZE == 0 || (i+1) % TILE_SIZE == 0 || (j+1) % TILE_SIZE == 0)
					color.r = color.g = color.b = color.a = 255;
			}


			pixels[i + j * TEX_SIZE] = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
		}
	}

	SDL_UpdateTexture(textureviewer_texture, NULL, pixels, sizeof(uint32_t) * TEX_SIZE);

	DUI_PutWindow(&textureviewer_window, "Texture Viewer");
	DUI_PutTexture(&textureviewer_window, textureviewer_texture, &x, &y, &pressed);

	if(pressed){
		int tile_x = TEX_SIZE * x / TILE_SIZE;
		int tile_y = TEX_SIZE * y / TILE_SIZE + viewer_offset;

		viewer3d_setCurrentTexture(tile_x + tile_y * NUM_TEX - 1);
	}
}

void textureviewer_quit(void){
	SDL_DestroyTexture(textureviewer_texture);
}
