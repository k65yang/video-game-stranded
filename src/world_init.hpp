#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "terrain_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
const vec2 HEALTH_BAR_SCALE = vec2(5.f, 0.5);
const vec2 FOOD_BAR_SCALE = vec2(5.5, 0.7);


// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the prey
Entity createItem(RenderSystem* renderer, vec2 position, ITEM_TYPE type);
// the enemy
Entity createBasicMob(RenderSystem* renderer, vec2 position);

// the spaceship 
Entity createSpaceship(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// fow
Entity createFOW(RenderSystem* renderer, vec2 pos);

// health and food bars
Entity createHealthBar(RenderSystem* renderer, vec2 position);
Entity createFoodBar(RenderSystem* renderer, vec2 position);

// test only
Entity createTestDummy(RenderSystem* renderer, vec2 position);

// terrain collider
Entity createTerrainCollider(RenderSystem* renderer,TerrainSystem* terrin, vec2 position);


/// <summary>
/// Creates a box boundary with given size and center point
/// </summary>
/// <param name="size">size of box in vec2</param>
/// <param name="center">center point of box boundary in world space</param>
/// <returns>void</returns>
void createBoxBoundary(RenderSystem* renderer, TerrainSystem* terrain, vec2 size, vec2 center);


/// <summary>
/// Creates a camera pointed at the given spot.
/// </summary>
/// <param name="pos">Position of the camera in world space</param>
/// <returns>The camera entity</returns>
Entity createCamera(vec2 pos);


