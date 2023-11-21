#include <string> 
#include <stdexcept>
#include <random>

#include "weapons_system.hpp"

void WeaponsSystem::step(float elapsed_ms) {
	if (!weapon_component || weapon_component->can_fire)
		return;

	weapon_component->elapsed_last_shot_time_ms += elapsed_ms;
	if (weapon_component->fire_rate < weapon_component->elapsed_last_shot_time_ms)
		weapon_component->can_fire = true;
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
	// can change the amount ammo 
	weapon.ammo_count = 10; 
	// Set tracking for the newly created weapon
	active_weapon_type = weapon_type;
	active_weapon_entity = entity;
	weapon_component = &weapon;
    return entity;
}

void WeaponsSystem::fireWeapon(float player_x, float player_y, float player_angle) {
	if (active_weapon_type == ITEM_TYPE::WEAPON_NONE)
		return;
	if (!weapon_component || !weapon_component->can_fire || weapon_component->ammo_count <= 0)
		return;

	// TODO: offset projectile location a little so it doesn't get created on top of player
	switch(active_weapon_type){
		case ITEM_TYPE::WEAPON_SHURIKEN:
			weapon_component->ammo_count--;
			fireShuriken(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_CROSSBOW:
			weapon_component->ammo_count--;
			fireCrossbow(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_SHOTGUN:
			// decrement ammunition count
			weapon_component->ammo_count -= 5;
			fireShotgun(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_MACHINEGUN:
			weapon_component->ammo_count--;
			fireMachineGun(player_x, player_y, player_angle);
			break;
		default:
			throw(std::runtime_error("Error: Failed to fire weapon because unknown weapon equipped"));
	}


	printf("Ammunition count: %d\n", weapon_component->ammo_count);

	// disable firing if ammo is empty 
	if (weapon_component->ammo_count == 0) {
		weapon_component->can_fire = false;
		}

		
}

void WeaponsSystem::fireShuriken(float player_x, float player_y, float angle) {
	// one of the upgrades is "double-shot", but we cannot easily throw one after another
	// instead we offset the second shuriken so it looks like we threw one after another
	float offset = 0.5f;

	switch(weapon_level[active_weapon_type]) {
		case 0:
			createProjectile(renderer, {player_x, player_y}, angle);
			weapon_component->can_fire = false;
			weapon_component->elapsed_last_shot_time_ms = 0.f;
			break;
		default:
			createProjectile(renderer, { player_x, player_y }, angle);
			createProjectile(renderer, { player_x + offset * cos(angle), player_y + offset * sin(angle) }, angle);
			weapon_component->can_fire = false;
			weapon_component->elapsed_last_shot_time_ms = 0.f;
				
			break;
	}
}

void WeaponsSystem::fireCrossbow(float player_x, float player_y, float angle) {
	createProjectile(renderer, {player_x, player_y}, angle);
	weapon_component->can_fire = false;
	weapon_component->elapsed_last_shot_time_ms = 0.f;
}

void WeaponsSystem::fireShotgun(float player_x, float player_y, float angle) {
	// determine how many shotgun pellets to fire
	int num_bullets;
	float max_spread_angle = 0.523599f; // 30 degrees
	switch(weapon_level[active_weapon_type]) {
		case 0:
			num_bullets = 5;
			break;
		default: // default case is max level
			num_bullets = 12;
			break;
	}

	for (int i=0; i < num_bullets; i++){
		float bullet_angle = angle -(max_spread_angle/2) + (max_spread_angle/num_bullets)*i;
		createProjectile(renderer, {player_x, player_y}, bullet_angle);
	}

	weapon_component->can_fire = false;
	weapon_component->elapsed_last_shot_time_ms = 0.f;
}

void WeaponsSystem::fireMachineGun(float player_x, float player_y, float angle) {
	// This is the max angle from player direction. Double for total range.
	float max_recoil_angle;
	switch(weapon_level[active_weapon_type]) {
		case 0:
			max_recoil_angle = 0.261799f; // 15 degrees
			break;
		default: // default case is max level
			max_recoil_angle = 0.174533f; // 10 degrees
			break;
	}
	float stddev = max_recoil_angle/2;
	float range_min = angle - max_recoil_angle;
	float range_max = angle + max_recoil_angle;

	// The following approximates recoil for the machine gun.
	// It is based on a gaussian distribution about the current angle
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<float> distribution(angle, stddev);

	float angle_with_recoil;
	do {
        angle_with_recoil = distribution(gen);
    } while (angle_with_recoil < range_min || angle_with_recoil > range_max);

	createProjectile(renderer, {player_x, player_y}, angle_with_recoil);
}

void WeaponsSystem::applyWeaponEffects(Entity proj, Entity mob) {
	// Determine the weapon (and upgrade level) that the projectile came from
	Projectile& projectile = registry.projectiles.get(proj);
	Weapon& weapon = registry.weapons.get(projectile.weapon);

	// Apply the weapon effects to the mob if necessary
	if (weapon.weapon_type == ITEM_TYPE::WEAPON_CROSSBOW && weapon_level[weapon.weapon_type] >= 1) {
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

Entity WeaponsSystem::createProjectile(RenderSystem* renderer, vec2 pos, float angle) {
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
	
	// printf("player x: %f, player y: %f \n", pos.x, pos.y);


	// Initialize the collider (MUST BE DONE AFTER MOTION COMPONENT)
	
	// Set projectile to use a hard coded smaller box collider instead of scale from motion. 
	// Will change when projectile texture are filalized.

	createProjectileCollider(entity, vec2{0.2f,0.2f});

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
