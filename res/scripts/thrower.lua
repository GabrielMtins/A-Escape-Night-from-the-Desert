local damager = require("scripts/damager")
local enemy_logic = require("scripts/enemy_logic")

g_player = nil
local max_distance_sqr_player = 200
local min_distance_sqr_player = 25
local max_timer_to_shoot = 0.7
local damage_cooldown = 0

math.randomseed(os.time())

function OnCreate(entity)
	entity_setMaxVelocity(entity, 5)
	entity_setMaxVelocityAbs(entity, 5)
	entity_setFriction(entity, 10)
	entity_setGroundAcceleration(entity, 80)
	entity_setAirAcceleration(entity, 10)
	entity_setGravity(entity, 10)

	local properties = {
		health = 100,
		is_looking_for_player = false,
		idling = true,
		timer_to_shoot = 0
	}

	entity_playAnimation(entity, {0, 1, 2, 3, 2, 1}, 10)

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	if g_player == nil then
		g_player = enemy_logic.FindForPlayer(context)
	end
	
	local properties = entity_getProperties(entity)

	if properties.health <= 0 then
		entity_kill(entity)
	end

	if damager.DamageItself(context, entity, damage_cooldown) then
		damager.DamageParticleSpread(context, entity, 20)
	end

	enemy_logic.NotLookingForPlayer(context, entity)
	LookingForPlayer(context, entity, g_player, max_distance_sqr_player)

	entity_solveCollisionWithEntity(entity, g_player)
	enemy_logic.SolveCollisionForType(context, entity, "enemy_thrower")
end

function LookingForPlayer(context, entity)
	local properties = entity_getProperties(entity)

	local distance = entity_getDistanceSqrToEntity(entity, g_player)

	if distance < max_distance_sqr_player and entity_testLineOfSight(context, entity, g_player) then
		properties.is_looking_for_player = true
	else
		properties.is_looking_for_player = false
	end

	if properties.is_looking_for_player then
		properties.timer_to_shoot = properties.timer_to_shoot + context_getElapsedTime(context)

		entity_setDirectionToEntity(entity, g_player)

		if distance < min_distance_sqr_player then
			entity_setRelativeAcceleration(entity, 1, 0)
		else
			entity_setRelativeAcceleration(entity, 0, 1)
		end

		if properties.timer_to_shoot > max_timer_to_shoot then
			properties.timer_to_shoot = 0
			local pos = entity_getPosition(entity)
			pos.z = pos.z + 0.5

			local ball = context_addEntity(context, "enemy_thrower_ball")

			entity_setPosition(ball, pos)
			entity_setDirectionToEntity(ball, g_player)
		end
	else
		properties.timer_to_shoot = 0
	end

	entity_setProperties(entity, properties)
end
