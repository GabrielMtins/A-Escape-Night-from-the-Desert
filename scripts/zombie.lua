local damager = require("scripts/damager")
local enemy_logic = require("scripts/enemy_logic")

g_player = nil

local max_distance_sqr_player = 160
local damage_cooldown = -1
math.randomseed(os.time())

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
		idling = true
	}

	local animation = {0, 1}
	entity_playAnimation(entity, animation, 5)

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

	DamagePlayer(context, entity)

	if damager.DamageItself(context, entity, damage_cooldown) then
		damager.DamageParticleSpread(context, entity, 20)
	end

	enemy_logic.NotLookingForPlayer(context, entity)
	enemy_logic.LookingForPlayer(context, entity, g_player, max_distance_sqr_player)

	entity_solveCollisionWithEntity(entity, g_player)

	enemy_logic.SolveCollisionForType(context, entity, "enemy_zombie")
end

function DamagePlayer(context, entity)
	if entity_getCollisionWithType(context, entity, "player") then
		damager.DoDamage(g_player, 10)
	end
end

function FindForPlayer(context)
	if g_player == nil then
		if context_searchEntityType(context, "player") then
			g_player = context_getSearchedEntity(context)
			local pos = entity_getPosition(g_player)
		end
	end
end
