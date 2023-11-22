#pragma once
#include <map>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "terrain_system.hpp"

// These are hard coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
const vec2 HEALTH_BAR_SCALE = vec2(5.5, 0.7);
const vec2 FOOD_BAR_SCALE = vec2(5.5, 0.7);
const vec2 AMMO_BAR_SCALE = vec2({ 3, 0.4 }); 
const vec2 TURKEY_BAR_SCALE = vec2({ 0.5, 2 });
const vec2 AMMO_STORAGE_SCALE = vec2({ 0.5, 2 });

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);

// the prey
Entity createItem(RenderSystem* renderer, vec2 position, ITEM_TYPE type);

// the spaceship 
Entity createSpaceship(RenderSystem* renderer, vec2 position);

// the spaceship home
Entity createSpaceshipHome(RenderSystem* renderer, vec2 position, bool is_inside, int food_storage, int ammo_storage); 

Entity createStorage(RenderSystem* renderer, vec2 position, ITEM_TYPE type);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);



// UI Elements
Entity createBar(RenderSystem* renderer, vec2 position, int amount, BAR_TYPE type);
Entity createFrame(RenderSystem* renderer, vec2 position, FRAME_TYPE type);
Entity createWeaponIndicator(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID weapon_texture);

// Tool tips for ease of use 
Entity createHelp(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

Entity createQuestItem(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

/// <summary>
/// create coord of a box given a scale(width/height). Points are in local coord and center is 0,0. 
/// points are added in clockwise direction starting from top left point
/// </summary>
/// <param name="points">destination buffer to store the points</param>
/// <param name="scale">width and height of the box</param>
/// <returns>void</returns>
void createBoundingBox(std::vector<vec2>& points, vec2 scale);


/// <summary>
/// Create a convex hull collider based on polygon given. 
//	The shape is hard coded to be box shape with entity's scale for now 
/// </summary>
/// <param name="entity">entity to attach this collider</param>
/// <returns>void</returns>
void createDefaultCollider(Entity entity);

void createMeshCollider(Entity entity, GEOMETRY_BUFFER_ID geom_id, RenderSystem* renderer);
void createProjectileCollider(Entity entity, vec2 scale);

/// <summary>
/// Creates a camera pointed at the given spot.
/// </summary>
/// <param name="pos">Position of the camera in world space</param>
/// <returns>The camera entity</returns>
Entity createCamera(vec2 pos);


