function OnCreate()
	main_font = hud_loadFont("materials/fonts/default.ttf", 20)
	bg_texture = hud_loadTexture("materials/title/background.png")

	texture_weapons = {
		hud_loadTexture("materials/hud/pistol.png"),
		hud_loadTexture("materials/hud/shotgun.png"),
		hud_loadTexture("materials/hud/sub.png")
	}

	timer = 0
	max_time = 0.5
	tick = 0
	current_fps = 60
	start = true
	init = true

	state = "start"

	style = {
		font = main_font,
		highlight_color = {r  = 200, g = 200, b = 200, a = 255},
		bg_color = {r = 60, g = 60, b = 60, a = 255},
		clicked_color = {r = 50, g = 50, b = 50, a = 255},
		text_color = {r = 255, g = 255, b = 255, a = 255},
		outline_width = 2
	}

	current_level = 0

	levels = {
		"maps/map1.lua",
	}

	hud_setGameName("A Escape Night from the Desert");
end

function OnRender(context)
	if state == "start" then
		RenderTitleScreen(context)
	elseif state == "game" then
		RenderGameHud(context)
	elseif state == "game_over" then
		RenderGameOver(context)
	elseif state == "game_win" then
		RenderGameWin(context)
	end
end

function RenderTitleScreen(context)
	hud_drawTexture(bg_texture, nil, nil)
	hud_drawTextCentered(style.font, "A ESCAPE NIGHT FROM THE DESERT", 640, 80, style.text_color)
	hud_drawTextCentered(style.font, "Developed by Gabriel Martins", 1100, 680, style.text_color)

	local text_rect = {x = 100, y = 200, w = 300, h = 50}

	if hud_putButton(style, text_rect, "New") then
		state = "game"
		hud_loadWorld(context, "maps/map1.lua")

		hud_setPause(false)
	end
end

function RenderGameHud(context)
	local player = nil

	if context_searchEntityType(context, "player") then
		player = context_getSearchedEntity(context)
	end

	local prop = entity_getProperties(player)

	if not hud_isPaused() then
		local weapon = nil
		local current_weapon = 0
		local show_texture = true

		weapon = prop.weapons[prop.current_weapon]
		current_weapon = prop.current_weapon

		if prop.damage.let and math.floor(prop.damage.timer * 8) % 2 ~= 0 then
			show_texture = false
		end
		
		local current_frame = math.floor(weapon.timer / 0.1 * 3)

		if current_frame > 6 then current_frame = 0 end
		if current_frame > 5 then current_frame = 1 end
		if current_frame > 4 then current_frame = 2 end
		if current_frame > 3 then current_frame = 3 end

		local in_rect = {x = current_frame * 320, y = 0, w = 320, h = 180}
		local dst_rect = {x = 40, y = 60, w = 1280, h = 720}

		if show_texture then
			hud_drawTexture(texture_weapons[current_weapon], in_rect, dst_rect)
		end

		local rect_cross = {x = 636, y = 356, w = 4, h = 4}
		hud_drawRect(rect_cross, {r = 255, g = 0, b = 0, a = 255})
	else
		local text_rect = {x = 100, y = 100, w = 250, h = 60}

		if hud_putButton(style, text_rect, "Resume") then
			hud_setPause(false)
		end

		text_rect.y = text_rect.y + 80

		if hud_putButton(style, text_rect, "Back to title") then
			state = "start"
		end
	end

	if prop.health <= 0 then
		state = "game_over"
	end

	if prop.end_level then
		state = "game_win"
	end

	DrawPlayerStatus(context, player)
end

function RenderGameWin(context)
	hud_setPause(true)
	hud_drawTexture(bg_texture, nil, nil)
	hud_drawTextCentered(style.font, "YOU WIN", 640, 80, style.text_color)
	hud_drawTextCentered(style.font, "THE END", 640, 120, style.text_color)

	local text_rect = {x = 100, y = 100, w = 250, h = 60}

	if hud_putButton(style, text_rect, "Back to title") then
		state = "start"
	end
end

function RenderGameOver(context)
	hud_setPause(true)
	hud_drawTexture(bg_texture, nil, nil)
	hud_drawTextCentered(style.font, "GAME OVER", 640, 80, style.text_color)

	local text_rect = {x = 100, y = 100, w = 250, h = 60}

	if hud_putButton(style, text_rect, "Back to title") then
		state = "start"
	end
end

function DrawPlayerStatus(context, player)
	local health_rect = {x = 40, y = 600, w = 200, h = 60}
	local bullet_rect = {x = 260, y = 600, w = 200, h = 60}
	local color_rect = {r = 40, g = 40, b = 40, a = 120}
	local center = {x = health_rect.x + health_rect.w / 2, y = health_rect.y + health_rect.h / 2}

	local properties = entity_getProperties(player)

	hud_drawRect(health_rect, color_rect)
	hud_drawRect(bullet_rect, color_rect)
	
	local num_bullet = properties.weapons[properties.current_weapon].num_bullet

	hud_drawTextCentered(style.font, string.format("HEALTH: %i", properties.health), center.x, center.y, style.text_color)
	hud_drawTextCentered(style.font, string.format("BULLET: %i", num_bullet), center.x + 220, center.y, style.text_color)

	if properties.message_to_hud ~= nil then
		hud_drawTextCentered(style.font, properties.message_to_hud, 640, 80, style.text_color)
	end
end
