#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "render_system.hpp"

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
			EFFECT_ASSET_ID::SPRITESHEET,
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

	createDefaultCollider(entity);

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

	createDefaultCollider(entity);

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
	createDefaultCollider(entity);

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
	createDefaultCollider(entity);

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
	createDefaultCollider(entity);

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

