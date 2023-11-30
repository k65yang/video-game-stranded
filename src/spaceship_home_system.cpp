#include "spaceship_home_system.hpp"

void SpaceshipHomeSystem::step(float elapsed_ms) {

};

Entity SpaceshipHomeSystem::createSpaceshipHome(vec2 position, bool is_inside, int food_storage, int ammo_storage) {
    auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
    motion.scale = { target_resolution.x / tile_size_px, target_resolution.y / tile_size_px };

	// Add spaceship home to spaceship home registry
	auto& spaceshipHome = registry.spaceshipHomes.emplace(entity);
	spaceshipHome.is_inside = is_inside;
	spaceshipHome.food_storage = food_storage;
	spaceshipHome.ammo_storage = ammo_storage;
	
	registry.renderRequests.insert(
		entity,
		{ 
            TEXTURE_ASSET_ID::SPACEHOME,
		    EFFECT_ASSET_ID::TEXTURED,
		    GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 
        }
    );

	return entity;
};

Entity SpaceshipHomeSystem::createSpaceshipHomeItem(vec2 position, TEXTURE_ASSET_ID texture) {
    auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
    motion.scale = { target_resolution.x / tile_size_px * 0.3, target_resolution.y / tile_size_px * 0.3 };

	registry.renderRequests.insert(
		entity,
		{ 
            texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 
        }
    );

	return entity;
};

void SpaceshipHomeSystem::enterSpaceship() {

};

void SpaceshipHomeSystem::resetSpaceshipHome() {
    
};

bool SpaceshipHomeSystem::isHome() {
    return true;
};

void SpaceshipHomeSystem::updateSpaceshipHomeUI() {

};

void SpaceshipHomeSystem::regenerateStat(int& stat, int& storage, int max_stat_value) {

};

void SpaceshipHomeSystem::updateBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor, bool is_stat) {

};
