#include <string> 
#include <stdexcept>
#include <random>

#include "weapons_system.hpp"

void WeaponsSystem::init(RenderSystem* renderer_arg, PhysicsSystem* physics_arg) {
	// Set the render and physics system
	this->renderer = renderer_arg;
	this->physics = physics_arg;

	// Create all weapons
	createAllWeapons();
}

void WeaponsSystem::step(float elapsed_ms) {
	// Iterate through each weapon component and update
	for (uint i = 0; i < registry.weapons.size(); i++) {
		Weapon& weapon = registry.weapons.components[i];

		// Continue if the weapon can fire
		if (weapon.can_fire)
			continue;

		// Continue if there is no ammo (weapon should already be in a cannot fire state)
		if (weapon.ammo_count <= 0)
			continue;

		// Update the the last shot time to see if the weapon is able to fire
		weapon.elapsed_last_shot_time_ms += elapsed_ms;
		if (weapon.fire_rate < weapon.elapsed_last_shot_time_ms)
			weapon.can_fire = true;
	}
}

void WeaponsSystem::resetWeaponsSystem() {
	// Unset the active weapons tracking variables
	active_weapon_component = nullptr;
	active_weapon_type = ITEM_TYPE::WEAPON_NONE;

	// Delete weapons (should already be deleted if restart_game is called)
	while (registry.weapons.entities.size() > 0)
		registry.remove_all_components_of(registry.weapons.entities.back());

	// Recreate all weapons
	createAllWeapons();
}

Entity WeaponsSystem::createWeapon(ITEM_TYPE weapon_type) {
	// Item must be a weapon
	assert((isValidWeapon(weapon_type)) && "Error: Attempted to create weapon from non-weapon type");

    // Reserve an entity
	auto entity = Entity();

	// Initialize the weapon
	auto& weapon = registry.weapons.emplace(entity);
	weapon.weapon_type = weapon_type;
	weapon.can_fire = true;
	weapon.elapsed_last_shot_time_ms = 0.f;
	weapon.fire_rate = weapon_fire_rate_map[weapon_type];
	weapon.projectile_velocity = weapon_projectile_velocity_map[weapon_type];
	weapon.projectile_damage = weapon_damage_map[weapon_type];
	weapon.ammo_count = weapon_ammo_capacity_map[weapon_type]; 
	

    return entity;
}

void WeaponsSystem::createAllWeapons() {
	createWeapon(ITEM_TYPE::WEAPON_SHURIKEN);
	createWeapon(ITEM_TYPE::WEAPON_CROSSBOW);
	createWeapon(ITEM_TYPE::WEAPON_SHOTGUN);
	createWeapon(ITEM_TYPE::WEAPON_MACHINEGUN);
}

void WeaponsSystem::setWeaponAttributes(
	ITEM_TYPE weapon_type, 
	bool weapon_can_fire, 
	int weapon_ammo_count, 
	int weapon_level) {
	// Iterate through the weapon components until the specified weapon is found
	for (uint i = 0; i < registry.weapons.size(); i++) {
		Weapon& weapon = registry.weapons.components[i];
		if (weapon.weapon_type == weapon_type) {
			weapon.can_fire = weapon_can_fire;
			weapon.ammo_count = weapon_ammo_count;
			weapon.level = weapon_level;
			break;
		}
	}
}

int WeaponsSystem::increaseAmmo(ITEM_TYPE weapon_type, int amount) {
	// Iterate through the weapon components until the specified weapon is found
	for (uint i = 0; i < registry.weapons.size(); i++) {
		Weapon& weapon = registry.weapons.components[i];
		if (weapon.weapon_type == weapon_type) {
			// Cannot increase ammo to more than the max capacity
			int amount_increased = 
				weapon.ammo_count + amount <= weapon_ammo_capacity_map[weapon_type] ? 
				amount : 
				weapon_ammo_capacity_map[weapon_type] - weapon.ammo_count;
			
			weapon.ammo_count += amount_increased;
			updateAmmoBar();

			// adjust sidebar ammo for previous weapon
			bool hasAmmo = updateSideBars(weapon_type);

			// set previous weapon indicator to be visible
			setIndicatorAlpha(weapon_type, true, hasAmmo);

			return amount_increased;
		}
	}
	return 0;
}

void WeaponsSystem::upgradeWeapon() {
	if (active_weapon_component)
		active_weapon_component->level++;
}

