#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "render_system.hpp"
#include "physics_system.hpp"

Entity createPlayer(RenderSystem* renderer, PhysicsSystem* physics, vec2 pos)
	{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	//motion.scale = vec2({ 100/49, 1 });
	motion.scale = vec2({ target_resolution.x / tile_size_px * 0.08503401, target_resolution.y / tile_size_px * 0.0625 });

	// Initialize the collider
	
	physics->createMeshCollider(entity, GEOMETRY_BUFFER_ID::PLAYER_MESH, renderer);

	// Add the player to the players registry
	Player& player = registry.players.emplace(entity);

	// Add player to inventory registry
	Inventory& inventory = registry.inventories.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::SPRITESHEET,
			GEOMETRY_BUFFER_ID::PLAYER_SPRITE,
			RENDER_LAYER_ID::LAYER_2 });

	return entity;
}

Entity createItem(RenderSystem* renderer, PhysicsSystem* physics, vec2 position, ITEM_TYPE type)
{
	// Reserve en entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Initialise the item data field
	auto& item = registry.items.emplace(entity);
	item.data = type;

	// Initialize the collider
	physics->createDefaultCollider(entity);

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
	case ITEM_TYPE::QUEST_ONE:
		texture = TEXTURE_ASSET_ID::QUEST_1_ITEM;
		break;
	case ITEM_TYPE::QUEST_TWO:
		texture = TEXTURE_ASSET_ID::QUEST_2_ITEM;
		break;
	case ITEM_TYPE::QUEST_THREE:
		texture = TEXTURE_ASSET_ID::QUEST_3_ITEM;
		break;
	case ITEM_TYPE::QUEST_FOUR:
		texture = TEXTURE_ASSET_ID::QUEST_4_ITEM;
		break;
	case ITEM_TYPE::WEAPON_UPGRADE:
		texture = TEXTURE_ASSET_ID::WEAPON_UPGRADE;
		break;
	case ITEM_TYPE::FOOD:
		texture = TEXTURE_ASSET_ID::FOOD;
		break;
	case ITEM_TYPE::WEAPON_SHURIKEN:
		texture = TEXTURE_ASSET_ID::WEAPON_SHURIKEN;
		break;
	case ITEM_TYPE::WEAPON_CROSSBOW:
		texture = TEXTURE_ASSET_ID::WEAPON_CROSSBOW;
		break;
	case ITEM_TYPE::WEAPON_SHOTGUN:
		texture = TEXTURE_ASSET_ID::WEAPON_SHOTGUN;
		break;
	case ITEM_TYPE::WEAPON_MACHINEGUN:
		texture = TEXTURE_ASSET_ID::WEAPON_MACHINEGUN;
		break;
	case ITEM_TYPE::POWERUP_SPEED:
		texture = TEXTURE_ASSET_ID::POWERUP_SPEED;
		break;
	case ITEM_TYPE::POWERUP_HEALTH:
		texture = TEXTURE_ASSET_ID::POWERUP_HEALTH;
		break;
	}

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}


