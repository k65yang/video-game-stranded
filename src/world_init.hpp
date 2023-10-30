#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "terrain_system.hpp"

// These are hard coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
const vec2 HEALTH_BAR_SCALE = vec2(5.f, 0.5);
const vec2 FOOD_BAR_SCALE = vec2(5.5, 0.7);


// the player
Entity createPlayer(RenderSystem* renderer, TerrainSystem* terrain, vec2 pos);
// the prey
Entity createItem(RenderSystem* renderer, vec2 position, ITEM_TYPE type);
// the enemy
Entity createBasicMob(RenderSystem* renderer, TerrainSystem* terrain, vec2 position);

// the spaceship 
Entity createSpaceship(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

/// <summary>
/// Creates FOW entity on given position
/// </summary>
/// <param name="renderer">renderer for render request</param>
/// <param name="position">position for FOW</param>
/// <returns>The fow entity</returns>
Entity createFOW(RenderSystem* renderer, vec2 pos);

// health and food bars
Entity createHealthBar(RenderSystem* renderer, vec2 position);
Entity createFoodBar(RenderSystem* renderer, vec2 position);

// test only
Entity createTestDummy(RenderSystem* renderer, vec2 position);

/// <summary>
/// Turn a terrain cell non-passable, as well as changing the sprite
/// </summary>
/// <param name="renderer">renderer for render request</param>
/// /// <param name="renderer">terrain from terrain system</param>
/// <param name="position">position of target terrain cell</param>
/// <returns>The terrain cell entity</returns>
Entity createTerrainCollider(RenderSystem* renderer,TerrainSystem* terrain, vec2 position);

/// <summary>
/// Create boundary block given position and scale
/// </summary>
/// <param name="renderer">renderer for render request</param>
/// <param name="position">scale of the block</param>
/// <param name="scale">scale of the block</param>
/// <returns>Boundary block entity</returns>
Entity createBoundaryBlock(RenderSystem* renderer, vec2 position, vec2 scale);

/// <summary>
/// Creates a box boundary with given size and center point
/// </summary>
/// <param name="size">size of box in vec2</param>
/// <param name="center">center point of box boundary in world space</param>
/// <returns>void</returns>
void boundaryInitialize(RenderSystem* renderer, vec2 size, vec2 center);

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
void createCollider(Entity entity);


/// <summary>
/// Creates a camera pointed at the given spot.
/// </summary>
/// <param name="pos">Position of the camera in world space</param>
/// <returns>The camera entity</returns>
Entity createCamera(vec2 pos);


