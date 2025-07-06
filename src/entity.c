#include "entity.h"
#include "logic.h"
#include "etable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <math.h>

#define PI 3.141592

#define MIN_VELOCITY 0.05
#define MIN_STEP (3.0 / 16)

static int entity_compareDistance(const void* a, const void* b);

static Vec3 entity_accelerate(Entity* entity, double acceleration, double delta_time);
static void entity_accelerateGround(Entity* entity, double delta_time);
static void entity_accelerateAir(Entity* entity, double delta_time);

static int entity_checkCollisionWorld(Entity* entity, World* world, Vec3 offset);
static void entity_solveCollisionWorld(Entity* entity, World* world);
static void entity_applyCollisionWorld(Entity* entity, World* world, double delta_time);

static int entity_checkCollisionWithEntity(Entity* entity, Entity* enemy);
static void entity_getMinAndMaxAngle(Entity* viewer, Entity* enemy, double* ang_min, double* ang_max);
static int entity_testLine(World* world, Vec3 start_pos, Vec3 end_pos);

static void entity_updateAnimation(Entity* entity, double delta_time);

static int entity_compareDistance(const void* a, const void* b){
	Entity* entity = *((Entity**) a);
	Entity* enemy = *((Entity**) b);
	Entity* parent = enemy->parent_entity;

	double diff = vec3_sizeSqr(vec3_sub(entity->position, parent->position)) -
		vec3_sizeSqr(vec3_sub(enemy->position, parent->position));

	if(diff > 0) return -1;
	else return 1;
}

static Vec3 entity_accelerate(Entity* entity, double acceleration, double delta_time){
	Vec3 accel_dir;
	double accel_vel;
	double proj_vel;

	accel_dir = vec3_normalize(entity->acceleration);

	proj_vel = vec3_dotProduct(accel_dir, entity->velocity);
	accel_vel = acceleration * delta_time;

	if(proj_vel + accel_vel > entity->max_velocity)
		accel_vel = entity->max_velocity - proj_vel;

	return vec3_add(entity->velocity, vec3_mul(accel_dir, accel_vel));
}

static void entity_accelerateGround(Entity* entity, double delta_time){
	if(vec3_size(entity->velocity) != 0){
		double drop = entity->friction * delta_time;
		entity->velocity = vec3_mul(entity->velocity, fmax(1.0 - drop, 0));
	}

	entity->velocity =
		entity_accelerate(
				entity,
				entity->ground_acceleration,
				delta_time
				);
}

static void entity_accelerateAir(Entity* entity, double delta_time){
	entity->velocity =
		entity_accelerate(
				entity,
				entity->air_acceleration,
				delta_time
				);
}

static int entity_checkCollisionWorld(Entity* entity, World* world, Vec3 offset){
	Vec3 position = vec3_add(entity->position, offset);

	double min_height = world_getBottomHeight(world, position.x, position.y);
	double top_height = world_getTopHeight(world, position.x, position.y);

	if(entity->position.z < min_height - MIN_STEP || entity->position.z + entity->profile->height > top_height){
		return 1;
	}

	if(position.x < 0 || position.y < 0 || position.x >= WORLD_SIZE || position.y >= WORLD_SIZE) return 1;

	return 0;
}

static void entity_solveCollisionWorld(Entity* entity, World* world){
	Vec3 offset_left = vec3_create(-entity->profile->width/2, 0, 0);
	Vec3 offset_right = vec3_create(entity->profile->width/2, 0, 0);
	Vec3 offset_top = vec3_create(0, entity->profile->width/2, 0);
	Vec3 offset_bottom = vec3_create(0, -entity->profile->width/2, 0);

	if(entity_checkCollisionWorld(entity, world, offset_left)){
		entity->position.x = (int)(entity->position.x - offset_left.x) - offset_left.x;
		entity->has_collided_with_world = X_AXIS;
	}
	if(entity_checkCollisionWorld(entity, world, offset_top)){
		entity->position.y = (int)(entity->position.y - offset_top.y + 1) - offset_top.y;
		entity->has_collided_with_world = Y_AXIS;
	}
	if(entity_checkCollisionWorld(entity, world, offset_right)){
		entity->position.x = (int)(entity->position.x - offset_right.x + 1) - offset_right.x;
		entity->has_collided_with_world = X_AXIS;
	}
	if(entity_checkCollisionWorld(entity, world, offset_bottom)){
		entity->position.y = (int)(entity->position.y - offset_bottom.y) - offset_bottom.y;
		entity->has_collided_with_world = Y_AXIS;
	}
}