Entity createSpaceship(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ target_resolution.x / tile_size_px * 0.20833333, target_resolution.y / tile_size_px * 0.3125});

	// Add spaceship to the spaceship_parts registry
	SpaceshipParts& spaceship_parts = registry.spaceshipParts.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPACESHIP,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER_ID::LAYER_1 });

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::PEBBLE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createBar(RenderSystem* renderer, vec2 position, int amount, BAR_TYPE type) {
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

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
		case BAR_TYPE::HEALTH_BAR:
			texture = TEXTURE_ASSET_ID::RED_BLOCK;
			motion.scale = vec2(((float)amount / (float)PLAYER_MAX_HEALTH) * HEALTH_BAR_SCALE[0], HEALTH_BAR_SCALE[1]);
			break;
		case BAR_TYPE::FOOD_BAR:
			texture = TEXTURE_ASSET_ID::BLUE_BLOCK;
			motion.scale = vec2(((float)amount / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);
			break;
		case BAR_TYPE::AMMO_BAR:
			// Ammo bar is created in weapons system.
			break; 
		case BAR_TYPE::HEALTH_STORAGE_BAR:
			texture = TEXTURE_ASSET_ID::BLACK_BLOCK;
			motion.scale = vec2(HEALTH_STORAGE_BAR_SCALE[0], ((float)amount / (float)SPACESHIP_MAX_HEALTH_STORAGE) * HEALTH_STORAGE_BAR_SCALE[1]);
			break;
		case BAR_TYPE::FOOD_STORAGE_BAR:
			texture = TEXTURE_ASSET_ID::BLACK_BLOCK;
			motion.scale = vec2(FOOD_STORAGE_BAR_SCALE[0], ((float)amount / (float)SPACESHIP_MAX_FOOD_STORAGE) * FOOD_STORAGE_BAR_SCALE[1]);
			break;
		case BAR_TYPE::AMMO_STORAGE_BAR:
			texture = TEXTURE_ASSET_ID::BLACK_BLOCK;
			motion.scale = vec2(AMMO_STORAGE_BAR_SCALE[0], ((float)amount / (float)SPACESHIP_MAX_AMMO_STORAGE) * AMMO_STORAGE_BAR_SCALE[1]);
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
}

Entity createFrame(RenderSystem* renderer, vec2 position, FRAME_TYPE type) {
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

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
		case FRAME_TYPE::HEALTH_FRAME:
			texture = TEXTURE_ASSET_ID::HEALTH_FRAME;
			motion.scale = vec2({ target_resolution.x / tile_size_px * 0.3333, target_resolution.y / tile_size_px * 0.075 });
			break;
		case FRAME_TYPE::FOOD_FRAME:
			texture = TEXTURE_ASSET_ID::FOOD_FRAME;
			motion.scale = vec2({ target_resolution.x / tile_size_px * 0.3333, target_resolution.y / tile_size_px * 0.075 });
			break;
		case FRAME_TYPE::STORAGE_FRAME:
			texture = TEXTURE_ASSET_ID::STORAGE_FRAME;
			motion.scale = vec2({ 1.2, 2 });
			break;
	}

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
	}

Entity createHelp(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 20.f, 6.f });

	registry.tips.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
}

Entity createQuestItem(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 3.f, 3.f });

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
}

Entity createWeaponIndicator(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID weapon_texture) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 2.5, 2.5 });

	registry.renderRequests.insert(
		entity,
		{ weapon_texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
}

Entity createPowerupIndicator(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID powerup_texture) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 2.f, 2.f });

	registry.renderRequests.insert(
		entity,
		{ powerup_texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
}

Entity createCamera(vec2 pos)
{
	auto entity = Entity();
	registry.set_main_camera(entity);

	Motion& motion = registry.motions.emplace(entity);
	Camera& camera = registry.cameras.emplace(entity);

	camera.mode_follow = true;
	motion.position = pos;

	// In this case, the camera's "scale" is the scale of the image plane.
	// Bigger value = things will look smaller
	motion.scale = { 1 , 1 };
	return entity;
}

Entity createText(RenderSystem* renderer, vec2 position, std::string str, float scale, vec3 color) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = { 1.f, 1.f };

	// Add entity to Text registry
	Text& text = registry.texts.emplace(entity);
	text.str = str;
	text.color = color;
	text.scale = scale;

	// Add entity to ScreenUI registry
	registry.screenUI.insert(entity, position);

	return entity;
}

Entity createSpaceshipDepart(RenderSystem* renderer) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { 0, -1.5f };
	motion.scale = vec2({ target_resolution.x / tile_size_px * 0.20833333*1.2, target_resolution.y / tile_size_px * 0.3125*1.4 });

	// Add spaceship to the spaceship_parts registry
	SpaceshipParts& spaceship_parts = registry.spaceshipParts.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPACESHIP_DEPART,
		 EFFECT_ASSET_ID::SPRITESHEET,
		 GEOMETRY_BUFFER_ID::SPACESHIP_DEPART_SPRITE,
		 RENDER_LAYER_ID::LAYER_1 });

	return entity;
}