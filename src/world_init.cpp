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
	motion.scale = vec2({ 100/49, 1 });

	// Initialize the collider
	
	physics->createMeshCollider(entity, GEOMETRY_BUFFER_ID::PLAYER_MESH, renderer);

	// Add the player to the players registry
	Player& player = registry.players.emplace(entity);

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

	//physics->createDefaultCollider(entity);

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 3, 4 });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPACESHIP,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}

Entity createSpaceshipHome(RenderSystem* renderer, vec2 position, bool is_inside, int food_storage, int ammo_storage) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	//physics_system->createDefaultCollider(entity);

	// Add spaceship home to spaceship home registry
	auto& spaceshipHome = registry.spaceshipHomes.emplace(entity);
	spaceshipHome.is_inside = is_inside;
	spaceshipHome.food_storage = food_storage;
	spaceshipHome.ammo_storage = ammo_storage;
	
	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ target_resolution.x / tile_size_px, target_resolution.y / tile_size_px });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPACEHOME,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

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
	motion.scale = vec2({ 5.5, 0.7 });

	// Initialize the collider
	//createDefaultCollider(entity);

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
		case BAR_TYPE::HEALTH_BAR:
			texture = TEXTURE_ASSET_ID::REDBLOCK;
			motion.scale = vec2(((float)amount / (float)PLAYER_MAX_HEALTH) * HEALTH_BAR_SCALE[0], HEALTH_BAR_SCALE[1]);

			break;
		case BAR_TYPE::FOOD_BAR:
			texture = TEXTURE_ASSET_ID::BLUEBLOCK;
			motion.scale = vec2(((float)amount / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);

			break;
		case BAR_TYPE::AMMO_BAR:
			texture = TEXTURE_ASSET_ID::BROWNBLOCK;
			motion.scale = vec2({ 3, 0.4 }); 
			break; 
		case BAR_TYPE::FOOD_STORAGE:
			texture = TEXTURE_ASSET_ID::FOOD_BLOCK;
			motion.scale = vec2({ 0.5, 2 });
			break;
		case BAR_TYPE::AMMO_STORAGE:
			texture = TEXTURE_ASSET_ID::AMMO_BLOCK;
			motion.scale = vec2({ 0.5, 2 });
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

Entity createFrame(RenderSystem* renderer, vec2 position, FRAME_TYPE type) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 7, 1 });

	// Initialize the collider
	//physics_system->createDefaultCollider(entity);

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
			case FRAME_TYPE::HEALTH_FRAME:
				texture = TEXTURE_ASSET_ID::HEALTH_FRAME;
				break;
			case FRAME_TYPE::FOOD_FRAME:
				texture = TEXTURE_ASSET_ID::FOOD_FRAME;
				motion.scale = vec2({ 6.7, 1 });

				break;
			case FRAME_TYPE::BAR_FRAME:
				texture = TEXTURE_ASSET_ID::BAR_FRAME;
				motion.scale = vec2({ 1.2, 2 });
				break;

		}
	motion.position = position; 
	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 });

	return entity;
	}

Entity createStorage(RenderSystem* renderer, vec2 position, ITEM_TYPE type) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 7, 5 });

	// Initialize the collider
	//physics_system->createDefaultCollider(entity);

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
			case ITEM_TYPE::AMMO:
				texture = TEXTURE_ASSET_ID::AMMO;
				motion.scale = vec2({ 8, 6 });
				break;
			case ITEM_TYPE::TURKEY:
				texture = TEXTURE_ASSET_ID::TURKEY;
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

/// <summary>
/// Creates a camera centred on a position
/// </summary>
/// <param name="pos">World space position where the camera is facing</param>
/// <returns>The camera entity</returns>
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













