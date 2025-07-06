local M = {}

function CreateProperties(properties)
	if properties.is_looking_for_player == nil then 
		properties.is_looking_for_player = false
	end

	if properties.idling == nil then 
		properties.idling = false
	end
end

function M.NotLookingForPlayer(context, entity)
	local properties = entity_getProperties(entity)

	CreateProperties(properties)

	if not properties.is_looking_for_player then
		if properties.idling then
			if math.random() < 0.1 then
				properties.idling = false
			end

			entity_setRelativeAcceleration(entity, 0, 0)

		else
			if math.random() < 0.3 then
				properties.idling = true
				entity_setDirection(entity, math.random() * math.pi * 2)
			end

			entity_setRelativeAcceleration(entity, 0, 1)
		end
	end

	entity_setProperties(entity, properties)

	if entity_hasCollidedWithWorld(entity) ~= NO_AXIS then
		local dir = entity_getDirection(entity)
		dir = dir + math.random() * math.pi / 4;
		entity_setDirection(entity, dir)
		entity_setRelativeAcceleration(entity, 0, 1)
	end
end

function M.LookingForPlayer(context, entity, g_player, max_distance_sqr_player)
	local properties = entity_getProperties(entity)

	if entity_getDistanceSqrToEntity(entity, g_player) < max_distance_sqr_player
		and entity_testLineOfSight(context, entity, g_player) then
		properties.is_looking_for_player = true
	else
		properties.is_looking_for_player = false
	end

	if properties.is_looking_for_player then
		entity_setDirectionToEntity(entity, g_player)
		entity_setRelativeAcceleration(entity, 0, 1)
	end

	entity_setProperties(entity, properties)
end

function M.FindForPlayer(context)
	if context_searchEntityType(context, "player") then
		local g_player = context_getSearchedEntity(context)
		return g_player
	end
end

function M.SolveCollisionForType(context, entity, type)
	if entity_getCollisionWithType(context, entity, type) then
		local collided = entity_getEntityCollided(entity)
		repeat
			entity_solveCollisionWithEntity(entity, collided)
			collided = entity_getEntityCollided(entity)
		until collided == nil
	end
end

return M
