#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Entity createPlayer(RenderSystem* renderer, vec2 pos)
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
	motion.scale = vec2({ 3, 3 });

	// Initialize the collider
	createCollider(entity);

	// Add the player to the players registry
	registry.players.emplace(entity);



	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::PLAYER,
			GEOMETRY_BUFFER_ID::PLAYER_SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}

Entity createProjectile(RenderSystem* renderer, vec2 pos) {
	// Reserve an entity
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = pos;

	// Add this projectile to the projectiles registry
	registry.projectiles.emplace(entity);

	// Initialize the collider
	createCollider(entity);

	// TODO: Change this later
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::WEAPON;

	registry.renderRequests.insert(
		entity,
		{ texture,
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

	// Initialize the collider
	createCollider(entity);

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
	case ITEM_TYPE::WEAPON_GENERIC:
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

	// Initialize the collider
	createCollider(entity);

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


/// <summary>
/// Creates FOW entity on given position
/// </summary>
/// <param name="renderer">renderer for render request</param>
/// <param name="position">position for FOW</param>
/// <returns>The fow entity</returns>
Entity createFOW(RenderSystem* renderer, vec2 position)
{
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
	motion.position = position;
	motion.scale = vec2({ 1, 1 });

	// Initialize collider
	createCollider(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::REDBLOCK,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER_ID::LAYER_1 });

	return entity;
}


/// <summary>
/// Turn a terrain cell non-passable, as well as changing the sprite
/// </summary>
/// <param name="renderer">renderer for render request</param>
/// <param name="renderer">terrain from terrain system</param>
/// <param name="position">position of target terrain cell</param>
/// <returns>The terrain cell entity</returns>
Entity createTerrainCollider(RenderSystem* renderer, TerrainSystem* terrain, vec2 position)
{
	auto entity = terrain->get_cell(position.x, position.y);
	TerrainCell& tCell = registry.terrainCells.get(entity);
	Motion& motion = registry.motions.get(entity);
	motion.position = position;

	// set the cell to collidable
	tCell.flag = uint(1) | TERRAIN_FLAGS::COLLIDABLE;

	// attach collider
	createCollider(entity);

	// change sprite to redblock  TEMPORARY FOR NOW
	RenderRequest& rr = registry.terrainRenderRequests.get(entity);
	rr.used_texture = TEXTURE_ASSET_ID::TERRAIN_STONE;

	return entity;
}


/// <summary>
/// Create boundary block given position and scale
/// </summary>
/// <param name="renderer">renderer for render request</param>
/// <param name="position">scale of the block</param>
/// <param name="scale">scale of the block</param>
/// <returns>Boundary block entity</returns>
Entity createBoundaryBlock(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	// attach boundary block component
	registry.boundaries.emplace(entity);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;

	// Setting initial values
	motion.scale = scale;

	// Initialize the collider
	createCollider(entity);

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
void boundaryInitialize(RenderSystem* renderer, vec2 size, vec2 center) {

	// optimized by making bigger scale block instead of multiple block with unit 1

	//top
	createBoundaryBlock(renderer, { center.x, center.y - floor(size.y / 2.f) }, { size.x, 1 });

	//bottom
	createBoundaryBlock(renderer, { center.x, center.y + floor(size.y / 2.f) }, { size.x, 1 });

	//left
	createBoundaryBlock(renderer, { center.x - floor(size.x / 2.f), center.y }, { 1, size.y - 2});

	//right
	createBoundaryBlock(renderer, { center.x + floor(size.x / 2.f), center.y }, { 1, size.y - 2});

}


/// <summary>
/// create coord of a box given a scale(width/height). Points are in local coord and center is 0,0. 
/// points are added in clockwise direction starting from top left point
/// </summary>
/// <param name="points">destination buffer to store the points</param>
/// <param name="scale">width and height of the box</param>
/// <returns>void</returns>
void createBoundingBox(std::vector<vec2>& points, vec2 scale) {

	// calculate top left x position by center(0) - width/2, similar on y position etc..
	vec2 b1_topLeft = {(0 - (scale.x / 2.f)), (0 - (scale.y / 2.f))};
	vec2 b1_topRight = { (0 + (scale.x / 2.f)), (0 - (scale.y / 2.f)) };
	vec2 b1_bottomRight = {(0 + (scale.x / 2.f)), (0 + (scale.y / 2.f))};
	vec2 b1_bottomLeft = {(0 - (scale.x / 2.f)), (0 + (scale.y / 2.f)) };

	points.push_back(b1_topLeft);
	points.push_back(b1_topRight);
	points.push_back(b1_bottomRight);
	points.push_back(b1_bottomLeft);

}


// reference:: https://gamedev.stackexchange.com/questions/105296/calculation-correct-position-of-object-after-collision-2d

/// <summary>
/// Create a convex hull collider based on polygon given. 
//	The shape is hard coded to be box shape with entity's scale for now 
/// </summary>
/// <param name="entity">entity to attach this collider</param>
/// <returns>void</returns>
void createCollider(Entity entity) {

	auto& motion = registry.motions.get(entity);
	auto& collider = registry.colliders.emplace(entity);

	// world position, used in collision detection. This needs to be updated by physics::step
	collider.position = motion.position;

	// generating points in local coord(center at 0,0)
	std::vector<glm::vec2> points;

	// HARDCODED TO BOX NOW. Assuming all entity will use a box collider
	createBoundingBox(points, motion.scale);

	// generating normals
	vec2 edge;
	for (int i = 0; i < points.size(); i++) {
		edge = points[(i + 1) % points.size()] - points[i];

		// We can obtain the normal by swapping <x,y> with <-y, x>. Normalizing vector to ease later calculation.
		collider.normals.push_back(glm::normalize(vec2(-edge.y, edge.x)));
		
	}

	// move all points over to collider
	collider.points = std::move(points);
	
	// rotation
	collider.rotation = mat2(cos(motion.angle), -sin(motion.angle), sin(motion.angle), cos(motion.angle));

	// scale
	collider.scale = motion.scale;

	// flags
	collider.flag = 0;

}
