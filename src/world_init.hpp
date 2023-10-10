#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are ahrd coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the prey
Entity createItem(RenderSystem* renderer, vec2 position, ITEM_TYPE type);
// the enemy
Entity createMob(RenderSystem* renderer, vec2 position);

// the spaceship 
Entity createSpaceship(RenderSystem* renderer, vec2 position);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// fow
Entity createFOW(RenderSystem* renderer, vec2 pos);
Entity createTestDummy(RenderSystem* renderer, vec2 position);


/// <summary>
/// Creates a camera pointed at the given spot.
/// </summary>
/// <param name="pos">Position of the camera in world space</param>
/// <returns>The camera entity</returns>
Entity createCamera(vec2 pos);
