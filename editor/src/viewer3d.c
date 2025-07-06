#include "viewer3d.h"
#include "renderer.h"
#include "entity.h"
#include "profile.h"

#include <math.h>

#include <dui_renderer.h>

enum viewer_tools{
	TOOL_TILE = 0,
	TOOL_TILE_SELECTOR,
	TOOL_ENTITY_PICKER
};

static Vec3 current_tile;
static Vec3 final_current_tile;

static Vec3 next_current_tile;
static Profile* current_profile;
static Entity* current_entity;
static Vec3 camera = {2, 2, 0.5};
static double camera_dir = 0;
static int mouse_up = 0;
static int tile_set = 0;
static int modify_top = 0;
static int modify_side = 0;
static int selected_side_x = 0;
static int current_texture = 0;
static int current_profile_id = 2;
static int current_tool = 0;

static Vec3 viewer3d_getTileOnPixel(const World* world, double mouse_x, double mouse_y);
static Entity* viewer3d_getEntityOnPixel(const World* world, Stack* entity_stack, double mouse_x, double mouse_y);
static double viewer3d_getZcoord(Vec3 tile, double y_pos);
static double viewer3d_getDepth(Vec3 tile);
static Vec3 viewer3d_project(Vec3 tile, int width, int height);
static Vec3 viewer3d_inverseProject(double x_pos, double y_pos, double z_world, int width, int height);
static void viewer3d_cameraInput(double delta_time, int mouse_xrel);
static void viewer3d_mouseInputTileBuilder(World* world, int pressed, int mouse_y, int window_width, int window_height);
static void viewer3d_mouseInputTileSelector(World* world, int pressed, int mouse_y, int window_width, int window_height);
static void viewer3d_mouseInputTexturePainting(World* world, int mouse_y, int window_height);
static void viewer3d_mouseInputEntityPicker(World* world, Stack* profile_stack, Stack* entity_stack, double y_normalized);
static void viewer3d_mouseInputEntityDeleter(World* world, Stack* profile_stack, Stack* entity_stack, double y_normalized);
static void viewer3d_mouseInput(World* world, Stack* profile_stack, Stack* entity_stack);
static void viewer3d_renderGuideLines(SDL_Renderer* renderer, World* world, int width, int height);

static Vec3 viewer3d_getTileOnPixel(const World* world, double mouse_x, double mouse_y){
	Vec3 dir = vec3_create(mouse_x - 0.5, 1, 0.5 - mouse_y);
	dir = vec3_normalize(dir);

	Vec3 new_dir = {
		dir.x * cos(camera_dir) + dir.y * sin(camera_dir),
		-dir.x * sin(camera_dir) + dir.y * cos(camera_dir),
		dir.z
	};

	dir = vec3_mul(new_dir, 0.01);

	Vec3 tile = camera;

	do{
		tile = vec3_add(tile, dir);

		if(vec3_size(vec3_sub(tile, camera)) >= world->fog_distance) break;
		if(tile.x < 0 || tile.y < 0 || tile.x >= WORLD_SIZE || tile.y >= WORLD_SIZE)
			break;

	}while(tile.z >= world_getBottomHeight(world, tile.x, tile.y) &&
			tile.z <= world_getTopHeight(world, tile.x, tile.y));

	return tile;
}

static Entity* viewer3d_getEntityOnPixel(const World* world, Stack* entity_stack, double mouse_x, double mouse_y){
	Vec3 closest_tile = viewer3d_getTileOnPixel(world, mouse_x, mouse_y);
	Entity* closest_entity = NULL;

	for(size_t i = 0; i < entity_stack->size; i++){
		Entity* current_entity = *((Entity**) stack_get(entity_stack, i));

		Vec3 projection = viewer3d_project(current_entity->position, 1, 1);

		double depth = viewer3d_getDepth(current_entity->position);

		double width = current_entity->profile->width / depth;
		double height = current_entity->profile->height / depth;

		if(fabs(mouse_x - projection.x) > width / 2 || mouse_y < projection.y - height ||
				mouse_y > projection.y)
			continue;

		if(vec3_size(vec3_sub(current_entity->position, camera)) <
				vec3_size(vec3_sub(closest_tile, camera))){
			closest_entity = current_entity;
			closest_tile = current_entity->position;
		}
	}

	if(vec3_size(vec3_sub(closest_tile, camera)) >= world->fog_distance) return NULL;

	return closest_entity;
}

