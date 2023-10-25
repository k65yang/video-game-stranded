#include "weapons_system.hpp"

void WeaponsSystem::step(float elapsed_ms) {
	if (weapon_component->can_fire)
		return;

	weapon_component->elapsed_last_shot_time_ms += elapsed_ms;
	if (weapon_component->fire_rate < weapon_component->elapsed_last_shot_time_ms)
		weapon_component->can_fire = true;
}

Entity WeaponsSystem::createWeapon(ITEM_TYPE weapon_type) {
    // Reserve an entity
	auto entity = Entity();

	// Initialize the weapon
	auto& weapon = registry.weapons.emplace(entity);
	weapon.weapon_type = weapon_type;
	weapon.can_fire = true;
	weapon.elapsed_last_shot_time_ms = 0.f;

	switch (weapon_type) {
		case ITEM_TYPE::WEAPON_NONE:
			weapon.fire_rate = 0;
			weapon.projectile_velocity = 0.f;
			weapon.projectile_damage = 0;
			break;

		case ITEM_TYPE::WEAPON_GENERIC:
			weapon.fire_rate = 1000.f;
			weapon.projectile_velocity = 10.f;
			weapon.projectile_damage = 100;
			break;
	
		default:
			// TODO: better way of handling this
			printf("Debug info: Item is not a weapon. Ignoring.\n");
			break;
		}
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

	// Set projectile direction
	// Motion& projectile_motion = registry.motions.get(entity);
	// projectile_motion.angle = player_motion.angle;
	// projectile_motion.velocity = weapon.projectile_velocity * vec2(cos(projectile_motion.angle), sin(projectile_motion.angle));

	// Control weapon fire rate
	// weapon_component = registry.weapons.get(active_weapon_entity);
	weapon_component->can_fire = false;
	// printf("%f\n", weapon_component.projectile_damage);
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
	motion.velocity = weaponProjectileVelocityMap[active_weapon_type] * vec2(cos(angle), sin(angle));;
	motion.position = pos;

	// Add this projectile to the projectiles registry
	registry.projectiles.emplace(entity);

	// TODO: Change this later
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::WEAPON;

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}