void WeaponsSystem::upgradeWeapon(ITEM_TYPE weapon_type) {
	// Iterate through the weapon components and upgrade the specified weapon
	for (uint i = 0; i < registry.weapons.size(); i++) {
		Weapon& weapon = registry.weapons.components[i];
		if (weapon.weapon_type == weapon_type) {
			weapon.level++;
			break;
		}
	}
}

void WeaponsSystem::setActiveWeapon(ITEM_TYPE weapon_type) {
	if (weapon_type == ITEM_TYPE::WEAPON_NONE) {
		active_weapon_component = nullptr;
		active_weapon_type = ITEM_TYPE::WEAPON_NONE;
	}

	for (Entity weapon_entity: registry.weapons.entities) {
		Weapon& weapon = registry.weapons.get(weapon_entity);

		// Find the correct weapon in the weapons vector
		if (weapon.weapon_type == weapon_type) {

			// adjust sidebar ammo for previous weapon
			bool hasAmmo = updateSideBars(active_weapon_type);

			// set previous weapon indicator to be visible
			setIndicatorAlpha(active_weapon_type, true, hasAmmo);


			// Set the weapon indicator first
			if (active_weapon_component) { // Delete existing weapon/ammo indicator
				registry.remove_all_components_of(weapon_indicator);
				registry.remove_all_components_of(ammo_indicator);
			}
			// Set active weapon tracking
			active_weapon_type = weapon_type;
			active_weapon_entity = weapon_entity;
			active_weapon_component = &weapon;

			// Add the new UI elements for the weapon
			weapon_indicator = createWeaponIndicator(renderer, weapon_indicator_textures_map[weapon_type], vec2{ -10.f, -6.f }, vec2{ 2.5, 2.5 });
			ammo_indicator = createAmmoBar(renderer, { -10.f, -4.f }, { 3.0, 0.4 });

			// set selected weapon indicator to invisible
			setIndicatorAlpha(active_weapon_type, false, false);
			

			
			// New scale for ammo bar. 
			updateAmmoBar();

			break;
		}
	}
}

ITEM_TYPE WeaponsSystem::getActiveWeapon() {
	return active_weapon_type;
}

int WeaponsSystem::getActiveWeaponAmmoCount() {
	return !active_weapon_component ? 0 : active_weapon_component->ammo_count;
}