static double viewer3d_getZcoord(Vec3 tile, double y_pos){
	double depth = viewer3d_getDepth(tile);

	return y_pos * depth + camera.z;
}

static double viewer3d_getDepth(Vec3 tile){
	double depth;

	tile = vec3_sub(tile, camera);

	depth = tile.x * sin(camera_dir) + tile.y * cos(camera_dir);

	return depth;
}

static Vec3 viewer3d_project(Vec3 tile, int width, int height){
	tile = vec3_sub(tile, camera);

	Vec3 new_tile = {
		tile.x * cos(camera_dir) - tile.y * sin(camera_dir),
		tile.x * sin(camera_dir) + tile.y * cos(camera_dir),
		tile.z
	};

	tile = new_tile;
	if(fabs(tile.y) < 0.1) tile.y = 1;
	
	Vec3 projection = {
		(tile.x / tile.y) * height + (double) width/2,
		(0.5 - tile.z / tile.y) * height,
		0
	};

	if(tile.y <= 0){
		projection.x = (double) width / 2;
		projection.y = height;
	}

	return projection;
}

static Vec3 viewer3d_inverseProject(double x_pos, double y_pos, double z_world, int width, int height){
	double tile_x, tile_y, tile_z;
	y_pos -= height / 2;
	tile_z = z_world - camera.z;
	tile_y = (0.5 - tile_z) * height / y_pos;
	tile_x = (x_pos - width/2) / height * y_pos;

	Vec3 inv_project = {
		tile_x * cos(camera_dir) + tile_y * sin(camera_dir),
		-tile_x * sin(camera_dir) + tile_y * cos(camera_dir),
		tile_z
	};

	inv_project = vec3_add(inv_project, camera);

	return inv_project;
}

static void viewer3d_cameraInput(double delta_time, int mouse_xrel){
	Vec3 accel = {0, 0, 0};
	double velocity = 5;

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_LCTRL]){
		velocity *= 3;
	}

	if(keys[SDL_SCANCODE_SPACE]){
		camera.z += delta_time * velocity;
	}

	if(keys[SDL_SCANCODE_LSHIFT]){
		camera.z -= delta_time * velocity;
	}

	if(keys[SDL_SCANCODE_W]){
		accel.x += sin(camera_dir) * delta_time * velocity;
		accel.y += cos(camera_dir) * delta_time * velocity;
	}

	if(keys[SDL_SCANCODE_S]){
		accel.x -= sin(camera_dir) * delta_time * velocity;
		accel.y -= cos(camera_dir) * delta_time * velocity;
	}

	if(keys[SDL_SCANCODE_D]){
		accel.x += cos(camera_dir) * delta_time * velocity;
		accel.y -= sin(camera_dir) * delta_time * velocity;
	}

	if(keys[SDL_SCANCODE_A]){
		accel.x -= cos(camera_dir) * delta_time * velocity;
		accel.y += sin(camera_dir) * delta_time * velocity;
	}

	accel = vec3_normalize(accel);
	Vec3 camera_vel = vec3_mul(accel, velocity);
	
	camera = vec3_add(camera, vec3_mul(camera_vel, delta_time));

	camera_dir += (double) mouse_xrel * 0.01;
}

