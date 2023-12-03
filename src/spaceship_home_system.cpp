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

void SpaceshipHomeSystem::resetSpaceshipHomeSystem(int food_storage, int ammo_storage) {
	Entity camera = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera);
	vec2 camera_pos = camera_motion.position;

	// Create spaceship home
	spaceship_home = createSpaceshipHome(camera_pos, food_storage, ammo_storage);

	// Create food storage elements
	food_item = createSpaceshipHomeItem(FOOD_ITEM_POSITION, TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD);
	food_storage_bar = createBar(renderer, FOOD_STORAGE_BAR_POSITION, food_storage, BAR_TYPE::FOOD_STORAGE);
	food_storage_bar_frame = createFrame(renderer, FOOD_STORAGE_BAR_FRAME_POSITION, FRAME_TYPE::BAR_FRAME); 

	// Create ammo storage elements
	ammo_item = createSpaceshipHomeItem(AMMO_ITEM_POSITION, TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO);
	ammo_storage_bar = createBar(renderer, AMMO_STORAGE_BAR_POSITION, ammo_storage, BAR_TYPE::AMMO_STORAGE);
	ammo_storage_bar_frame = createFrame(renderer, AMMO_STORAGE_BAR_FRAME_POSITION, FRAME_TYPE::BAR_FRAME);
};

void SpaceshipHomeSystem::enterSpaceship(Entity player_health_bar, Entity player_food_bar) {
	Entity camera = registry.cameras.entities[0];
	Motion& camera_motion = registry.motions.get(camera);
	SpaceshipHome& spaceship_home_info = registry.spaceshipHomes.get(spaceship_home);
	Entity player = registry.players.entities[0]; 
	Player& player_info = registry.players.get(player);
	Motion& player_health_bar_motion = registry.motions.get(player_health_bar);
	Motion& player_food_bar_motion = registry.motions.get(player_food_bar);
	Motion& food_storage_bar_motion = registry.motions.get(food_storage_bar);
	Motion& ammo_storage_bar_motion = registry.motions.get(ammo_storage_bar); 

	// No camera shake 
	camera_motion.angle = 0;
	camera_motion.scale = { 1.f, 1.f };

	// Submit quest items
	if (quest_system->submitQuestItems()) {
		printf("ALL QUEST ITEMS SUBMITTED\n");
	}

	// Regenerate health
	player_info.health = PLAYER_MAX_HEALTH;
	player_health_bar_motion.scale = HEALTH_BAR_SCALE;

	// Regenerate food
	regenerateStat(player_info.food, spaceship_home_info.food_storage, PLAYER_MAX_FOOD);
	updateStatBar(player_info.food, player_food_bar_motion, PLAYER_MAX_FOOD, FOOD_BAR_SCALE);
	updateStorageBar(spaceship_home_info.food_storage, food_storage_bar_motion, SPACESHIP_MAX_FOOD_STORAGE, FOOD_STORAGE_BAR_SCALE);

	// Reload all weapons. This can be problematic because it always reloads in a specific order.
	// TODO: Somehow give options to player to reload specific weapons.
	int amount_reloaded = 0;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_SHURIKEN, 100);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_CROSSBOW, 100);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_SHOTGUN, 100);
	spaceship_home_info.ammo_storage -= amount_reloaded;
	amount_reloaded = weaponsSystem->increaseAmmo(ITEM_TYPE::WEAPON_MACHINEGUN, 100);
	spaceship_home_info.ammo_storage -= amount_reloaded;

	updateStorageBar(spaceship_home_info.ammo_storage, ammo_storage_bar_motion, SPACESHIP_MAX_AMMO_STORAGE, AMMO_STORAGE_BAR_SCALE);

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

	// Add entity to spaceship home registry
	auto& spaceshipHome = registry.spaceshipHomes.emplace(entity);
	spaceshipHome.food_storage = food_storage;
	spaceshipHome.ammo_storage = ammo_storage;

	// Add entity to screen UI registry
	registry.screenUI.insert(entity, position);
	
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
  
	// Add entity to screen UI registry
	registry.screenUI.insert(entity, position);

	switch(texture) {
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

void SpaceshipHomeSystem::updateStatBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor) {
	bar.scale = vec2(((float) new_val / (float) max_bar_value) * scale_factor.x, scale_factor.y);
};

void SpaceshipHomeSystem::updateStorageBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor) {
	bar.scale = vec2(scale_factor.x, ((float) new_val / (float) max_bar_value) * scale_factor.y);
};
