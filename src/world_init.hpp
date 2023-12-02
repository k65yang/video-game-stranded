#pragma once
#include <map>
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "terrain_system.hpp"
#include "physics_system.hpp"

// These are hard coded to the dimensions of the entity texture
const float FISH_BB_WIDTH = 0.4f * 296.f;
const float FISH_BB_HEIGHT = 0.4f * 165.f;
const float TURTLE_BB_WIDTH = 0.4f * 300.f;
const float TURTLE_BB_HEIGHT = 0.4f * 202.f;
const vec2 HEALTH_BAR_SCALE = vec2(8, 1.2);
const vec2 FOOD_BAR_SCALE = vec2(8, 1.2);
// adjust health and food bar here
const vec2 HEALTH_BAR_POS = vec2{ -5.f, 6.5f };
const vec2 FOOD_BAR_POS = vec2{ 5.f, 6.5f };
const vec2 AMMO_BAR_SCALE = vec2({ 3, 0.4 });
const vec2 TURKEY_BAR_SCALE = vec2({ 0.5, 2 });
const vec2 AMMO_STORAGE_SCALE = vec2({ 0.5, 2 });

// the player
Entity createPlayer(RenderSystem* renderer, PhysicsSystem* physics, vec2 pos);

// the prey
Entity createItem(RenderSystem* renderer, PhysicsSystem* physics, vec2 position, ITEM_TYPE type);

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
Entity createPowerupIndicator(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID powerup_texture);

// Tool tips for ease of use 
Entity createHelp(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

Entity createQuestItem(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

/// <summary>
/// Creates a camera pointed at the given spot.
/// </summary>
/// <param name="pos">Position of the camera in world space</param>
/// <returns>The camera entity</returns>
Entity createCamera(vec2 pos);