static void viewer3d_mouseInputTexturePainting(World* world, int mouse_y, int window_height){
	double y_normalized = (double) mouse_y / window_height;

	Vec3 new_tile = viewer3d_getTileOnPixel(world, 0.5, y_normalized);

	double bottom_height = world_getBottomHeight(world, new_tile.x, new_tile.y);
	double top_height = world_getTopHeight(world, new_tile.x, new_tile.y);
	double z_coord = viewer3d_getZcoord(new_tile, 0.5 - y_normalized);

	if(current_tool != TOOL_TILE_SELECTOR){
		int tile_x = new_tile.x;
		int tile_y = new_tile.y;

		if(fabs(z_coord - bottom_height) < 0.05)
			world->world[tile_x][tile_y].bottom_id = current_texture;
		else if(fabs(z_coord - top_height) < 0.05)
			world->world[tile_x][tile_y].top_id = current_texture;
		else if(z_coord < bottom_height)
			world->world[tile_x][tile_y].bottom_wall_id = current_texture;
		else if(z_coord > top_height)
			world->world[tile_x][tile_y].top_wall_id = current_texture;

		return;
	}

	double min_x = fmin(current_tile.x, final_current_tile.x);
	double max_x = fmax(current_tile.x, final_current_tile.x);
	double min_y = fmin(current_tile.y, final_current_tile.y);
	double max_y = fmax(current_tile.y, final_current_tile.y);

	for(int i = min_x; i < max_x; i++){
		for(int j = min_y; j < max_y; j++){
			if(fabs(z_coord - bottom_height) < 0.05)
				world->world[i][j].bottom_id = current_texture;
			else if(fabs(z_coord - top_height) < 0.05)
				world->world[i][j].top_id = current_texture;
			else if(z_coord < bottom_height)
				world->world[i][j].bottom_wall_id = current_texture;
			else if(z_coord > top_height)
				world->world[i][j].top_wall_id = current_texture;
		}
	}

	tile_set = 0;
}

static void viewer3d_mouseInputTileBuilder(World* world, int pressed, int mouse_y, int window_width, int window_height){
	double y_normalized = (double) mouse_y / window_height;

	if(!tile_set && pressed){
		Vec3 new_tile = viewer3d_getTileOnPixel(world, 0.5, y_normalized);

		double bottom_height = world_getBottomHeight(world, new_tile.x, new_tile.y);
		double top_height = world_getTopHeight(world, new_tile.x, new_tile.y);
		double z_coord = viewer3d_getZcoord(new_tile, 0.5 - y_normalized);

		if(fabs(z_coord - bottom_height) < 0.05){
			current_tile = new_tile;
			tile_set = 1;
			modify_top = 0;
			modify_side = 0;
		}
		else if(fabs(z_coord - top_height) < 0.05){
			current_tile = new_tile;
			tile_set = 1;
			modify_top = 1;
			modify_side = 0;
		}
		else if(z_coord < bottom_height){
			current_tile = new_tile;
			tile_set = 1;
			modify_top = 0;
			modify_side = 1;
			current_tile.z = z_coord;

			if(fabs(round(new_tile.x) - new_tile.x) < 0.1) selected_side_x = 0;
			else selected_side_x = 1;
		}
		else if(z_coord > top_height){
			current_tile = new_tile;
			tile_set = 1;
			modify_top = 1;
			modify_side = 1;
			current_tile.z = z_coord;

			if(fabs(round(new_tile.x) - new_tile.x) < 0.1) selected_side_x = 0;
			else selected_side_x = 1;
		}
		else{
			current_tile = vec3_create(WORLD_SIZE, WORLD_SIZE, 0);
		}
	}


	if(!modify_side && pressed){
		double new_height = viewer3d_getZcoord(current_tile, 0.5 - y_normalized);

		new_height = new_height - fmod(new_height, 1.0 / 16);

		if(modify_top) 
			world->world[(int) current_tile.x][(int) current_tile.y].top_height = new_height;
		else
			world->world[(int) current_tile.x][(int) current_tile.y].bottom_height = new_height;
	}
	else if(modify_side && pressed){
		next_current_tile =
			//viewer3d_inverseProject(window_width/2, mouse_y, current_tile.z+0.5, window_width, window_height);
			viewer3d_getTileOnPixel(world, 0.5, y_normalized);

		if(selected_side_x) next_current_tile.x = current_tile.x;
		else next_current_tile.y = current_tile.y;
	}
	else if(modify_side && !pressed){
		Vec3 new_tile = next_current_tile;
		int add_x = (new_tile.x < current_tile.x ? 1 : -1) * (!selected_side_x);
		int add_y = (new_tile.y < current_tile.y ? 1 : -1) * (selected_side_x);

		while(1){
			if((int) new_tile.x == (int) current_tile.x && !selected_side_x) break;
			if((int) new_tile.y == (int) current_tile.y && selected_side_x) break;

			if(modify_top)
				world->world[(int)new_tile.x][(int)new_tile.y].top_height =
					world_getTopHeight(world, current_tile.x, current_tile.y);
			else
				world->world[(int)new_tile.x][(int)new_tile.y].bottom_height =
					world_getBottomHeight(world, current_tile.x, current_tile.y);

			new_tile.x += add_x;
			new_tile.y += add_y;
		}

		modify_side = 0;
	}
}

