#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };

	// Add the player to the players registry
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}

Entity createItem(RenderSystem* renderer, vec2 position, ITEM_TYPE type)
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

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
	case ITEM_TYPE::QUEST:
		texture = TEXTURE_ASSET_ID::ITEM;
		break;
	case ITEM_TYPE::UPGRADE:
		texture = TEXTURE_ASSET_ID::ITEM;
		break;
	case ITEM_TYPE::FOOD:
		texture = TEXTURE_ASSET_ID::FOOD;
		break;
	case ITEM_TYPE::WEAPON:
		texture = TEXTURE_ASSET_ID::WEAPON;
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

Entity createBasicMob(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 1, 1 });

	// Initialize path
	registry.paths.emplace(entity);

	// Classify this entity as a mob.
	auto& mob_info = registry.mobs.emplace(entity);
	mob_info.damage = 50;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MOB,
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

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 3, 4 });
	//motion.scale = vec2({ 50.f, 50.f });
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

Entity createHealthBar(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position; 
	motion.scale = vec2({ 5.f, 0.5 });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::REDBLOCK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_3 });

	return entity;
}
Entity createFoodBar(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 5.5, 0.7 });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BLUEBLOCK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_3 });

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

// Creates FOW entity on given position
Entity createFOW(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ 50.f, 50.f });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOW,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_3 });

	return entity;
}

Entity createTestDummy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 1, 1 });

	// Create and (empty) Turtle component to be able to refer to all turtles
	registry.colliders.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::REDBLOCK,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER_ID::LAYER_1 });

	return entity;
}


Entity createTerrainCollider(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ 1, 1 });

	// Create and (empty) Turtle component to be able to refer to all turtles
	registry.colliders.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::REDBLOCK,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER_ID::LAYER_1 });

	return entity;
}


/// <summary>
/// Creates a box boundary with given size and center point
/// </summary>
/// <param name="size">size of box in vec2</param>
/// <param name="center">center point of box boundary in world space</param>
/// <returns>void</returns>
void createBoxBoundary(RenderSystem* renderer , vec2 size, vec2 center) {
	vec2 topleftpos = {center.x - (int)size.x / 2, center.y - (int)size.y / 2};
	// start from top left

	// draw top and bottom of the box
	for (int i = 0; i < size.x; i++) {
		
		createTerrainCollider(renderer, {topleftpos.x + i, topleftpos.y});

		// We're subtracting the y position by 1 because we're off by 1
		createTerrainCollider(renderer, {topleftpos.x + i, topleftpos.y + size.y - 1});
	}

	//draw left and right of the box
	for (int i = 1; i < size.y - 1; i++) {
		// start from top left
		createTerrainCollider(renderer, { topleftpos.x, topleftpos.y + i});
		createTerrainCollider(renderer, { topleftpos.x + size.x - 1, topleftpos.y + i});
	}

}
