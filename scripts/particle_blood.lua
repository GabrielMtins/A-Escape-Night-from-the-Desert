local time_to_live = 2.5

function OnCreate(entity)
	entity_setMaxVelocity(entity, 1000)
	entity_setMaxVelocityAbs(entity, 1000)
	entity_setFriction(entity, 10)
	entity_setGroundAcceleration(entity, 0)
	entity_setAirAcceleration(entity, 0)
	entity_setGravity(entity, 10)

	local properties = {
		timer_to_live = 0
	}

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	local properties = entity_getProperties(entity)

	properties.timer_to_live = properties.timer_to_live + context_getElapsedTime(context) 

	if properties.timer_to_live > time_to_live then
		entity_kill(entity)
	end

	entity_setProperties(entity, properties)
end
