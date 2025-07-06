#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dui_renderer.h>
#include <dui.h>

#include "loader.h"
#include "world.h"
#include "renderer.h"
#include "viewer3d.h"
#include "gui.h"
#include "stack.h"
#include "entity.h"

int main(int argc, char** argv){
	dui_renderer_init();

	SDL_Renderer* renderer = dui_renderer_getRenderer();
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	Stack* profile_stack = loader_loadProfiles("scripts/profile.lua");
	Stack* entity_stack = stack_create(Entity*);

	World* world = world_create();

	loader_loadWorldTextures(world, "scripts/pack.lua");

	Renderer3D* renderer3D = renderer_init(renderer, world);
	viewer3d_init();
	renderer_setRes(renderer3D, 1280, 720, 4);

	DUI_Context context = DUI_CreateContext();
	int quit = 0;
	double delta_time = 0;
	int tab_pressed = 0;
	int tab_up = 1;
	int is_on_viewer = 1;
	int old_mouse_x, old_mouse_y;

	gui_init(&context);
	SDL_SetRelativeMouseMode(1);

	while(!quit){
		uint32_t first_time = SDL_GetTicks();
		const uint8_t* keys = SDL_GetKeyboardState(NULL);

		int mouse_x, mouse_y, mouse_pressed;
		mouse_pressed = SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON_LMASK;
		DUI_UpdateContext(&context, mouse_pressed, mouse_x, mouse_y);
		int mouse_xrel = 0;

		if(keys[SDL_SCANCODE_TAB]){
			if(tab_up){
				tab_pressed = 1;
				tab_up = 0;
			}
		}
		else{
			tab_up = 1;
			tab_pressed = 0;
		}

		if(tab_pressed){
			is_on_viewer = !is_on_viewer;
			SDL_SetRelativeMouseMode(is_on_viewer);
			tab_pressed = 0;
			int really_old_x = old_mouse_x;
			int really_old_y = old_mouse_y;
			SDL_GetMouseState(&old_mouse_x, &old_mouse_y);
			SDL_WarpMouseInWindow(NULL, really_old_x, really_old_y);
		}

		SDL_Event e;

		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT) quit = 1;
			if(e.type == SDL_WINDOWEVENT)
				if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					dui_renderer_resize();
					int new_width = e.window.data1;
					int new_height = e.window.data2;
					renderer_setRes(renderer3D, 1280, 1280 * new_height / new_width, 4);
				}
			if(e.type == SDL_MOUSEMOTION)
				mouse_xrel = e.motion.xrel;
			if(e.type == SDL_TEXTINPUT)
				DUI_UpdateCharTyped(&context, e.text.text[0]);
			if(e.type == SDL_KEYDOWN)
				if(e.key.keysym.sym == SDLK_BACKSPACE)
					DUI_PopCharTyped(&context);
		}

		world_clearProp(world);
		for(size_t i = 0; i < entity_stack->size; i++){
			Entity** entity = (stack_get(entity_stack, i));
			world_addProp(world, entity_toProp(*entity));
		}

		DUI_Color bg = {world->fog.r, world->fog.g, world->fog.b, 255};
		dui_renderer_clear(bg);
		viewer3d_render(renderer3D, world, !is_on_viewer);

		if(is_on_viewer){
			viewer3d_input(world, profile_stack, entity_stack, delta_time, mouse_xrel);
		}
		else{
			gui_render(world, profile_stack, entity_stack);
		}

		dui_renderer_present();
		delta_time = (double) (SDL_GetTicks() - first_time) / 1000;
	}

	world_destroy(world);
	renderer_destroy(renderer3D);
	viewer3d_quit();
	dui_renderer_end();

	return 0;
}