ITEM_TYPE WeaponsSystem::fireWeapon(float player_x, float player_y, float player_angle) {
	if (active_weapon_type == ITEM_TYPE::WEAPON_NONE || !active_weapon_component)
		return ITEM_TYPE::WEAPON_NONE;
	if (!active_weapon_component || !
		active_weapon_component->can_fire || 
		active_weapon_component->ammo_count <= 0)
		return ITEM_TYPE::WEAPON_NONE;

	Entity player = registry.players.entities[0];
	if (registry.playerInaccuracyEffects.has(player)) {
		PlayerInaccuracyEffect& playerInaccuracyEffect = registry.playerInaccuracyEffects.get(player);
		applyProjectileInaccuracy(player_angle, playerInaccuracyEffect.inaccuracy_percent);
	}

	// TODO: offset projectile location a little so it doesn't get created on top of player
	switch(active_weapon_type){
		case ITEM_TYPE::WEAPON_SHURIKEN:
			active_weapon_component->ammo_count--;
			fireShuriken(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_CROSSBOW:
			active_weapon_component->ammo_count--;
			fireCrossbow(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_SHOTGUN:
			// decrement ammunition count
			active_weapon_component->ammo_count -= 1;
			fireShotgun(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_MACHINEGUN:
			active_weapon_component->ammo_count--;
			fireMachineGun(player_x, player_y, player_angle);
			break;
		default:
			throw(std::runtime_error("Error: Failed to fire weapon because unknown weapon equipped"));
	}

	// disable firing if ammo is empty 
	if (active_weapon_component->ammo_count == 0) {
		active_weapon_component->can_fire = false;
	}

	// New scale for ammo bar. 
	updateAmmoBar();

	return active_weapon_type;
}

void WeaponsSystem::fireShuriken(float player_x, float player_y, float angle) {
	// one of the upgrades is "double-shot", but we cannot easily throw one after another
	// instead we offset the second shuriken so it looks like we threw one after another
	float offset = 0.5f;

	switch(active_weapon_component->level) {
		case 0:
			createProjectile(renderer, physics, {player_x, player_y}, angle);
			active_weapon_component->can_fire = false;
			active_weapon_component->elapsed_last_shot_time_ms = 0.f;
			break;
		default:
			createProjectile(renderer, physics, { player_x, player_y }, angle);
			createProjectile(renderer, physics, { player_x + offset * cos(angle), player_y + offset * sin(angle) }, angle);
			active_weapon_component->can_fire = false;
			active_weapon_component->elapsed_last_shot_time_ms = 0.f;
				
			break;
	}
}

void WeaponsSystem::fireCrossbow(float player_x, float player_y, float angle) {
	createProjectile(renderer, physics, {player_x, player_y}, angle);
	active_weapon_component->can_fire = false;
	active_weapon_component->elapsed_last_shot_time_ms = 0.f;
}

void WeaponsSystem::fireShotgun(float player_x, float player_y, float angle) {
	// determine how many shotgun pellets to fire
	int num_bullets;
	float max_spread_angle = 0.523599f; // 30 degrees
	switch(active_weapon_component->level) {
		case 0:
			num_bullets = 5;
			break;
		default: // default case is max level
			num_bullets = 12;
			break;
	}

	for (int i=0; i < num_bullets; i++){
		float bullet_angle = angle -(max_spread_angle/2) + (max_spread_angle/num_bullets)*i;
		createProjectile(renderer, physics, {player_x, player_y}, bullet_angle);
	}

	active_weapon_component->can_fire = false;
	active_weapon_component->elapsed_last_shot_time_ms = 0.f;
}

void WeaponsSystem::fireMachineGun(float player_x, float player_y, float angle) {
	float inaccuracy_percent;
	switch(active_weapon_component->level) {
		case 0:
			inaccuracy_percent = 0.4f;
			break;
		default: // default case is max level
			inaccuracy_percent = 0.3f;
			break;
	}
	
	applyProjectileInaccuracy(angle, inaccuracy_percent);
	
	createProjectile(renderer, physics, {player_x, player_y}, angle);
}

void WeaponsSystem::applyWeaponEffects(Entity proj, Entity mob) {
	// Determine the weapon (and upgrade level) that the projectile came from
	Projectile& projectile = registry.projectiles.get(proj);
	Weapon& weapon = registry.weapons.get(projectile.weapon);


	// Apply the weapon effects to the mob if necessary
	if (weapon.weapon_type == ITEM_TYPE::WEAPON_CROSSBOW && active_weapon_component->level >= 1) {
		applySlow(mob, 10000.f, 0.1);
	}
}




void WeaponsSystem::applySlow(Entity mob, float duration_ms, float slow_ratio) {
	// If the slow effect exists, assume that it is already applied.
	// Valid assumption because of slow fire rate, there is enough time to apply the slow
	bool already_slowed = registry.mobSlowEffects.has(mob) ? true : false;

	// Create mobSlowEffect component only if it does not exist already
	MobSlowEffect& mobSlowEffect = already_slowed ? registry.mobSlowEffects.get(mob) : registry.mobSlowEffects.emplace(mob);
	mobSlowEffect.applied = already_slowed ? true : false;
	mobSlowEffect.duration_ms = duration_ms;
	mobSlowEffect.elapsed_slow_time_ms = 0.f;
	mobSlowEffect.slow_ratio = slow_ratio;
}

void WeaponsSystem::applyProjectileInaccuracy(float& angle, float inaccuracy_percent) {
	// Determine max angle using inaccuracy percentage
	// Limit max angle to 45 degrees so projectiles go in the general direction the player is facing
	float max_angle = radians(45.f * inaccuracy_percent);
	
	float stddev = max_angle/2;
	float range_min = angle - max_angle;
	float range_max = angle + max_angle;

	// The following approximates recoil
	// It is based on a gaussian distribution about the current angle
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<float> distribution(angle, stddev);

	float angle_with_inaccuracy;
	do {
        angle_with_inaccuracy = distribution(gen);
    } while (angle_with_inaccuracy < range_min || angle_with_inaccuracy > range_max);

	angle = angle_with_inaccuracy;
}

Entity WeaponsSystem::createProjectile(RenderSystem* renderer, PhysicsSystem* physics, vec2 pos, float angle) {
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	if (active_weapon_type == ITEM_TYPE::WEAPON_SHOTGUN || active_weapon_type == ITEM_TYPE::WEAPON_MACHINEGUN) {
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PEBBLE);
		registry.meshPtrs.emplace(entity, &mesh);
	}
	else {
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);
	}
	
	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.velocity = weapon_projectile_velocity_map[active_weapon_type] * vec2(cos(angle), sin(angle));;
	motion.position = pos;

	// Initialize the collider (MUST BE DONE AFTER MOTION COMPONENT)
	// Set projectile to use a hard coded smaller box collider instead of scale from motion. 
	// Will change when projectile texture are filalized.
	physics->createCustomsizeBoxCollider(entity, vec2{0.2f,0.2f});

	// Add this projectile to the projectiles registry
	auto& projectile = registry.projectiles.emplace(entity);
	projectile.weapon = active_weapon_entity;
	projectile.damage = weapon_damage_map[active_weapon_type];

	// For shotgun/machine gun pellets, we use the PEBBLE effect/geometry, not an actual texture
	if (active_weapon_type == ITEM_TYPE::WEAPON_SHOTGUN || active_weapon_type == ITEM_TYPE::WEAPON_MACHINEGUN) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // won't be used, but we still need something here
				EFFECT_ASSET_ID::PEBBLE,
				GEOMETRY_BUFFER_ID::PEBBLE,
				RENDER_LAYER_ID::LAYER_1 });
	} else {
		TEXTURE_ASSET_ID texture = projectile_textures_map[active_weapon_type];
		registry.renderRequests.insert(
			entity,
			{ texture,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER_ID::LAYER_1 });
	}

	return entity;
}

