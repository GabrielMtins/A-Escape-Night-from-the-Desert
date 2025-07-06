local damager = require("scripts/damager")
local DAMAGE_COOLDOWN = 2

local PISTOL_COOLDOWN_TIME = 0.4
local SHOTGUN_COOLDOWN_TIME = 0.8
local SUB_COOLDOWN_TIME = 0.1
local MESSAGE_MAX_TIME = 2

local PISTOL_MAGAZINE = 10
local SHOTGUN_MAGAZINE = 10
local SUB_MAGAZINE = 30

local weapon_pistol = {
	num_bullet = 0,
	damage = 20,
	cooldown = PISTOL_COOLDOWN_TIME,
	timer = 100
}

local weapon_shotgun = {
	num_bullet = 0,
	damage = 50,
	cooldown = SHOTGUN_COOLDOWN_TIME,
	timer = 100
}

local weapon_sub = {
	num_bullet = 0,
	damage = 20,
	cooldown = SUB_COOLDOWN_TIME,
	timer = 100
}

function OnCreate(entity)
	entity_setMaxVelocity(entity, 8)
	entity_setMaxVelocityAbs(entity, 12)
	entity_setFriction(entity, 10)
	entity_setGroundAcceleration(entity, 80)
	entity_setAirAcceleration(entity, 10)
	entity_setGravity(entity, 10)

	local properties = {
		health = 100,
		key_pressed = false,

		weapons = {
			weapon_pistol, weapon_shotgun, weapon_sub
		},

		current_weapon = 1,
		message_to_hud = nil,
		message_timer = 0,
		end_level = false,

		damage = {
			let = false
		}
	}

	entity_setProperties(entity, properties)
end

function OnUpdate(context, entity)
	if damager.DamageItself(context, entity, DAMAGE_COOLDOWN) then
		context_playSfx(context, 3)
	end

	OnControl(entity)
	UpdateGun(context, entity)
	TestCollisionOnGuns(context, entity)

	if input_getMouseDown("left") then
		OnGun(context, entity)
	end
	
	local properties = entity_getProperties(entity)

	if properties.message_to_hud ~= nil then
		properties.message_timer = properties.message_timer + context_getElapsedTime(context)
	end

	if properties.message_timer > MESSAGE_MAX_TIME then
		properties.message_timer = 0
		properties.message_to_hud = nil
	end

	local has_collided_with_world = entity_hasCollidedWithWorld(entity)

	if has_collided_with_world == X_AXIS then
		local velocity = entity_getVelocity(entity)
		local accel = entity_getAcceleration(entity)

		if (accel.x > 0 and velocity.x < 0) or (accel.x < 0 and velocity.x > 0) then
			velocity.x = 0
		end

		entity_setVelocity(entity, velocity)
	end

	if has_collided_with_world == Y_AXIS then
		local velocity = entity_getVelocity(entity)

		local accel = entity_getAcceleration(entity)

		if (accel.y > 0 and velocity.y < 0) or (accel.y < 0 and velocity.y > 0) then
			velocity.y = 0
		end

		entity_setVelocity(entity, velocity)
	end

	entity_setProperties(entity, properties)
end

function OnGun(context, entity)
	local damage = WeaponShootByEntity(context, entity)

	if damage ~= 0 and context_searchEntityType(context, "enemy_zombie", "enemy_thrower", "enemy_slime") then
		context_sortByDistanceToParent(context, entity)
		local enemy = context_getSearchedEntity(context)

		repeat
			if entity_isLookingAtEntity(context, entity, enemy) then
				damager.DoDamage(enemy, damage)
			end

			enemy = context_getSearchedEntity(context)
		until enemy == nil
	end
end

function UpdateGun(context, entity)
	local properties = entity_getProperties(entity)

	UpdateWeaponTimer(context, properties.weapons[properties.current_weapon])

	entity_setProperties(entity, properties)
end

function UpdateWeaponTimer(context, weapon)
	weapon.timer = weapon.timer + context_getElapsedTime(context)
end

function WeaponShootByEntity(context, entity)
	local properties = entity_getProperties(entity)
	local value = 0

	value = WeaponShoot(properties.weapons[properties.current_weapon])

	if value ~= 0 then
		if properties.current_weapon == 1 then
			context_playSfx(context, 0) -- shotgun sfx
		elseif properties.current_weapon == 2 then
			context_playSfx(context, 1) -- shotgun sfx
		elseif properties.current_weapon == 3 then
			context_playSfx(context, 2) -- shotgun sfx
		end
	end

	entity_setProperties(entity, properties)

	return value
end

function WeaponShoot(weapon)
	if weapon.timer > weapon.cooldown then
		if weapon.num_bullet > 0 then
			weapon.timer = 0
			weapon.num_bullet = weapon.num_bullet - 1
			return weapon.damage
		end
	end

	return 0
end

function TestCollisionOnGuns(context, entity)
	local properties = entity_getProperties(entity)

	if entity_getCollisionWithType(context, entity, "weapon_pistol") then
		local collided = entity_getEntityCollided(entity)

		while collided ~= nil do
			entity_kill(collided)
			properties.weapons[1].num_bullet = properties.weapons[1].num_bullet + PISTOL_MAGAZINE
			collided = entity_getEntityCollided(entity)
		end
	end

	if entity_getCollisionWithType(context, entity, "weapon_shotgun") then
		local collided = entity_getEntityCollided(entity)

		while collided ~= nil do
			entity_kill(collided)
			properties.weapons[2].num_bullet = properties.weapons[2].num_bullet + SHOTGUN_MAGAZINE
			collided = entity_getEntityCollided(entity)
		end
	end

	if entity_getCollisionWithType(context, entity, "weapon_sub") then
		local collided = entity_getEntityCollided(entity)

		while collided ~= nil do
			entity_kill(collided)
			properties.weapons[3].num_bullet = properties.weapons[3].num_bullet + SUB_MAGAZINE
			collided = entity_getEntityCollided(entity)
		end
	end

	if entity_getCollisionWithType(context, entity, "prop_health") then
		local collided = entity_getEntityCollided(entity)

		while collided ~= nil do
			entity_kill(collided)
			properties.health = properties.health + 20
			if properties.health > 100 then properties.health = 100 end
			collided = entity_getEntityCollided(entity)
		end
	end

	if entity_getCollisionWithType(context, entity, "trigger_end") then
		local collided = entity_getEntityCollided(entity)

		while collided ~= nil do
			entity_kill(collided)
			properties.end_level = true
			collided = entity_getEntityCollided(entity)
		end
	end

	entity_setProperties(entity, properties)
end

function OnControl(entity)
	local properties = entity_getProperties(entity)
	local x = 0
	local y = 0

	if input_getKeyDown(KEY_W) then
		y = y + 1
	end

	if input_getKeyDown(KEY_S) then
		y = y - 1
	end

	if input_getKeyDown(KEY_A) then
		x = x - 1
	end

	if input_getKeyDown(KEY_D) then
		x = x + 1
	end

	if input_getKeyDown(KEY_SPACE) then
		entity_jump(entity, 5)
	end

	if input_getKeyDown(KEY_1) then
		properties.current_weapon = 1
	end

	if input_getKeyDown(KEY_2) then
		properties.current_weapon = 2
	end

	if input_getKeyDown(KEY_3) then
		properties.current_weapon = 3
	end

	entity_setRelativeAcceleration(entity, x, y)

	entity_setProperties(entity, properties)
end