static void entity_applyCollisionWorld(Entity* entity, World* world, double delta_time){
	Vec3 delta_vel = vec3_mul(entity->velocity, delta_time);
	double old_min_height, old_top_height;
	entity->position.z += delta_vel.z;

	old_min_height = world_getBottomHeight(world, entity->position.x, entity->position.y);
	old_top_height = world_getTopHeight(world, entity->position.x, entity->position.y);

	if(entity->position.z < old_min_height){
		entity->position.z = old_min_height;
		entity->velocity.z = 0;
		entity->is_on_ground = 1;
	}
	else if(entity->position.z + entity->profile->height > old_top_height){
		entity->position.z = old_top_height - entity->profile->height;
		entity->velocity.z = 0;
	}
	else{
		entity->is_on_ground = 0;
	}

	entity->position.x += delta_vel.x;
	entity_solveCollisionWorld(entity, world);
	entity->position.y += delta_vel.y;
	entity_solveCollisionWorld(entity, world);
}

static int entity_checkCollisionWithEntity(Entity* entity, Entity* enemy){
	Vec3 ini_pos = entity->position;
	Vec3 ene_pos = enemy->position;

	ini_pos.z = ene_pos.z = 0;

	double max_radius_ini = entity->profile->width;
	double max_radius_ene = enemy->profile->width;

	double dist_sqr = vec3_sizeSqr(vec3_sub(ini_pos, ene_pos));
	double max_dist_sqr = (max_radius_ini + max_radius_ene)/2;
	max_dist_sqr *= max_dist_sqr;

	if(dist_sqr > max_dist_sqr)
		return 0;

	if(entity->position.z + entity->profile->height < enemy->position.z ||
			entity->position.z > enemy->position.z + enemy->profile->height) return 0;

	if(enemy->position.z + enemy->profile->height < entity->position.z ||
			enemy->position.z > entity->position.z + entity->profile->height) return 0;

	return 1;
}

static void entity_getMinAndMaxAngle(Entity* viewer, Entity* enemy, double* ang_min, double* ang_max){
	double variation = atan2(
			enemy->profile->width / 2,
			vec3_size(vec3_sub(enemy->position, viewer->position))
			);

	double ang = atan2(
			enemy->position.x - viewer->position.x,
			enemy->position.y - viewer->position.y
			);

	if(ang < 0) ang += 2 * PI;

	*ang_min = ang - fabs(variation);
	*ang_max = ang + fabs(variation);
}

static int entity_testLine(World* world, Vec3 start_pos, Vec3 end_pos){
	double dx, dy, steps, start_x, start_y;

	dx = (end_pos.x - start_pos.x);
	dy = (end_pos.y - start_pos.y);

	if(fabs(dx) < 1 && fabs(dy) < 1) return 1;

	steps = 0;

	if(fabs(dx) > fabs(dy)) steps = fabs(dx);
	else steps = fabs(dy);

	start_x = start_pos.x, start_y = start_pos.y;

	for(int i = 0; i < steps - 1; i++){
		start_x += (double)dx/steps;
		start_y += (double)dy/steps;

		double bottom_height = world_getBottomHeight(world, start_x, start_y);
		double top_height = world_getTopHeight(world, start_x, start_y);

		if(end_pos.z > start_pos.z && (bottom_height > end_pos.z || top_height < end_pos.z))
			return 0;
		if(end_pos.z <= start_pos.z && (bottom_height > start_pos.z || top_height < start_pos.z))
			return 0;
	}

	return 1;
}