static void viewer3d_mouseInputTileSelector(World* world, int pressed, int mouse_y, int window_width, int window_height){
	double y_normalized = (double) mouse_y / window_height;

	if(tile_set == 2 && pressed){
		//Vec3 new_tile = vec3_mul(vec3_add(current_tile, final_current_tile), 0.5);
		Vec3 new_tile = viewer3d_getTileOnPixel(world, 0.5, y_normalized);
		//Vec3 new_tile = current_tile;

		double bottom_height = world_getBottomHeight(world, new_tile.x, new_tile.y);
		double top_height = world_getTopHeight(world, new_tile.x, new_tile.y);
		double z_coord = viewer3d_getZcoord(new_tile, 0.5 - y_normalized);
		next_current_tile = viewer3d_getTileOnPixel(world, 0.5, y_normalized);

		if(fabs(z_coord - bottom_height) < fabs(z_coord - top_height)){
			modify_top = 0;
			modify_side = 0;
			tile_set = 3;
		}
		else{
			modify_top = 1;
			modify_side = 0;
			tile_set = 3;
		}
	}

	if(tile_set == 3 && !pressed)
		tile_set = 0;

	if(!modify_side && pressed && tile_set == 3){
		double new_height = viewer3d_getZcoord(next_current_tile, 0.5 - y_normalized);

		new_height = new_height - fmod(new_height, 1.0 / 16);

		double min_x = fmin(current_tile.x, final_current_tile.x);
		double max_x = fmax(current_tile.x, final_current_tile.x);
		double min_y = fmin(current_tile.y, final_current_tile.y);
		double max_y = fmax(current_tile.y, final_current_tile.y);

		for(int i = min_x; i < max_x; i++){
			for(int j = min_y; j < max_y; j++){
				if(modify_top) world->world[i][j].top_height = new_height;
				else world->world[i][j].bottom_height = new_height;
			}
		}
	}
}

static void viewer3d_mouseInputEntityPicker(World* world, Stack* profile_stack, Stack* entity_stack, double y_normalized){
	if(!mouse_up){
		if(current_entity == NULL){
			current_entity = viewer3d_getEntityOnPixel(world, entity_stack, 0.5, y_normalized);
		}
	}
	else current_entity = NULL;

	Vec3 current_position = viewer3d_getTileOnPixel(world, 0.5, y_normalized);

	current_position.x = current_position.x - fmod(current_position.x, 1.0 / 16);
	current_position.y = current_position.y - fmod(current_position.y, 1.0 / 16);

	if(!mouse_up && current_entity == NULL){
		current_entity = entity_create(current_profile);
		stack_push(entity_stack, &current_entity);
	}

	if(current_entity != NULL){
		double bottom_height = world_getBottomHeight(world, current_position.x, current_position.y);
		double top_height = world_getTopHeight(world, current_position.x, current_position.y);

		if(fabs(current_position.z - top_height) <= 0.05)
			current_position.z = top_height - current_entity->profile->height;

		if(fabs(current_position.z - bottom_height) <= 0.05)
			current_position.z = bottom_height;

		current_entity->position = current_position;
	}
}

static void viewer3d_mouseInputEntityDeleter(World* world, Stack* profile_stack, Stack* entity_stack, double y_normalized){
	Entity* delete_entity = viewer3d_getEntityOnPixel(world, entity_stack, 0.5, y_normalized);
	if(delete_entity == NULL) return;

	for(size_t i = 0; i < entity_stack->size; i++){
		if(delete_entity == *((Entity**) stack_get(entity_stack, i))){
			current_entity = NULL;
			entity_destroy(delete_entity);
			stack_remove(entity_stack, i);

			break;
		}
	}
}

