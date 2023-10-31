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
	motion.scale = vec2({ 1, 1 });

	// Initialize the collider
	
	createMeshCollider(entity, GEOMETRY_BUFFER_ID::PLAYER_MESH, renderer);

	// Add the player to the players registry
	Player& player = registry.players.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::PLAYER,
			GEOMETRY_BUFFER_ID::PLAYER_SPRITE,
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
	createDefaultCollider(entity);

	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
	switch (type) {
	case ITEM_TYPE::QUEST_ONE:
		texture = TEXTURE_ASSET_ID::QUEST_1_ITEM;
		break;
	case ITEM_TYPE::QUEST_TWO:
		texture = TEXTURE_ASSET_ID::QUEST_2_ITEM;
		break;
	case ITEM_TYPE::UPGRADE:
		texture = TEXTURE_ASSET_ID::ITEM;
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
	}

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });

	return entity;
}

// TODO: CLEAN UP ALL THIS CREATE INTO ONE GENERALISED FN
Entity createBasicMob(RenderSystem* renderer, TerrainSystem* terrain, vec2 position)
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
	mob_info.curr_cell = terrain->get_cell(motion.position);

	// Initialize the collider
	createMeshCollider(entity, GEOMETRY_BUFFER_ID::MOB001_MESH, renderer);


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
			RENDER_LAYER_ID::LAYER_4 });

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
	createDefaultCollider(entity);

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
	Entity entity = terrain->get_cell(position);
	TerrainCell& tCell = registry.terrainCells.get(entity);

	// set the cell to collidable
	tCell.flag |= TERRAIN_FLAGS::COLLIDABLE;
	tCell.terrain_type = TERRAIN_TYPE::ROCK;

	// attach collider
	if (!registry.colliders.has(entity))
		createDefaultCollider(entity);

	// change sprite to redblock  TEMPORARY FOR NOW
	terrain->update_tile(entity, tCell);

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
	createDefaultCollider(entity);

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



// NIT: this might be better suited in the physics system. Weapons system imports world_init.hpp to create colliders for projectiles.
// reference:: https://gamedev.stackexchange.com/questions/105296/calculation-correct-position-of-object-after-collision-2d

/// <summary>
/// Create a convex hull collider based on polygon given. 
//	The shape is hard coded to be box shape with entity's scale for now 
/// </summary>
/// <param name="entity">entity to attach this collider</param>
/// <returns>void</returns>
void createDefaultCollider(Entity entity) {

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


void createMeshCollider(Entity entity, GEOMETRY_BUFFER_ID geom_id, RenderSystem* renderer) {

	auto& motion = registry.motions.get(entity);
	auto& collider = registry.colliders.emplace(entity);

	// world position, used in collision detection. This needs to be updated by physics::step
	collider.position = motion.position;

	std::vector<vec2> points;

	// grabing vertice from corresponding mesh
	auto& mesh = renderer->getMesh(geom_id);

	for (int i = 0; i < mesh.vertices.size(); i++) {
		points.push_back(vec2{ mesh.vertices[i].position.x, mesh.vertices[i].position.y });
	
	}

	// generating normals
	vec2 edge;
	for (int i = 0; i < points.size(); i++) {
		edge = points[(i + 1) % points.size()] - points[i];

		// We can obtain the normal by swapping <x,y> with <-y, x>. Normalizing vector to ease later calculation.
		collider.normals.push_back(glm::normalize(vec2(-edge.y, edge.x)));

	}

	// move all points over to collider
	collider.points = std::move(points);

	// DISABLE rotation FOR NOW SINCE MESH HITBOX DOESNT ROTATE
	
	//collider.rotation = mat2(cos(motion.angle), -sin(motion.angle), sin(motion.angle), cos(motion.angle));
	collider.rotation = mat2(cos(0.f), -sin(0.f), sin(0.f), cos(0.f));

	// scale
	collider.scale = motion.scale;

	// flags
	collider.flag = 0;

}

// use a given scale instead since changing motion.scale interfer with the texture rendering
void createProjectileCollider(Entity entity, vec2 scale) {

	auto& motion = registry.motions.get(entity);
	auto& collider = registry.colliders.emplace(entity);

	// world position, used in collision detection. This needs to be updated by physics::step
	collider.position = motion.position;

	// generating points in local coord(center at 0,0)
	std::vector<glm::vec2> points;

	// HARDCODED TO BOX NOW. Assuming all entity will use a box collider
	createBoundingBox(points, scale);

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
	collider.scale = scale;

	// flags
	collider.flag = 0;

}

