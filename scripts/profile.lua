function OnLoadProfiles(profile_list)
	player = {
		type = "player",
		texture = "materials/entities/player.png",
		script = "scripts/player.lua",
		width = 0.5,
		height = 1,
	}

	enemy_zombie = {
		type = "enemy_zombie",
		texture = "materials/entities/zombie.png",
		script = "scripts/zombie.lua",
		width = 0.5,
		height = 1,
		cell_width = 32,
		cell_height = 64
	}

	enemy_thrower = {
		type = "enemy_thrower",
		texture = "materials/entities/thrower.png",
		script = "scripts/thrower.lua",
		width = 0.5,
		height = 1,
		cell_width = 32,
		cell_height = 64
	}

	enemy_thrower_ball = {
		type = "enemy_thrower_ball",
		texture = "materials/entities/thrower_ball.png",
		script = "scripts/thrower_ball.lua",
		width = 0.25,
		height = 0.25,
		cell_width = 16,
		cell_height = 16
	}

	enemy_slime = {
		type = "enemy_slime",
		texture = "materials/entities/slime.png",
		script = "scripts/slime.lua",
		width = 0.5,
		height = 1,
		cell_width = 32,
		cell_height = 64
	}

	particle_blood = {
		type = "particle_blood",
		texture = "materials/entities/blood.png",
		script = "scripts/particle_blood.lua",
		width = 0.1,
		height = 0.1
	}

	prop_tree = {
		type = "prop_desert_tree",
		texture = "materials/props/desert_tree.png",
		width = 0.4,
		height = 0.4
	}

	prop_cactus = {
		type = "prop_cactus",
		texture = "materials/props/cactus.png",
		width = 0.5,
		height = 1
	}

	prop_health = {
		type = "prop_health",
		texture = "materials/props/health_kit.png",
		width = 0.5,
		height = 0.5,
		cell_width = 64,
		cell_height = 64
	}

	trigger_door = {
		type = "trigger_door",
		texture = "materials/entities/door.png",
		script = "scripts/door.lua",
		width = 0.25,
		height = 0.25,
		cell_width = 32,
		cell_height = 32
	}

	trigger_end = {
		type = "trigger_end",
		texture = "materials/entities/end.png",
		script = "scripts/trigger_end.lua",
		width = 1.42,
		height = 1.42,
		cell_width = 32,
		cell_height = 32
	}

	weapon_pistol = {
		type = "weapon_pistol",
		texture = "materials/entities/weapon_pistol.png",
		width = 0.8,
		height = 18 / 28 * 0.6
	}

	weapon_shotgun = {
		type = "weapon_shotgun",
		texture = "materials/entities/weapon_shotgun.png",
		width = 1,
		height = 18 / 55
	}

	weapon_sub = {
		type = "weapon_sub",
		texture = "materials/entities/weapon_sub.png",
		width = 0.8,
		height = 23 / 55 * 0.8
	}

	addProfile(profile_list, player)
	addProfile(profile_list, enemy_zombie)
	addProfile(profile_list, enemy_thrower)
	addProfile(profile_list, enemy_slime)
	addProfile(profile_list, enemy_thrower_ball)
	addProfile(profile_list, particle_blood)
	addProfile(profile_list, trigger_door)
	addProfile(profile_list, trigger_end)

	-- props --
	addProfile(profile_list, prop_tree)
	addProfile(profile_list, prop_cactus)
	addProfile(profile_list, prop_health)

	-- weapons --
	addProfile(profile_list, weapon_pistol)
	addProfile(profile_list, weapon_shotgun)
	addProfile(profile_list, weapon_sub)
end
