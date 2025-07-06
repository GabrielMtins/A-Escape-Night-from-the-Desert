local M = {}

function M.DamageItself(context, entity, damage_cooldown)
	local properties = entity_getProperties(entity)
	local has_damaged = false
	
	if properties == nil then
		properties = {}
	end

	if properties.damage == nil then
		properties.damage = {
			let = false,
			timer = 0,
			health = 10
		}
	end

	if properties.health == nil then
		properties.health = 100
	end

	if properties.damage.let then
		if properties.damage.timer == 0 then
			properties.health = properties.health - properties.damage.health
			has_damaged = true
		end

		properties.damage.timer = properties.damage.timer + context_getElapsedTime(context)

		if properties.damage.timer > damage_cooldown then
			properties.damage.let = false
			properties.damage.timer = 0 
		end
	else
		properties.damage.timer = 0 
	end

	entity_setProperties(entity, properties)

	return has_damaged
end

function M.DoDamage(entity, damage)
	local properties = entity_getProperties(entity)
	
	if properties == nil then
		properties = {}
	end

	if properties.damage == nil then
		properties.damage = {
			let = false,
			timer = 0,
			health = 0
		}
	end

	properties.damage.let = true
	properties.damage.health = damage

	entity_setProperties(entity, properties)
end

function M.DamageParticleSpread(context, entity, num_particle)
	for i = 1, num_particle, 1 do
		local particle = context_addEntity(context, "particle_blood")
		local velocity = entity_getVelocity(particle)
		local position = entity_getPosition(entity)
		local dir = math.random() * 2 * math.pi
		local modulus = math.random() * 4

		position.z = position.z + 0.3

		velocity.x = velocity.x + math.cos(dir) * modulus
		velocity.y = velocity.y + math.sin(dir) * modulus
		velocity.z = velocity.z + modulus * 1.5

		entity_setPosition(particle, position)
		entity_setVelocity(particle, velocity)
	end
end

return M
