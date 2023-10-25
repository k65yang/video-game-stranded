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

	// Create the projectile
	// TODO: offset projectile location a little so it doesn't get created on top of player
	createProjectile(renderer, {player_x, player_y}, player_angle);
	weapon_component->can_fire = false;
	weapon_component->elapsed_last_shot_time_ms = 0.f;
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
	registry.projectiles.emplace(entity);

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
