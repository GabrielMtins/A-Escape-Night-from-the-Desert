local damager = require("scripts/damager")
local time_to_live = 5

function OnCreate(entity)
	entity_setMaxVelocity(entity, 6)
	entity_setMaxVelocityAbs(entity, 6)
	entity_setFriction(entity, 10)
	entity_setGroundAcceleration(entity, 8000)
	entity_setAirAcceleration(entity, 8000)
	entity_setGravity(entity, 0)

	local properties = {
		timer_to_live = 0
	}

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	local properties = entity_getProperties(entity)
	properties.timer_to_live = properties.timer_to_live + context_getElapsedTime(context)

	entity_setRelativeAcceleration(entity, 0, 1)

	if entity_getCollisionWithType(context, entity, "player") then
		local player = entity_getEntityCollided(entity)

		damager.DoDamage(player, 10)

		entity_kill(entity)
	end

	if entity_hasCollidedWithWorld(entity) ~= NO_AXIS then
		entity_kill(entity)
	end

	if properties.timer_to_live > time_to_live then
		entity_kill(entity)
	end

	entity_setProperties(entity, properties)
end