Entity WeaponsSystem::createAmmoBar(RenderSystem* renderer, vec2 position, vec2 scale) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale; 

	// Set up color component for alpha adjustmnet later
	auto& color = registry.colors.emplace(entity);
	color = vec4{ 1.0f };

	registry.screenUI.insert(entity, position);


	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::BROWN_BLOCK;
	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
}

Entity WeaponsSystem::createWeaponIndicator(RenderSystem* renderer, TEXTURE_ASSET_ID weapon_texture, vec2 position, vec2 scale) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale;

	// Set up color component for alpha adjustmnet later
	auto& color = registry.colors.emplace(entity);
	color = vec4{ 1.0f };

	registry.screenUI.insert(entity, position);

	registry.renderRequests.insert(
		entity,
		{ weapon_texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
}



bool WeaponsSystem::isValidWeapon(ITEM_TYPE test) {
	static const std::set<ITEM_TYPE> weapons_list{
		ITEM_TYPE::WEAPON_NONE,
		ITEM_TYPE::WEAPON_SHURIKEN,
		ITEM_TYPE::WEAPON_CROSSBOW,
		ITEM_TYPE::WEAPON_SHOTGUN,
		ITEM_TYPE::WEAPON_MACHINEGUN,
	};
	return weapons_list.count(test);
}

void WeaponsSystem::updateAmmoBar() {
	if (!active_weapon_component)
		return;
	
	Motion& ammo_motion = registry.motions.get(ammo_indicator);
	ammo_motion.scale = vec2(
		(
			(float)active_weapon_component->ammo_count / 
			(float)weapon_ammo_capacity_map[active_weapon_type]) * 3.f, 
			0.4f);
}



bool WeaponsSystem::updateSideIndicatorHelper(Entity weapon_indicator, Entity ammo_indicator, Weapon& weapon) {
	Motion& ammo_motion = registry.motions.get(ammo_indicator);
	
	ammo_motion.scale = vec2(((float)weapon.ammo_count / (float)weapon_ammo_capacity_map[weapon.weapon_type]) * 1.3f, 0.2f);

	auto& color = registry.colors.get(weapon_indicator);
	bool hasAmmo = true;

	
	// lower alpha when empty ammo
	if (weapon.ammo_count == 0) {
		color.a = 0.2f;
		hasAmmo = false;
	}
	

	return hasAmmo;
}

// update side bar given a weapon that has property change
bool WeaponsSystem::updateSideBars(ITEM_TYPE targetWeaponToUpdate) {
	bool hasAmmo = true;
	// Iterate through the weapon components until the specified weapon is found
	for (uint i = 0; i < registry.weapons.size(); i++) {
		Weapon& weapon = registry.weapons.components[i];
		if (weapon.weapon_type == targetWeaponToUpdate) {
			switch (targetWeaponToUpdate) {

				//update ammo indicator ui scale and weapon indicator ui color
				case ITEM_TYPE::WEAPON_SHURIKEN:
					hasAmmo = updateSideIndicatorHelper(shuriken_weapon_indicator, shuriken_ammo_indicator, weapon);
					break;
				case ITEM_TYPE::WEAPON_CROSSBOW:
					hasAmmo = updateSideIndicatorHelper(crossbow_weapon_indicator, crossbow_ammo_indicator, weapon);
					break;
				case ITEM_TYPE::WEAPON_SHOTGUN:
					hasAmmo = updateSideIndicatorHelper(shot_gun_weapon_indicator, shot_gun_ammo_indicator,weapon);
					break;
				case ITEM_TYPE::WEAPON_MACHINEGUN:
					hasAmmo = updateSideIndicatorHelper(machine_gun_weapon_indicator, machine_gun_ammo_indicator, weapon);
					break;

				default:
					break;

			}
		}
		
	}

	return hasAmmo;
}


void WeaponsSystem::createNonselectedWeaponIndicators() {
	vec2 shuriken_indicator_pos = {-11.0f, -2.0f};
	vec2 shuriken_indicator_size = vec2{ 1.0f };
	vec2 shuriken_ammo_pos = {shuriken_indicator_pos.x, shuriken_indicator_pos.y + 0.7f};
	vec2 shuriken_ammo_size = { 1.3f, 0.2f };
	float spaceBetweenIndicators = 1.5f;

	// NON SELECTED WEAPON INDICATORS
	shuriken_weapon_indicator = createWeaponIndicator(renderer, weapon_indicator_textures_map[ITEM_TYPE::WEAPON_SHURIKEN],
		shuriken_indicator_pos, shuriken_indicator_size);
	shuriken_ammo_indicator = createAmmoBar(renderer, shuriken_ammo_pos, shuriken_ammo_size);
	

	crossbow_weapon_indicator = createWeaponIndicator(renderer, weapon_indicator_textures_map[ITEM_TYPE::WEAPON_CROSSBOW], 
		{ shuriken_indicator_pos.x, shuriken_indicator_pos.y + spaceBetweenIndicators * 1 } , shuriken_indicator_size);
	crossbow_ammo_indicator = createAmmoBar(renderer, { shuriken_ammo_pos.x , shuriken_ammo_pos.y + spaceBetweenIndicators * 1 }, shuriken_ammo_size);
	

	shot_gun_weapon_indicator = createWeaponIndicator(renderer, weapon_indicator_textures_map[ITEM_TYPE::WEAPON_SHOTGUN],
		{ shuriken_indicator_pos.x, shuriken_indicator_pos.y + spaceBetweenIndicators * 2 }, shuriken_indicator_size);
	shot_gun_ammo_indicator = createAmmoBar(renderer, { shuriken_ammo_pos.x , shuriken_ammo_pos.y + spaceBetweenIndicators * 2 }, shuriken_ammo_size);
	

	machine_gun_weapon_indicator = createWeaponIndicator(renderer, weapon_indicator_textures_map[ITEM_TYPE::WEAPON_MACHINEGUN],
		{ shuriken_indicator_pos.x, shuriken_indicator_pos.y + spaceBetweenIndicators * 3 }, shuriken_indicator_size);
	machine_gun_ammo_indicator = createAmmoBar(renderer, { shuriken_ammo_pos.x , shuriken_ammo_pos.y + spaceBetweenIndicators * 3 }, shuriken_ammo_size);
	
	
	updateSideBars(ITEM_TYPE::WEAPON_SHURIKEN);
	updateSideBars(ITEM_TYPE::WEAPON_CROSSBOW);
	updateSideBars(ITEM_TYPE::WEAPON_SHOTGUN);
	updateSideBars(ITEM_TYPE::WEAPON_MACHINEGUN);

}

void WeaponsSystem::setIndicatorAlpha(ITEM_TYPE weapon_type, bool turnOn, bool hasAmmo) {
	float value;

	// dim when no ammo and trying to turn indicator on
	if (turnOn == true && hasAmmo == false) {
		value = 0.2f;
	}
	else {
		value = turnOn ? 1.0f : 0.0f;
	}

	// change the selected side indicator to transparent
	switch (weapon_type) {
	case ITEM_TYPE::WEAPON_SHURIKEN:
		registry.colors.get(shuriken_weapon_indicator).a = value;
		registry.colors.get(shuriken_ammo_indicator).a = value;
		break;
	case ITEM_TYPE::WEAPON_CROSSBOW:
		registry.colors.get(crossbow_weapon_indicator).a = value;
		registry.colors.get(crossbow_ammo_indicator).a = value;
		break;
	case ITEM_TYPE::WEAPON_SHOTGUN:
		registry.colors.get(shot_gun_weapon_indicator).a = value;
		registry.colors.get(shot_gun_ammo_indicator).a = value;
		break;
	case ITEM_TYPE::WEAPON_MACHINEGUN:
		registry.colors.get(machine_gun_weapon_indicator).a = value;
		registry.colors.get(machine_gun_ammo_indicator).a = value;
		break;

	default:
		break;
	}



	
}
