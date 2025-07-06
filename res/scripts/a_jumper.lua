g_player = nil

function OnCreate(entity)
	math.randomseed(os.time())
	entity_setMaxVelocity(entity, 5)
	entity_setMaxVelocityAbs(entity, 5)
	entity_setFriction(entity, 10)
	entity_setGroundAcceleration(entity, 80)
	entity_setAirAcceleration(entity, 80)
	entity_setGravity(entity, 10)

	local properties = {
		health = 100
	}

	local animation = {0, 1, 2, 3, 4, 3, 2, 1}

	entity_playAnimation(entity, animation, 12)

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	local properties = entity_getProperties(entity)

	entity_jump(entity, 2)
	
	if g_player == nil then
		if context_searchEntityType(context, "player") then
			g_player = context_getSearchedEntity(context)
			local pos = entity_getPosition(g_player)
		end
	end

	if entity_getDistanceSqrToEntity(entity, g_player) < 25 then
		entity_setDirectionToEntity(entity, g_player)
		entity_setRelativeAcceleration(entity, 0, 1)
	end

	if entity_hasCollidedWithWorld(entity) ~= NO_AXIS then
		local dir = entity_getDirection(entity)
		dir = dir + math.random() * math.pi / 4
		entity_setDirection(entity, dir)
		entity_setRelativeAcceleration(entity, 0, 1)
	end

	entity_setProperties(entity, properties)
end
