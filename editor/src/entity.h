#ifndef ENTITY_H
#define ENTITY_H

#include "world.h"
#include "profile.h"
#include "context.h"
#include "vec3.h"

enum axis{
	NO_AXIS = 0,
	X_AXIS,
	Y_AXIS
};

typedef struct{
	Stack* frames;
	size_t fps;
	double timer;
	size_t current_frame;
} Eanimation;

typedef struct{
	Profile* profile;

	Vec3 position;
	Vec3 velocity;
	Vec3 acceleration;
	int is_on_ground;
	int kill;
	double direction;

	int has_collided_with_world;

	double max_velocity_abs;
	double max_velocity;
	double air_acceleration;
	double ground_acceleration;
	double friction;
	double gravity;

	Stack* properties; /* must be of etable type */
	Stack* working_stack;
	void* parent_entity; /* used for sorting distance */

	Eanimation current_animation;
} Entity;

Entity* entity_create(Profile* profile);

Wprop entity_toProp(Entity* entity);

void entity_sortByDistanceFromParent(Stack* working_stack, Entity* parent_entity);

void entity_jump(Entity* entity, double jump_velocity);

void entity_setRelativeAcceleration(Entity* entity, double x, double y);

int entity_isLookingAt(Entity* viewer, Entity* enemy, World* world);

int entity_testLineOfSight(Entity* viewer, Entity* enemy, World* world);

void entity_searchForEntity(Stack* search_stack, Stack* entity_stack, Stack* type_stack);

void entity_solveCollisionForEntities(Entity* entity, Entity* fixer);

void entity_findCollisionOnStack(Entity* entity, Stack* entity_stack, Stack* type_stack);

void entity_update(Entity* entity, Lcontext* context);

void entity_destroy(Entity* entity);

#endif
