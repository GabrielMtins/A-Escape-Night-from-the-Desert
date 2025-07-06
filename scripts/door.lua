local enemy_logic = require("scripts/enemy_logic")
g_player = nil

function OnCreate(entity)
	local properties = {
		has_set_tile = false,
		og_height = 0,
		timer = 0,
		idling = true,
		closed = false
	}

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	entity_playAnimation(entity, {1, 1}, 10)

	if g_player == nil then
		g_player = enemy_logic.FindForPlayer(context)
	end

	local properties = entity_getProperties(entity)
	local player_prop = entity_getProperties(g_player)

	if not properties.has_set_tile then
		local pos = entity_getPosition(entity)
		local tile = context_getWorldTile(context, pos.x, pos.y)

		properties.og_height = tile.bottom_height

		properties.has_set_tile = true
	end

	if entity_getDistanceSqrToEntity(entity, g_player) < 16 then
		player_prop.message_to_hud = "Press E to open or close doors"
		
		if input_getKeyDown(KEY_E) then
			properties.idling = false
		end
	end

	if not properties.idling then
		local pos = entity_getPosition(entity)
		local tile = context_getWorldTile(context, pos.x, pos.y)
		properties.timer = properties.timer + context_getElapsedTime(context)

		local offset = Smooth(properties.timer)

		if properties.closed then offset = 1 - offset end

		tile.bottom_height = properties.og_height - offset
		tile.offset_texture = offset

		context_setWorldTile(context, pos.x, pos.y, tile)
	end

	if properties.timer > 1 then
		properties.closed = not properties.closed
		properties.timer = 0
		properties.idling = true
	end

	entity_setProperties(entity, properties)
	entity_setProperties(g_player, player_prop)
end

function Smooth(i)
	if i > 1 then return 1 end
	if i < 0 then return 0 end

	return ((6*i - 15)*i + 10)*i*i*i;
end
