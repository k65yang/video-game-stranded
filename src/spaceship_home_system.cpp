#include "spaceship_home_system.hpp"

void SpaceshipHomeSystem::step(float elapsed_ms) {

};

void SpaceshipHomeSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
}

void SpaceshipHomeSystem::resetSpaceshipHomeSystem(Entity camera, bool is_home, int food_storage, int ammo_storage) {
	Motion& camera_motion = registry.motions.get(camera);
	float camera_pos_x = camera_motion.position.x;
	float camera_pos_y = camera_motion.position.y;

	// Create spaceship home
	spaceship_home = createSpaceshipHome(camera_motion.position, is_home, food_storage, ammo_storage);

	// Create food storage elements
	food_item = createSpaceshipHomeItem({ -5.5f + camera_pos_x, 0.f + camera_pos_y }, TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD);
	food_storage_bar = createBar(renderer, { -3.5f + camera_pos_x, camera_pos_y }, food_storage, BAR_TYPE::FOOD_STORAGE);
	food_storage_bar_frame = createFrame(renderer, { -3.49f + camera_pos_x, camera_pos_y }, FRAME_TYPE::BAR_FRAME); 

	// Create ammo storage elements
	ammo_item = createSpaceshipHomeItem({ 1.f + camera_pos_x, 0.5f + camera_pos_y }, TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO);
	ammo_storage_bar = createBar(renderer, { 4.5f + camera_pos_x,  0.5f + camera_pos_y }, ammo_storage, BAR_TYPE::AMMO_STORAGE);
	ammo_storage_bar_frame = createFrame(renderer, { 4.51f + camera_pos_x,  0.5f + camera_pos_y }, FRAME_TYPE::BAR_FRAME);
};

void SpaceshipHomeSystem::enterSpaceship(Entity player_health_bar, Entity player_food_bar, Entity player_ammo_bar, Entity player_weapon) {
	SpaceshipHome& spaceship_home_info = registry.spaceshipHomes.get(spaceship_home);
	Entity player = registry.players.entities[0]; 

	// No camera shake 
	Entity camera = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera);
	camera_motion.angle = 0;
	camera_motion.scale = vec2(1, 1);

	// Update Spaceship home position based on camera 
	Motion& s_motion = registry.motions.get(spaceship_home);
	s_motion.position = { camera_motion.position.x,camera_motion.position.y };

	// Update position of bars based on camera
	Motion& fs_motion = registry.motions.get(food_storage_bar);
	Motion& fs_frame_motion = registry.motions.get(food_storage_bar_frame);
	Motion& as_motion = registry.motions.get(ammo_storage_bar); 
	Motion& as_frame_motion = registry.motions.get(ammo_storage_bar_frame); 
	fs_motion.position = { -3.5f + camera_motion.position.x, camera_motion.position.y };
	as_motion.position = { 4.5f + camera_motion.position.x,  0.5f + camera_motion.position.y };
	fs_frame_motion.position = { -3.49f + camera_motion.position.x, camera_motion.position.y };
	as_frame_motion.position = { 4.51f + camera_motion.position.x,  0.5f + camera_motion.position.y };

	// Update position of spaceship home items based on camera 
	Motion& t_motion = registry.motions.get(food_item);
	Motion& a_item_motion = registry.motions.get(ammo_item);
	t_motion.position = { -5.5f + camera_motion.position.x, 0.f + camera_motion.position.y };
	a_item_motion.position = { 1.f + camera_motion.position.x, 0.5f + camera_motion.position.y };

	// Regenerate health
	Player& player_info = registry.players.get(player);
	Motion& health = registry.motions.get(player_health_bar);
	Motion& food = registry.motions.get(player_food_bar);
	player_info.health = PLAYER_MAX_HEALTH;
	health.scale = HEALTH_BAR_SCALE;

	// Regenerate food
	regenerateStat(player_info.food, spaceship_home_info.food_storage, PLAYER_MAX_FOOD);
	food.scale = vec2(((float)player_info.food / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);
	fs_motion.scale = vec2(TURKEY_BAR_SCALE[0], ((float)spaceship_home_info.food_storage / (float) SPACESHIP_HOME_MAX_FOOD_STORAGE) * TURKEY_BAR_SCALE[1]);

	// Check if player has weapon equipped
	if (registry.weapons.has(player_weapon)) {
		auto& weapon = registry.weapons.get(player_weapon);
		auto& w_motion = registry.motions.get(player_ammo_bar);
		
		// Regenerate ammo
		regenerateStat(weapon.ammo_count, spaceship_home_info.ammo_storage, PLAYER_MAX_AMMO);
		w_motion.scale = vec2(((float)weapon.ammo_count / (float)PLAYER_MAX_AMMO) * AMMO_BAR_SCALE[0], AMMO_BAR_SCALE[1]);
		as_motion.scale = vec2(AMMO_STORAGE_SCALE[0], ((float)spaceship_home_info.ammo_storage / (float) SPACESHIP_HOME_MAX_AMMO_STORAGE) * AMMO_STORAGE_SCALE[1]);
	}

	spaceship_home_info.is_inside = true;
};

bool SpaceshipHomeSystem::isHome() {
    return registry.spaceshipHomes.get(spaceship_home).is_inside;
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


void SpaceshipHomeSystem::updateSpaceshipHomeUI() {

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