static void viewer3d_mouseInput(World* world, Stack* profile_stack, Stack* entity_stack){
	int mouse_y, window_width, window_height, mouse_pressed;

	mouse_pressed = SDL_GetMouseState(NULL, &mouse_y);
	dui_renderer_getSize(&window_width, &window_height);

	const uint8_t* keys = SDL_GetKeyboardState(NULL);

	if(keys[SDL_SCANCODE_E]) current_tool = TOOL_ENTITY_PICKER;
	if(keys[SDL_SCANCODE_T]) current_tool = TOOL_TILE;
	if(keys[SDL_SCANCODE_R]) current_tool = TOOL_TILE_SELECTOR;
	if(keys[SDL_SCANCODE_ESCAPE]) tile_set = 0;

	if(current_tool == TOOL_TILE_SELECTOR){
		viewer3d_mouseInputTileSelector(world, mouse_pressed & SDL_BUTTON_LMASK, mouse_y, window_width, window_height);

		if(mouse_pressed && tile_set == 0){
			tile_set = 1;
			current_tile = viewer3d_getTileOnPixel(world, 0.5, (double) mouse_y / window_height);
		}

		if(mouse_pressed && tile_set == 1){
			final_current_tile = viewer3d_getTileOnPixel(world, 0.5, (double) mouse_y / window_height);
		}

		if(!mouse_pressed && tile_set == 1){
			tile_set = 2;
		}
	
		if(mouse_pressed & SDL_BUTTON_RMASK){
			viewer3d_mouseInputTexturePainting(world, mouse_y, window_height);
		}
	}
	else if(current_tool == TOOL_TILE){
		viewer3d_mouseInputTileBuilder(world, mouse_pressed & SDL_BUTTON_LMASK, mouse_y, window_width, window_height);

		if(!mouse_pressed) tile_set = 0;

		if(mouse_pressed & SDL_BUTTON_RMASK){
			viewer3d_mouseInputTexturePainting(world, mouse_y, window_height);
		}
	}
	else if(current_tool == TOOL_ENTITY_PICKER){
		if(mouse_pressed & SDL_BUTTON_LMASK){
			mouse_up = 0;
		}
		else mouse_up = 1;

		viewer3d_mouseInputEntityPicker(world, profile_stack, entity_stack, (double) mouse_y / window_height);

		if(mouse_pressed & SDL_BUTTON_RMASK)
			viewer3d_mouseInputEntityDeleter(world, profile_stack, entity_stack, (double) mouse_y / window_height);
	}
}

void viewer3d_init(void){
	current_profile_id = 0;
	current_profile = NULL;
}

void viewer3d_input(World* world, Stack* profile_stack, Stack* entity_stack, double delta_time, int mouse_xrel){
	if(current_profile_id < (int) profile_stack->size)
		current_profile = *((Profile**) stack_get(profile_stack, current_profile_id));

	viewer3d_cameraInput(delta_time, mouse_xrel);
	viewer3d_mouseInput(world, profile_stack, entity_stack);

	if(camera.z - 0.5 <= world_getBottomHeight(world, camera.x, camera.y))
		camera.z = world_getBottomHeight(world, camera.x, camera.y) + 0.5;

	if(camera.z + 0.5 >= world_getTopHeight(world, camera.x, camera.y))
		camera.z = world_getTopHeight(world, camera.x, camera.y) - 0.5;
}

void viewer3d_setCurrentTexture(int id){
	current_texture = id;
}

void viewer3d_setCurrentProfile(int id){
	current_profile_id = id;
}

int viewer3d_getCurrentTexture(void){
	return current_texture;
}

Vec3 viewer3d_getCurrentTile(void){
	return current_tile;
}

