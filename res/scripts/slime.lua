local damager = require("scripts/damager")
local enemy_logic = require("scripts/enemy_logic")
local damage_cooldown = 0.1
local max_distance_sqr_player = 160
local g_player = nil

function OnCreate(entity)
	entity_setMaxVelocity(entity, 4)
	entity_setMaxVelocityAbs(entity, 4)
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

	entity_playAnimation(entity, {0, 1, 2, 1}, 5)

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	if g_player == nil then
		g_player = enemy_logic.FindForPlayer(context)
	end

	local properties = entity_getProperties(entity)

	if properties.health <= 0 then
		for i = 0, 2 * math.pi, math.pi/6 do
			local ball = context_addEntity(context, "enemy_thrower_ball")
			local anim = {1, 1, 1, 1, 1, 1}
			local pos = entity_getPosition(entity)
			pos.z = pos.z + 0.3

			entity_playAnimation(ball, anim, 10)
			entity_setDirection(ball, i)
			entity_setPosition(ball, pos)
		end

		entity_kill(entity)
	end

	if damager.DamageItself(context, entity, damage_cooldown) then
		damager.DamageParticleSpread(context, entity, 20)
	end

	enemy_logic.NotLookingForPlayer(context, entity)
	enemy_logic.LookingForPlayer(context, entity, g_player, max_distance_sqr_player)

	entity_solveCollisionWithEntity(entity, g_player)
	enemy_logic.SolveCollisionForType(context, entity, "enemy_slime")
end
