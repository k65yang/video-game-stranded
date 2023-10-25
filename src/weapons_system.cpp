#include <string> 

#include "weapons_system.hpp"

void WeaponsSystem::step(float elapsed_ms) {
	if (weapon_component->can_fire)
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

	// Set tracking for the newly created weapon
	active_weapon_type = weapon_type;
	active_weapon_entity = entity;
	weapon_component = &weapon;
    return entity;
}

void WeaponsSystem::fireWeapon(float player_x, float player_y, float player_angle) {
	if (active_weapon_type == ITEM_TYPE::WEAPON_NONE)
		return;
	if (!weapon_component->can_fire)
		return;

	// TODO: offset projectile location a little so it doesn't get created on top of player
	switch(active_weapon_type){
		case ITEM_TYPE::WEAPON_SHURIKEN:
			fireShuriken(player_x, player_y, player_angle);
			break;
		case ITEM_TYPE::WEAPON_CROSSBOW:
			fireCrossbow(player_x, player_y, player_angle);
			break;
		default:
			throw("Error: Filed to fire weapon because unknown weapon equipped");
	}
		
}

void WeaponsSystem::fireShuriken(float player_x, float player_y, float player_angle) {
	// one of the upgrades is "double-shot", but we cannot easily throw one after another
	// instead we offset the second shuriken so it looks like we threw one after another
	float offset = 0.5f;

	switch(weapon_level[active_weapon_type]) {
		case 0:
			createProjectile(renderer, {player_x, player_y}, player_angle);
			weapon_component->can_fire = false;
			weapon_component->elapsed_last_shot_time_ms = 0.f;
			break;
		case 1:
			createProjectile(renderer, {player_x, player_y}, player_angle);
			createProjectile(renderer, {player_x + offset * cos(player_angle), player_y + offset * sin(player_angle)}, player_angle);
			weapon_component->can_fire = false;
			weapon_component->elapsed_last_shot_time_ms = 0.f;
			break;
		default:
			throw("Error: Shuriken level not supported (level: " + std::to_string(weapon_level[active_weapon_type]) + ")");
	}
}

void WeaponsSystem::fireCrossbow(float player_x, float player_y, float player_angle) {
	createProjectile(renderer, {player_x, player_y}, player_angle);
	weapon_component->can_fire = false;
	weapon_component->elapsed_last_shot_time_ms = 0.f;
}

void WeaponsSystem::applyWeaponEffects(Entity proj, Entity mob) {
	// Determine the weapon (and upgrade level) that the projectile came from
	Projectile& projectile = registry.projectiles.get(proj);
	Weapon& weapon = registry.weapons.get(projectile.weapon);

	// Apply the weapon effects to the mob if necessary
	if (weapon.weapon_type == ITEM_TYPE::WEAPON_CROSSBOW && weapon_level[weapon.weapon_type] == 1) {
		applySlow(mob, 10.f, 0.1);
	}
}

void WeaponsSystem::applySlow(Entity mob, float duration_ms, float slow_ratio) {
	MobSlowEffect& mobSlowEffect = registry.mobSlowEffects.emplace(mob);
	mobSlowEffect.applied = false;
	mobSlowEffect.duration_ms = duration_ms;
	mobSlowEffect.elapsed_slow_time_ms = 0.f;
	mobSlowEffect.slow_ratio = slow_ratio;
}

Entity WeaponsSystem::createProjectile(RenderSystem* renderer, vec2 pos, float angle) {
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.velocity = weapon_projectile_velocity_map[active_weapon_type] * vec2(cos(angle), sin(angle));;
	motion.position = pos;

	// Add this projectile to the projectiles registry
	auto& projectile = registry.projectiles.emplace(entity);
	projectile.weapon = active_weapon_entity;
	projectile.damage = weapon_damage_map[active_weapon_type];

	// TODO: Change this later
	TEXTURE_ASSET_ID texture = projectile_textures_map[active_weapon_type];

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}

bool WeaponsSystem::isValidWeapon(ITEM_TYPE test) {
	static const std::set<ITEM_TYPE> weapons_list{
		ITEM_TYPE::WEAPON_NONE,
		ITEM_TYPE::WEAPON_SHURIKEN,
		ITEM_TYPE::WEAPON_CROSSBOW,
	};
	return weapons_list.count(test);
}
