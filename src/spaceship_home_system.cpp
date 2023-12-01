#include "spaceship_home_system.hpp"

void SpaceshipHomeSystem::step(float elapsed_ms) {

};

void SpaceshipHomeSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
}

void SpaceshipHomeSystem::resetSpaceshipHomeSystem(int food_storage, int ammo_storage) {
	Entity camera = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera);
	vec2 camera_pos = camera_motion.position;

	// Create spaceship home
	spaceship_home = createSpaceshipHome(camera_pos, food_storage, ammo_storage);

	// Create food storage elements
	food_item = createSpaceshipHomeItem(getNewPosition(camera_pos, FOOD_ITEM_OFFSET), TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD);
	food_storage_bar = createBar(renderer, getNewPosition(camera_pos, FOOD_STORAGE_BAR_OFFSET), food_storage, BAR_TYPE::FOOD_STORAGE);
	food_storage_bar_frame = createFrame(renderer, getNewPosition(camera_pos, FOOD_STORAGE_BAR_FRAME_OFFSET), FRAME_TYPE::BAR_FRAME); 

	// Create ammo storage elements
	ammo_item = createSpaceshipHomeItem(getNewPosition(camera_pos, AMMO_ITEM_OFFSET), TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO);
	ammo_storage_bar = createBar(renderer, getNewPosition(camera_pos, AMMO_STORAGE_BAR_OFFSET), ammo_storage, BAR_TYPE::AMMO_STORAGE);
	ammo_storage_bar_frame = createFrame(renderer, getNewPosition(camera_pos, AMMO_STORAGE_BAR_FRAME_OFFSET), FRAME_TYPE::BAR_FRAME);
};

void SpaceshipHomeSystem::enterSpaceship(Entity player_health_bar, Entity player_food_bar, Entity player_ammo_bar, Entity player_weapon) {
	SpaceshipHome& spaceship_home_info = registry.spaceshipHomes.get(spaceship_home);
	Entity player = registry.players.entities[0]; 
	Player& player_info = registry.players.get(player);
	Motion& player_health_bar_motion = registry.motions.get(player_health_bar);
	Motion& player_food_bar_motion = registry.motions.get(player_food_bar);
	Motion& food_storage_bar_motion = registry.motions.get(food_storage_bar);
	Motion& ammo_storage_bar_motion = registry.motions.get(ammo_storage_bar); 

	updateSpaceshipHomeUI();

	// Regenerate health
	player_info.health = PLAYER_MAX_HEALTH;
	player_health_bar_motion.scale = HEALTH_BAR_SCALE;

	// Regenerate food
	regenerateStat(player_info.food, spaceship_home_info.food_storage, PLAYER_MAX_FOOD);
	player_food_bar_motion.scale = vec2(((float)player_info.food / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);
	food_storage_bar_motion.scale = vec2(TURKEY_BAR_SCALE[0], ((float)spaceship_home_info.food_storage / (float) SPACESHIP_HOME_MAX_FOOD_STORAGE) * TURKEY_BAR_SCALE[1]);

	// Check if player has weapon equipped
	if (registry.weapons.has(player_weapon)) {
		auto& weapon = registry.weapons.get(player_weapon);
		auto& w_motion = registry.motions.get(player_ammo_bar);
		
		// Regenerate ammo
		regenerateStat(weapon.ammo_count, spaceship_home_info.ammo_storage, PLAYER_MAX_AMMO);
		w_motion.scale = vec2(((float)weapon.ammo_count / (float)PLAYER_MAX_AMMO) * AMMO_BAR_SCALE[0], AMMO_BAR_SCALE[1]);
		ammo_storage_bar_motion.scale = vec2(AMMO_STORAGE_SCALE[0], ((float)spaceship_home_info.ammo_storage / (float) SPACESHIP_HOME_MAX_AMMO_STORAGE) * AMMO_STORAGE_SCALE[1]);
	}

	player_info.is_home = true;
};

bool SpaceshipHomeSystem::isHome() {
    return registry.players.components[0].is_home;
};


Entity SpaceshipHomeSystem::createSpaceshipHome(vec2 position, int food_storage, int ammo_storage) {
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


void SpaceshipHomeSystem::updateSpaceshipHomeUI() {
	Entity camera = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera);
	vec2 camera_pos = camera_motion.position;

	// No camera shake 
	camera_motion.angle = 0;
	camera_motion.scale = { 1.f, 1.f };

	// Update spaceship home screen position
	Motion& spaceship_home_motion = registry.motions.get(spaceship_home);
	spaceship_home_motion.position = camera_motion.position;

	// Update food storage element positions
	Motion& food_item_motion = registry.motions.get(food_item);
	Motion& food_storage_bar_motion = registry.motions.get(food_storage_bar);
	Motion& food_storage_bar_frame_motion = registry.motions.get(food_storage_bar_frame);
	food_item_motion.position = getNewPosition(camera_pos, FOOD_ITEM_OFFSET);
	food_storage_bar_motion.position = getNewPosition(camera_pos, FOOD_STORAGE_BAR_OFFSET);
	food_storage_bar_frame_motion.position = getNewPosition(camera_pos, FOOD_STORAGE_BAR_FRAME_OFFSET);

	// Update ammo storage element positions
	Motion& ammo_item_motion = registry.motions.get(ammo_item);
	Motion& ammo_storage_bar_motion = registry.motions.get(ammo_storage_bar); 
	Motion& ammo_storage_bar_frame_motion = registry.motions.get(ammo_storage_bar_frame); 
	ammo_item_motion.position = getNewPosition(camera_pos, AMMO_ITEM_OFFSET);
	ammo_storage_bar_motion.position = getNewPosition(camera_pos, AMMO_STORAGE_BAR_OFFSET);
	ammo_storage_bar_frame_motion.position = getNewPosition(camera_pos, AMMO_STORAGE_BAR_FRAME_OFFSET);
};

void SpaceshipHomeSystem::regenerateStat(int& stat, int& storage, int max_stat_value) {
	int missing = max_stat_value - stat;
	
	if (missing <= storage) {
		storage -= missing;
		stat = max_stat_value;
	} else {
		stat += storage;
		storage = 0;
	}
};

void SpaceshipHomeSystem::updateBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor, bool is_stat) {

};

vec2 SpaceshipHomeSystem::getNewPosition(vec2 camera_pos, vec2 offset) {
	return { camera_pos.x + offset.x, camera_pos.y + offset.y };
}