static void entity_updateAnimation(Entity* entity, double delta_time){
	entity->current_animation.timer += delta_time;

	if(entity->current_animation.frames->size > 0)
		entity->current_animation.current_frame %= entity->current_animation.frames->size;

	if(entity->current_animation.timer * entity->current_animation.fps > 1.0){
		entity->current_animation.current_frame++;
		entity->current_animation.timer = 0;
	}
}

Entity* entity_create(Profile* profile){
	Entity* entity = malloc(sizeof(Entity));

	entity->profile = profile;
	entity->position = vec3_create(0, 0, 0);
	entity->velocity = vec3_create(0, 0, 0);
	entity->acceleration = vec3_create(0, 0, 0);
	entity->kill = 0;
	entity->direction = 0;
	entity->is_on_ground = 1;
	entity->has_collided_with_world = 0;

	entity->max_velocity = 0;
	entity->max_velocity_abs = 10000;
	entity->friction = 0;
	entity->ground_acceleration = 0;
	entity->air_acceleration = 0;
	entity->gravity = 0;

	entity->properties = NULL;
	entity->working_stack = stack_create(Entity*);

	entity->current_animation.fps = 30;
	entity->current_animation.timer = 0;
	entity->current_animation.current_frame = 0;
	entity->current_animation.frames = stack_create(size_t);

	logic_callCreateFunction(entity);

	return entity;
}

Wprop entity_toProp(Entity* entity){
	Wprop prop;

	if(entity->current_animation.frames->size > 0)
		entity->current_animation.current_frame %= entity->current_animation.frames->size;

	int* current_cell = stack_get(entity->current_animation.frames, entity->current_animation.current_frame);

	prop.width = entity->profile->width;
	prop.height = entity->profile->height;
	prop.direction = entity->direction;
	prop.texture = entity->profile->texture;
	prop.position = entity->position;

	if(current_cell != NULL)
		prop.cell = *current_cell;
	else
		prop.cell = 0;

	return prop;
}

void entity_sortByDistanceFromParent(Stack* working_stack, Entity* parent_entity){
	if(parent_entity == NULL) return;
	if(working_stack == NULL) return;

	for(size_t i = 0; i < working_stack->size; i++){
		Entity* entity = *((Entity**) stack_get(working_stack, i));
		entity->parent_entity = parent_entity;
	}

	qsort(working_stack->stack, working_stack->size, working_stack->memb_size, entity_compareDistance);
}

void entity_jump(Entity* entity, double jump_velocity){
	if(!entity->is_on_ground) return;

	entity->is_on_ground = 0;
	entity->velocity.z = jump_velocity;
}

void entity_setRelativeAcceleration(Entity* entity, double x, double y){
	entity->acceleration = vec3_create(0, 0, 0);

	entity->acceleration.x = sin(entity->direction) * y + cos(entity->direction) * x;
	entity->acceleration.y = cos(entity->direction) * y - sin(entity->direction) * x;
}

int entity_isLookingAt(Entity* viewer, Entity* enemy, World* world){
	double ang_min, ang_max;

	entity_getMinAndMaxAngle(viewer, enemy, &ang_min, &ang_max);

	Vec3 viw_max = viewer->position;
	Vec3 ene_max = enemy->position;

	viw_max.z += viewer->profile->height/2 - 0.01;
	ene_max.z += enemy->profile->height/2 - 0.01;

	if(entity_testLine(world, viw_max, ene_max)){
		double ang_player = viewer->direction;

		while(ang_player < 0) ang_player += 2 * PI;

		ang_player = fmod(ang_player, 2 * PI);

		if(ang_min < ang_player && ang_player < ang_max) return 1;

		return 0;
	}

	return 0;
}