static void viewer3d_renderGuideLines(SDL_Renderer* renderer, World* world, int width, int height){
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);

	Vec3 north, east, west, south;
	Vec3 tmp_tile = current_tile;
	Vec3 final_tmp_tile = final_current_tile;
	tmp_tile.z = 0;

	if(!modify_side && current_tool == TOOL_TILE_SELECTOR){
		SDL_SetRenderDrawColor(renderer, 128, 255, 0, 200);

		int min_x = fmin(tmp_tile.x, final_tmp_tile.x);
		int max_x = fmax(tmp_tile.x, final_tmp_tile.x);
		int min_y = fmin(tmp_tile.y, final_tmp_tile.y);
		int max_y = fmax(tmp_tile.y, final_tmp_tile.y);

		int dx = max_x - min_x + 1;
		int dy = max_y - min_y + 1;

		Vec3 min_tile = {min_x, min_y, current_tile.z};

		north = viewer3d_project(vec3_add(min_tile, vec3_create(0, 0, 0)), width, height);
		south = viewer3d_project(vec3_add(min_tile, vec3_create(dx, 0, 0)), width, height);
		east = viewer3d_project(vec3_add(min_tile, vec3_create(0, dy, 0)), width, height);
		west = viewer3d_project(vec3_add(min_tile, vec3_create(dx, dy, 0)), width, height);

		if(tile_set == 1){
			SDL_RenderDrawLine(renderer, north.x, north.y, south.x, south.y);
			SDL_RenderDrawLine(renderer, south.x, south.y, west.x, west.y);
			SDL_RenderDrawLine(renderer, west.x, west.y, east.x, east.y);
			SDL_RenderDrawLine(renderer, east.x, east.y, north.x, north.y);
		}
		else if(tile_set == 3){
			SDL_RenderDrawLine(renderer, north.x, 0, north.x, height);
			SDL_RenderDrawLine(renderer, south.x, 0, south.x, height);
			SDL_RenderDrawLine(renderer, east.x, 0, east.x, height);
			SDL_RenderDrawLine(renderer, west.x, 0, west.x, height);
		}
	}
	else if(!modify_side && tile_set && current_tool == TOOL_TILE){
		tmp_tile.x = (int) tmp_tile.x;
		tmp_tile.y = (int) tmp_tile.y;
		tmp_tile.z = 0;

		north = viewer3d_project(vec3_add(tmp_tile, vec3_create(0, 0, 0)), width, height);
		south = viewer3d_project(vec3_add(tmp_tile, vec3_create(1, 0, 0)), width, height);
		east = viewer3d_project(vec3_add(tmp_tile, vec3_create(0, 1, 0)), width, height);
		west = viewer3d_project(vec3_add(tmp_tile, vec3_create(1, 1, 0)), width, height);

		SDL_RenderDrawLine(renderer, north.x, 0, north.x, height);
		SDL_RenderDrawLine(renderer, south.x, 0, south.x, height);
		SDL_RenderDrawLine(renderer, east.x, 0, east.x, height);
		SDL_RenderDrawLine(renderer, west.x, 0, west.x, height);
		return;
	}
	else if(modify_side && tile_set){
		if(selected_side_x)
			tmp_tile.x = (int) tmp_tile.x;
		else
			tmp_tile.y = (int) tmp_tile.y;

		Vec3 n_north, n_south, n_east, n_west;
		Vec3 tmp_tile_next = next_current_tile;

		if(selected_side_x){
			tmp_tile_next.x = (int) tmp_tile_next.x;
		}
		else{
			tmp_tile_next.y = (int) tmp_tile_next.y;
		}


		tmp_tile_next.z = 0;
		double h;

		if(modify_top){
			tmp_tile.z = tmp_tile_next.z = world_getTopHeight(world, tmp_tile.x, tmp_tile.y);
			h = 1000;
		}
		else{
			h = world_getBottomHeight(world, tmp_tile.x, tmp_tile.y);
		}
		
		north = viewer3d_project(vec3_add(tmp_tile, vec3_create(0, 0, 0)), width, height);
		south = viewer3d_project(vec3_add(tmp_tile, vec3_create(0, 0, h)), width, height);
		east = viewer3d_project(vec3_add(tmp_tile, vec3_create(selected_side_x, !selected_side_x, 0)), width, height);
		west = viewer3d_project(vec3_add(tmp_tile, vec3_create(selected_side_x, !selected_side_x, h)), width, height);

		n_north = viewer3d_project(vec3_add(tmp_tile_next, vec3_create(0, 0, 0)), width, height);
		n_south = viewer3d_project(vec3_add(tmp_tile_next, vec3_create(0, 0, h)), width, height);
		n_east = viewer3d_project(vec3_add(tmp_tile_next, vec3_create(selected_side_x, !selected_side_x, 0)), width, height);
		n_west = viewer3d_project(vec3_add(tmp_tile_next, vec3_create(selected_side_x, !selected_side_x, h)), width, height);

		SDL_RenderDrawLine(renderer, north.x, north.y, n_north.x, n_north.y);
		SDL_RenderDrawLine(renderer, south.x, south.y, n_south.x, n_south.y);
		SDL_RenderDrawLine(renderer, east.x, east.y, n_east.x, n_east.y);
		SDL_RenderDrawLine(renderer, west.x, west.y, n_west.x, n_west.y);

		SDL_RenderDrawLine(renderer, n_north.x, n_north.y, n_south.x, n_south.y);
		SDL_RenderDrawLine(renderer, n_south.x, n_south.y, n_west.x, n_west.y);
		SDL_RenderDrawLine(renderer, n_west.x, n_west.y, n_east.x, n_east.y);
		SDL_RenderDrawLine(renderer, n_east.x, n_east.y, n_north.x, n_north.y);
	}

	SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);

	if(current_entity != NULL){
		double depth = viewer3d_getDepth(current_entity->position);
		
		int texture_width = height * current_entity->profile->width / depth;
		int texture_height = height * current_entity->profile->height / depth;

		Vec3 projection = viewer3d_project(current_entity->position, width, height);
		projection.x -= texture_width / 2;

		SDL_RenderDrawLine(renderer, projection.x, projection.y, projection.x + texture_width, projection.y);
		projection.y -= texture_height;
		SDL_RenderDrawLine(renderer, projection.x, projection.y, projection.x + texture_width, projection.y);
		projection.y += texture_height;
		SDL_RenderDrawLine(renderer, projection.x, projection.y - texture_height, projection.x, projection.y);
		projection.x += texture_width;
		SDL_RenderDrawLine(renderer, projection.x, projection.y - texture_height, projection.x, projection.y);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		for(int i = 0; i < 9; i++){
			Vec3 tile_int, tile_start, tile_end;
			Vec3 proj_tile_start, proj_tile_end;
			double step = (double) i / 8;

			tile_int = current_entity->position;
			tile_int.x = (int) tile_int.x;
			tile_int.y = (int) tile_int.y;
			double top_height = world_getTopHeight(world, tile_int.x, tile_int.y);

			/* testing if we need to draw the guide lines on the top of the tile */
			if(fabs(tile_int.z + current_entity->profile->height - top_height) < 0.05)
				tile_int.z += current_entity->profile->height;

			tile_start = vec3_add(tile_int, vec3_create(step, 0, 0));
			tile_end = vec3_add(tile_int, vec3_create(step, 1, 0));

			proj_tile_start = viewer3d_project(tile_start, width, height);
			proj_tile_end = viewer3d_project(tile_end, width, height);

			SDL_RenderDrawLine(renderer, proj_tile_start.x, proj_tile_start.y,
					proj_tile_end.x, proj_tile_end.y);

			tile_start = vec3_add(tile_int, vec3_create(0, step, 0));
			tile_end = vec3_add(tile_int, vec3_create(1, step, 0));

			proj_tile_start = viewer3d_project(tile_start, width, height);
			proj_tile_end = viewer3d_project(tile_end, width, height);

			SDL_RenderDrawLine(renderer, proj_tile_start.x, proj_tile_start.y,
					proj_tile_end.x, proj_tile_end.y);
		}
	}
}

void viewer3d_render(Renderer3D* renderer, World* world, int paused){
	renderer_render(renderer, camera, camera_dir);

	SDL_RenderCopy(
			renderer->renderer,
			renderer->texture,
			NULL,
			NULL
			);

	if(paused) return;

	int window_width, window_height, mouse_y;
	dui_renderer_getSize(&window_width, &window_height);

	SDL_GetMouseState(NULL, &mouse_y);
	SDL_Rect crosshair = {window_width/2, mouse_y, 10, 10};

	switch(current_tool){
		case TOOL_TILE:
		SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 0, 255);
		break;

		case TOOL_TILE_SELECTOR:
		SDL_SetRenderDrawColor(renderer->renderer, 128, 255, 0, 255);
		break;

		case TOOL_ENTITY_PICKER:
		SDL_SetRenderDrawColor(renderer->renderer, 0, 255, 255, 255);
		break;
	}

	SDL_RenderFillRect(renderer->renderer, &crosshair);

	viewer3d_renderGuideLines(renderer->renderer, world, window_width, window_height);
}

void viewer3d_quit(void){
}
