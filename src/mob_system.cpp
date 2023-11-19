#include "mob_system.hpp"

void MobSystem::step(float elapsed_ms) {
    
};

void MobSystem::spawn_mobs() {
    // NOTE: do not add more mobs than there are grid cells available in the zone!!!
	std::map<ZONE_NUMBER,int> zone_mob_numbers = {
		{ZONE_0, 5},    // zone 1 has 5 mobs
		{ZONE_1, 5},	// zone 2 has 5 mobs
	};

	std::vector<vec2> zone_mob_locations = terrain->get_mob_spawn_locations(zone_mob_numbers);

	for (const auto& spawn_location: zone_mob_locations) {
		create_mob(spawn_location, MOB_TYPE::DISRUPTOR);
	}
}

void MobSystem::apply_mob_attack_effects(Entity player, Entity mob) {
	Mob& mob_info = registry.mobs.get(mob);

	if (mob_info.type == MOB_TYPE::DISRUPTOR) {
		apply_knockback(player, mob, 500.f, 10.f);
	}
}

Entity MobSystem::create_mob(vec2 mob_position, MOB_TYPE mob_type) {
    auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::MOB_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = mob_position;
	motion.scale = vec2({ 1, 1 });

	// Initialize path
	registry.paths.emplace(entity);

	// Classify this entity as a mob
	auto& mob_info = registry.mobs.emplace(entity);
	mob_info.damage = mob_damage_map.at(mob_type);
	mob_info.aggro_range = mob_aggro_range_map.at(mob_type);
	mob_info.health = mob_health_map.at(mob_type);
	mob_info.speed_ratio = mob_speed_ratio_map.at(mob_type);
	mob_info.curr_cell = terrain->get_cell(motion.position);
	mob_info.type = mob_type;

	// Initialize the collider
	createMeshCollider(entity, GEOMETRY_BUFFER_ID::MOB001_MESH, renderer);

	TEXTURE_ASSET_ID texture = mob_textures_map.at(mob_type);
	switch (mob_type) {
		case MOB_TYPE::SLIME:	// Handle slime differently because it has a sprite sheet animation
			registry.renderRequests.insert(
				entity,
				{	
					texture,
					EFFECT_ASSET_ID::SPRITESHEET,
					GEOMETRY_BUFFER_ID::MOB_SPRITE,
					RENDER_LAYER_ID::LAYER_1 
				}
			);
			break;
		default:
			registry.renderRequests.insert(
				entity,
				{	
					texture,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE,
					RENDER_LAYER_ID::LAYER_1 
				}
			);
			break;
	}

	return entity;
};

void MobSystem::apply_knockback(Entity player, Entity mob, float duration_ms, float knockback_speed_ratio) {
	// Apply knock back only if player is not knocked back already
	if (registry.playerKnockbackEffects.has(player)) {
		return;
	}

	// Create PlayerKnockbackEffect component
	PlayerKnockbackEffect& playerKnockbackEffect = registry.playerKnockbackEffects.emplace(player);
	playerKnockbackEffect.duration_ms = duration_ms;
	playerKnockbackEffect.elapsed_knockback_time_ms = 0.f;

	// Change velocity of player so that they move in the opposite direction of the mob (i.e. knock them back)
	Motion& player_motion = registry.motions.get(player);
	Motion& mob_motion = registry.motions.get(mob);
	float angle = atan2(player_motion.position.y - mob_motion.position.y, player_motion.position.x - mob_motion.position.x);
	player_motion.velocity[0] = cos(angle) * knockback_speed_ratio;
    player_motion.velocity[1] = sin(angle) * knockback_speed_ratio;
};