int entity_testLineOfSight(Entity* viewer, Entity* enemy, World* world){
	double ang_min, ang_max;

	entity_getMinAndMaxAngle(viewer, enemy, &ang_min, &ang_max);

	Vec3 viw_max = viewer->position;
	Vec3 ene_max = enemy->position;

	viw_max.z += viewer->profile->height - 0.01;
	ene_max.z += enemy->profile->height - 0.01;

	if(entity_testLine(world, viw_max, ene_max)){
		double ang = atan2(enemy->position.x - viewer->position.x, enemy->position.y - viewer->position.y);
		double ang_player = viewer->direction;

		if(ang < 0) ang += 2 * PI;
		while(ang_player < 0) ang_player += 2 * PI;

		ang_player = fmod(ang_player, 2 * PI);

		if(ang_min < ang && ang < ang_max) return 1;

		return 0;
	}

	return 0;
}

void entity_searchForEntity(Stack* search_stack, Stack* entity_stack, Stack* type_stack){
	for(size_t i = 0; i < entity_stack->size; i++){
		Entity* current_entity = *((Entity**) stack_get(entity_stack, i));

		for(size_t j = 0; j < type_stack->size; j++){
			char* type = *((char**) stack_get(type_stack, j));

			if(!strcmp(current_entity->profile->type, type))
				stack_push(search_stack, &current_entity);
		}
	}
}

void entity_solveCollisionForEntities(Entity* entity, Entity* fixer){
	if(entity == NULL || fixer == NULL) return;
	if(!entity_checkCollisionWithEntity(entity, fixer)) return;

	Vec3 delta = vec3_sub(entity->position, fixer->position);
	delta = vec3_normalize(delta);

	double max_radius_ent = entity->profile->width / 2;
	double max_radius_fix = fixer->profile->width / 2;

	delta = vec3_mul(delta, max_radius_ent + max_radius_fix);

	entity->position = vec3_add(fixer->position, delta);
}

void entity_findCollisionOnStack(Entity* entity, Stack* entity_stack, Stack* type_stack){
	for(size_t i = 0; i < entity_stack->size; i++){
		Entity* current_entity = *((Entity**) stack_get(entity_stack, i));

		if(current_entity == entity) continue;

		for(size_t j = 0; j < type_stack->size; j++){
			char* type = *((char**) stack_get(type_stack, j));

			if(!strcmp(current_entity->profile->type, type) && entity_checkCollisionWithEntity(entity, current_entity))
				stack_push(entity->working_stack, &current_entity);
		}
	}
}

void entity_update(Entity* entity, Lcontext* context){
	if(entity->kill) return;
	/* applying animations */
	entity_updateAnimation(entity, context->delta_time);

	entity->has_collided_with_world = NO_AXIS;
	stack_clean(entity->working_stack);

	/* applying gravity */

	entity->velocity.z -= entity->gravity * context->delta_time;

	/* checking if we are over max velocity */

	double old_z = entity->velocity.z;
	entity->velocity.z = 0;

	if(vec3_size(entity->velocity) < MIN_VELOCITY)
		entity->velocity = vec3_create(0, 0, 0);

	if(vec3_size(entity->velocity) > entity->max_velocity_abs)
		entity->velocity = vec3_mul(vec3_normalize(entity->velocity), entity->max_velocity_abs);

	entity->velocity.z = old_z;

	old_z = entity->velocity.z;
	entity->velocity.z = 0;

	if(entity->is_on_ground)
		entity_accelerateGround(entity, context->delta_time);
	else{
		entity_accelerateAir(entity, context->delta_time);
	}

	entity->velocity.z = old_z;

	entity_applyCollisionWorld(entity, context->world, context->delta_time);

	logic_callUpdateFunction(entity, context);
}

void entity_destroy(Entity* entity){
	if(entity != NULL){
		etable_destroy(entity->properties);
		stack_destroy(entity->working_stack);
		stack_destroy(entity->current_animation.frames);
		free(entity);
	}
}
