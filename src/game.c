#include "game.h"
#include "world.h"
#include "renderer.h"
#include "loader.h"
#include "profile.h"
#include "entity.h"
#include "stack.h"
#include "hud.h"
#include "sfx.h"
#include "context.h"

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static Renderer3D* renderer3D = NULL;

static Stack* profile_stack = NULL;
static Stack* adder_stack = NULL;
static Stack* entity_stack = NULL;
static Stack* sfx_stack = NULL;

Entity* player = NULL;
static World* world = NULL;
static Vec3 camera;
static double camera_dir;
static double delta_time;
static int pause = 0;

static Lcontext context;

void game_init(void){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	window = SDL_CreateWindow(
			"Game",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, -1, 0);
	//renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_SetRelativeMouseMode(1);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	
	world = world_create();
	loader_loadWorldTextures(world, "scripts/pack.lua");

	camera = vec3_create(5, 5, 0.6);
	camera_dir = 0;

	renderer3D = renderer_init(renderer, world);

	renderer_setRes(renderer3D, 1280, 1280 * 9 / 16, 4);
	
	loader_create(&entity_stack);

	profile_stack = loader_loadProfiles("scripts/profile.lua");
	sfx_stack = loader_loadSfx("scripts/sfx.lua");

	adder_stack = stack_create(Entity*);

	context = logic_createContext(entity_stack, adder_stack, profile_stack, sfx_stack, world);

	pause = 1;

	const char* game_name = hud_init(renderer, &pause);

	if(game_name != NULL)
		SDL_SetWindowTitle(window, game_name);
}

void game_update(void){
	camera.z = 0;
	if(!pause) SDL_SetRelativeMouseMode(1);
	else SDL_SetRelativeMouseMode(0);

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_ESCAPE] && !pause) pause = 1;

	stack_clean(adder_stack);
	world_clearProp(world);

	for(size_t i = 0; i < entity_stack->size; i++){
		Entity** entity = (stack_get(entity_stack, i));
		if(!pause)
			entity_update(*entity, &context);
		world_addProp(world, entity_toProp(*entity));

		if(!strcmp((*entity)->profile->type, "player")){
			player = *entity;
			camera = (*entity)->position;
			camera_dir = (*entity)->direction;
		}
	}

	for(size_t i = 0; i < adder_stack->size; i++){
		stack_push(entity_stack, stack_get(adder_stack, i));
	}

	for(int i = entity_stack->size - 1; i >= 0; i--){
		Entity** entity = (stack_get(entity_stack, i));

		if((*entity)->kill){
			entity_destroy(*entity);
			stack_remove(entity_stack, i);
		}
	}

	camera.z += 0.5;

	logic_setContextElapsedTime(&context, delta_time);
}


void game_render(void){
	SDL_SetRenderDrawColor(renderer, world->fog.r, world->fog.g, world->fog.b, 255);
	SDL_RenderClear(renderer);

	renderer_render(renderer3D, camera, camera_dir);

	SDL_RenderCopy(renderer, renderer3D->texture, NULL, NULL);

	hud_render(&context);

	SDL_RenderPresent(renderer);
}

void game_single_loop(void){
	SDL_Event e;
	uint32_t first_time;

	first_time = SDL_GetTicks();

	while(SDL_PollEvent(&e)){
		//if(e.type == SDL_QUIT) quit = 1;
		if(e.type == SDL_MOUSEMOTION){
			if(player != NULL && !pause)
				player->direction += (double)(e.motion.xrel) / 500;
		}
	}

	game_update();
	game_render();

	delta_time = (double) (SDL_GetTicks() - first_time) / 1000;
}

void game_loop(void){
	int quit = 0;
	SDL_Event e;
	uint32_t zero_tick = SDL_GetTicks();
	uint32_t frames = 0;
	uint32_t first_time;

	while(!quit){
		first_time = SDL_GetTicks();

		while(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT) quit = 1;
			if(e.type == SDL_MOUSEMOTION){
				if(player != NULL && !pause)
					player->direction += (double)(e.motion.xrel) / 500;
			}
		}

		game_update();
		game_render();

		delta_time = (double) (SDL_GetTicks() - first_time) / 1000;

		frames++;
	}

	printf("Average Framerate: %lf\n", (double) frames * 1000 / (SDL_GetTicks() - zero_tick));
}

void game_run(void){
	game_init();
	game_loop();
	game_quit();
}

void game_quit(void){
	hud_quit();
	world_destroy(world);
	
	loader_free(profile_stack, entity_stack, sfx_stack);
	stack_destroy(adder_stack);

	renderer_destroy(renderer3D);

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}
