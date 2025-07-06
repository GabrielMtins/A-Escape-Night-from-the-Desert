/*
	Copyright (C) 2023 Gabriel Martins
  
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
  	
	1. The origin of this software must not be misrepresented; you must not
   	claim that you wrote the original software. If you use this software
   	in a product, an acknowledgment in the product documentation would be
   	appreciated but is not required. 
	2. Altered source versions must be plainly marked as such, and must not be
   	misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

#include "dui_renderer.h"

#ifdef USE_SDL

#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define FONT_SIZE 12

static SDL_Window* window;
static SDL_Renderer* renderer;
static TTF_Font* font;
static SDL_Texture* top_texture;
static SDL_Texture* bottom_texture;

static SDL_Rect clip_rect;

static SDL_Texture* dui_renderer_createTargetTexture(void);

static SDL_Texture* dui_renderer_createTargetTexture(void){
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	SDL_Texture* target = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_TARGET,
			w, h
			);

	SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);

	return target;
}

void dui_renderer_init(void){
	SDL_Init(SDL_INIT_VIDEO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();

	window = SDL_CreateWindow("window", 
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			1136, 640,
			SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	font = TTF_OpenFont("default.ttf", FONT_SIZE);

	if(font == NULL)
		fprintf(stderr, "Failed to load default.ttf\n");

	top_texture = dui_renderer_createTargetTexture();
	bottom_texture = dui_renderer_createTargetTexture();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void* dui_renderer_getRenderer(void){
	return renderer;
}

void dui_renderer_resize(void){
	if(top_texture != NULL) SDL_DestroyTexture(top_texture);
	if(bottom_texture != NULL) SDL_DestroyTexture(bottom_texture);

	top_texture = dui_renderer_createTargetTexture();
	bottom_texture = dui_renderer_createTargetTexture();
}

void dui_renderer_getSize(int* w, int* h){
	SDL_GetWindowSize(window, w, h);
}

void dui_renderer_setRenderTop(void){
	SDL_SetRenderTarget(renderer, top_texture);
}

void dui_renderer_setRenderBottom(void){
	SDL_SetRenderTarget(renderer, bottom_texture);
}

void dui_renderer_drawRect(DUI_Rect rect, DUI_Color color){
	SDL_Rect dst_rect = {rect.x, rect.y, rect.w, rect.h};

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

	SDL_RenderFillRect(renderer, &dst_rect);
}

void dui_renderer_drawTexture(void* texture, DUI_Rect rect){
	SDL_Rect dst_rect = {rect.x, rect.y, rect.w, rect.h};

	SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
}

int dui_renderer_getTextWidth(const char* text){
	int width;
	TTF_SizeText(font, text, &width, NULL);
	return width;
}

int dui_renderer_getTextHeight(){
	return TTF_FontHeight(font);
}

void dui_renderer_drawText(const char* text, int pos_x, int pos_y, DUI_Color color){
	SDL_Color foreground = {color.r, color.g, color.b, color.a};

	SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, foreground);

	if(text_surface == NULL) return;

	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

	SDL_Rect size_rect = {pos_x, pos_y, text_surface->w, text_surface->h};

	SDL_Rect dst_rect = size_rect;

	SDL_RenderSetClipRect(renderer, &clip_rect);

	SDL_RenderCopy(renderer, text_texture, NULL, &dst_rect);

	SDL_DestroyTexture(text_texture);
	SDL_FreeSurface(text_surface);
	SDL_RenderSetClipRect(renderer, NULL);
}

void dui_renderer_setClipRect(DUI_Rect rect){
	clip_rect.x = rect.x;
	clip_rect.y = rect.y;
	clip_rect.w = rect.w;
	clip_rect.h = rect.h;
}

void dui_renderer_clear(DUI_Color color){
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, top_texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, bottom_texture);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, NULL);
}

void dui_renderer_present(void){
	SDL_SetRenderTarget(renderer, NULL);

	SDL_RenderCopy(renderer, bottom_texture, NULL, NULL);
	SDL_RenderCopy(renderer, top_texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void dui_renderer_end(void){
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	TTF_CloseFont(font);

	if(bottom_texture != NULL) SDL_DestroyTexture(bottom_texture);
	if(top_texture != NULL) SDL_DestroyTexture(top_texture);

	SDL_Quit();
	IMG_Quit();
	TTF_Quit();
}

#endif
