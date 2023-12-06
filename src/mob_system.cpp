#include "mob_system.hpp"

void MobSystem::step(float elapsed_ms) {
    
};

// NOTE: do not add more mobs than there are grid cells available in the zone!!!
void MobSystem::spawn_mobs() {
	// spawn slimes
	std::map<ZONE_NUMBER,int> zone_mob_slime = {
		{ZONE_1, 5},    
		{ZONE_2, 10},	
		{ZONE_3, 10},	
	};
	std::vector<vec2> zone_slime_locations = terrain->get_mob_spawn_locations(zone_mob_slime);

	for (const auto& spawn_location: zone_slime_locations) {
		create_mob(spawn_location, MOB_TYPE::SLIME);
	}

	// spawn ghosts
	std::map<ZONE_NUMBER,int> zone_mob_ghost = {
		{ZONE_1, 0},    
		{ZONE_2, 7},	
		{ZONE_3, 10},
	};
	std::vector<vec2> zone_ghost_locations = terrain->get_mob_spawn_locations(zone_mob_ghost);

	for (const auto& spawn_location: zone_ghost_locations) {
		create_mob(spawn_location, MOB_TYPE::GHOST);
	}

	// spawn brutes
	std::map<ZONE_NUMBER,int> zone_mob_brute = {
		{ZONE_1, 0},    
		{ZONE_2, 5},	
		{ZONE_3, 15},
	};
	std::vector<vec2> zone_brute_locations = terrain->get_mob_spawn_locations(zone_mob_brute);

	for (const auto& spawn_location: zone_brute_locations) {
		create_mob(spawn_location, MOB_TYPE::BRUTE);
	}

	// spawn DISRUPTOR
	std::map<ZONE_NUMBER,int> zone_mob_disruptor = {
		{ZONE_1, 0},    
		{ZONE_2, 0},	
		{ZONE_3, 30},		// I'm in danger
	};
	std::vector<vec2> zone_disruptor_locations = terrain->get_mob_spawn_locations(zone_mob_disruptor);

	for (const auto& spawn_location: zone_disruptor_locations) {
		create_mob(spawn_location, MOB_TYPE::DISRUPTOR);
	}

	// TODO: for milestone 4
	// // spawn turret
	// std::map<ZONE_NUMBER,int> zone_mob_turret = {
	// 	{ZONE_0, 5},
	// 	{ZONE_1, 0},    
	// 	{ZONE_2, 0},	
	// 	{ZONE_3, 0},
	// };
	// std::vector<vec2> zone_turret_locations = terrain->get_mob_spawn_locations(zone_mob_turret);

	// for (const auto& spawn_location: zone_turret_locations) {
	// 	create_mob(spawn_location, MOB_TYPE::TURRET);
	// }
}

void MobSystem::apply_mob_attack_effects(Entity player, Entity mob) {
	Mob& mob_info = registry.mobs.get(mob);

	if (mob_info.type == MOB_TYPE::DISRUPTOR) {
		apply_knockback(player, mob, 500.f, 10.f);
		apply_inaccuracy(player, 5000.f, 0.5f);
	}
}

Entity MobSystem::create_mob(vec2 mob_position, MOB_TYPE mob_type, int current_health) {
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
	mob_info.is_tracking_player = false;
	vec2 health_position = { mob_position.x, mob_position.y - 1 };
	if (current_health != 0) {
		mob_info.health = current_health;
		// Create a health bar for the mob
		mob_info.health_bar = create_mob_health_bar(renderer, health_position, current_health, mob_type);
	}
	else {
		mob_info.health = mob_health_map.at(mob_type);
		// Create a health bar for the mob
		mob_info.health_bar = create_mob_health_bar(renderer, health_position, mob_health_map.at(mob_type), mob_type);
	}
	mob_info.speed_ratio = mob_speed_ratio_map.at(mob_type);
	mob_info.curr_cell = terrain->get_cell(motion.position);
	mob_info.type = mob_type;


	// Initialize the collider
	physics->createMeshCollider(entity, GEOMETRY_BUFFER_ID::MOB001_MESH, renderer);

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

Entity MobSystem::create_mob_health_bar(RenderSystem* renderer, vec2 position, int amount, MOB_TYPE type) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2(((float)amount / (float)mob_health_map.at(type)) * 3.5, 0.7);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::RED_BLOCK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1
		}
	);

	return entity;
}

void MobSystem::apply_knockback(Entity player, Entity mob, float duration_ms, float knockback_speed_ratio) {
	// Apply knock back only if player is not knocked back already
	if (registry.playerKnockbackEffects.has(player)) {
		return;
	}

	// Create PlayerKnockbackEffect component
	PlayerKnockbackEffect& playerKnockbackEffect = registry.playerKnockbackEffects.emplace(player);
	playerKnockbackEffect.duration_ms = duration_ms;
	playerKnockbackEffect.elapsed_knockback_time_ms = 0.f;

	// Change velocity of player so that they move in the opposite they were just travelling (i.e. knock them back)
	Motion& player_motion = registry.motions.get(player);
	Motion& mob_motion = registry.motions.get(mob);
	float angle = atan2(player_motion.position.y - mob_motion.position.y, player_motion.position.x - mob_motion.position.x);
	player_motion.velocity[0] = cos(angle) * knockback_speed_ratio;
    player_motion.velocity[1] = sin(angle) * knockback_speed_ratio;

	printf("KNOCKBACK APPLIED\n");
};

void MobSystem::apply_inaccuracy(Entity player, float duration_ms, float inaccuracy_percent) {
	bool already_applied = registry.playerInaccuracyEffects.has(player);

	// Create PlayerInaccuracyEffects component only if it does not exist already
	PlayerInaccuracyEffect& playerInaccuracyEffect = already_applied ? registry.playerInaccuracyEffects.get(player) : registry.playerInaccuracyEffects.emplace(player);
	playerInaccuracyEffect.duration_ms = duration_ms;
	playerInaccuracyEffect.elapsed_inaccuracy_time_ms = 0.f;
	playerInaccuracyEffect.inaccuracy_percent = inaccuracy_percent;

	printf("INACCURACY APPLIED\n");
};
