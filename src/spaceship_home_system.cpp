#include "spaceship_home_system.hpp"

void SpaceshipHomeSystem::step(float elapsed_ms) {
	Motion& camera_motion = registry.motions.get(registry.cameras.entities[0]);

	// UI Movement
	for (Entity e : registry.screenUI.entities) {
		if (registry.motions.has(e)) {
			vec2& ui_inital_position = registry.screenUI.get(e);
			Motion& ui_motion = registry.motions.get(e);
			ui_motion.position = ui_inital_position + camera_motion.position;
		}
	}
};

void SpaceshipHomeSystem::init(RenderSystem* renderer_arg, WeaponsSystem* weapon_system_arg, QuestSystem* quest_system_arg) {
	this->renderer = renderer_arg;
	this->weaponsSystem = weapon_system_arg;
	this->quest_system = quest_system_arg;
}

void SpaceshipHomeSystem::resetSpaceshipHomeSystem(int health_storage, int food_storage, int ammo_storage) {
	// Create spaceship home
	spaceship_home = createSpaceshipHome(SPACESHIP_HOME_POSITION, health_storage, food_storage, ammo_storage);

	// Create health storage elements
	health_item = createSpaceshipHomeItem(HEALTH_ITEM_POSITION, TEXTURE_ASSET_ID::SPACESHIP_HOME_HEALTH);

	// Create food storage elements
	food_item = createSpaceshipHomeItem(FOOD_ITEM_POSITION, TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD);

	// Create ammo storage elements
	ammo_item = createSpaceshipHomeItem(AMMO_ITEM_POSITION, TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO);
};

void SpaceshipHomeSystem::enterSpaceship(Entity player_health_bar, Entity player_food_bar) {
	SpaceshipHome& spaceship_home_info = registry.spaceshipHomes.get(spaceship_home);
	Entity player = registry.players.entities[0]; 
	Player& player_info = registry.players.get(player);
	Motion& player_health_bar_motion = registry.motions.get(player_health_bar);
	Motion& player_food_bar_motion = registry.motions.get(player_food_bar);

	// Set player to be home
	player_info.is_home = true;

	// No camera shake 
	Entity camera = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera);
	camera_motion.angle = 0;
	camera_motion.scale = { 1.f, 1.f };

	// Submit quest items
	if (quest_system->submitQuestItems()) {
		printf("ALL QUEST ITEMS SUBMITTED\n");
	}

	// Regenerate health
	updateStat(player_info.health, spaceship_home_info.health_storage, PLAYER_MAX_HEALTH);
	updateStatBar(player_info.health, player_health_bar_motion, PLAYER_MAX_HEALTH, HEALTH_BAR_SCALE);
	health_storage_count = createText(
		renderer, 
		HEALTH_STORAGE_COUNT_POSITION, 
		createStorageCountTextString(spaceship_home_info.health_storage), 
		STORAGE_COUNT_TEXT_SCALE, 
		spaceship_home_info.health_storage > 0 ? STORAGE_FULL_TEXT_COLOR : STORAGE_EMPTY_TEXT_COLOR 
	);

	// Regenerate food
	updateStat(player_info.food, spaceship_home_info.food_storage, PLAYER_MAX_FOOD);
	updateStatBar(player_info.food, player_food_bar_motion, PLAYER_MAX_FOOD, FOOD_BAR_SCALE);
	food_storage_count = createText(
		renderer, 
		FOOD_STORAGE_COUNT_POSITION, 
		createStorageCountTextString(spaceship_home_info.food_storage), 
		STORAGE_COUNT_TEXT_SCALE, 
		spaceship_home_info.food_storage > 0 ? STORAGE_FULL_TEXT_COLOR : STORAGE_EMPTY_TEXT_COLOR 
	);

	// Reload all weapons. This can be problematic because it always reloads in a specific order.
	// TODO: Somehow give options to player to reload specific weapons.
	int amount_reloaded = 0;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_SHURIKEN, spaceship_home_info.ammo_storage);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_CROSSBOW, spaceship_home_info.ammo_storage);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_SHOTGUN, spaceship_home_info.ammo_storage);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_MACHINEGUN, spaceship_home_info.ammo_storage);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	ammo_storage_count = createText(
		renderer, 
		AMMO_STORAGE_COUNT_POSITION, 
		createStorageCountTextString(spaceship_home_info.ammo_storage), 
		STORAGE_COUNT_TEXT_SCALE, 
		spaceship_home_info.ammo_storage > 0 ? STORAGE_FULL_TEXT_COLOR : STORAGE_EMPTY_TEXT_COLOR 
	);
};

void SpaceshipHomeSystem::exitSpaceship() {
	// Remove storage count text
	registry.remove_all_components_of(health_storage_count);
	registry.remove_all_components_of(food_storage_count);
	registry.remove_all_components_of(ammo_storage_count);

	// Reset player's position
	Entity player = registry.players.entities[0]; 
	Motion& motion = registry.motions.get(player);
	motion.position = { 0.f, 0.f };

	// Set player to not be home
	Player& player_info = registry.players.get(player);
	player_info.is_home = false;
};

bool SpaceshipHomeSystem::isHome() {
    return registry.players.components[0].is_home;
};

bool SpaceshipHomeSystem::isMouseOverStorageItem(vec2 mouse_pos, ITEM_TYPE type) {
	return false;
};

void SpaceshipHomeSystem::regenerateStat(RESOURCE_TYPE type) {

};

Entity SpaceshipHomeSystem::updateStorageCountText(RESOURCE_TYPE type) {
	return Entity();
};

Entity SpaceshipHomeSystem::createSpaceshipHome(vec2 position, int health_storage, int food_storage, int ammo_storage) {
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

	// Add entity to spaceship home registry
	auto& spaceshipHome = registry.spaceshipHomes.emplace(entity);
	spaceshipHome.health_storage = health_storage;
	spaceshipHome.food_storage = food_storage;
	spaceshipHome.ammo_storage = ammo_storage;

	// Add entity to screen UI registry
	registry.screenUI.insert(entity, position);
	
	registry.renderRequests.insert(
		entity,
		{ 
            TEXTURE_ASSET_ID::SPACESHIP_HOME,
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
  
	// Add entity to screen UI registry
	registry.screenUI.insert(entity, position);

	switch(texture) {
		case TEXTURE_ASSET_ID::SPACESHIP_HOME_HEALTH:
			motion.scale = { target_resolution.x / tile_size_px * 0.1875, target_resolution.y / tile_size_px * 0.28125 };
			break;
		case TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD:
			motion.scale = { target_resolution.x / tile_size_px * 0.125, target_resolution.y / tile_size_px * 0.1875 };
			break;
		case TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO:
			motion.scale = { target_resolution.x / tile_size_px * 0.3, target_resolution.y / tile_size_px * 0.3 };
			break;
	}

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

void SpaceshipHomeSystem::updateStat(int& stat, int& storage, int max_stat_value) {
	int missing = max_stat_value - stat;
	
	if (missing <= storage) {
		storage -= missing;
		stat = max_stat_value;
	} else {
		stat += storage;
		storage = 0;
	}
};

void SpaceshipHomeSystem::updateStatBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor) {
	bar.scale = vec2(((float) new_val / (float) max_bar_value) * scale_factor.x, scale_factor.y);
};

void SpaceshipHomeSystem::updateStorageBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor) {
	bar.scale = vec2(scale_factor.x, ((float) new_val / (float) max_bar_value) * scale_factor.y);
};

std::string SpaceshipHomeSystem::createStorageCountTextString(int storage) {
	return "x " + std::to_string(storage);
};
