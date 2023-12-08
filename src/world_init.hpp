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
const vec2 AMMO_BAR_SCALE = vec2({ 3, 0.4 }); 
const vec2 FOOD_STORAGE_BAR_SCALE = { 0.5f, 2.f };
const vec2 AMMO_STORAGE_BAR_SCALE = { 0.5f, 2.f }; 
const vec2 HEALTH_STORAGE_BAR_SCALE = { 0.5f, 2.f };
const vec2 HEALTH_BAR_SCALE = { target_resolution.x / tile_size_px * 0.3333, target_resolution.y / tile_size_px * 0.075 };
const vec2 FOOD_BAR_SCALE = { target_resolution.x / tile_size_px * 0.3333, target_resolution.y / tile_size_px * 0.075 };
const vec2 HEALTH_BAR_FRAME_POS = vec2{ -5.f, 6.5f };
const vec2 FOOD_BAR_FRAME_POS = vec2{ 5.f, 6.5f };

//const vec2 TURKEY_BAR_SCALE = vec2({ 0.5, 2 });


// the player
Entity createPlayer(RenderSystem* renderer, PhysicsSystem* physics, vec2 pos);

// the prey
Entity createItem(RenderSystem* renderer, PhysicsSystem* physics, vec2 position, ITEM_TYPE type);

// the spaceship 
Entity createSpaceship(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// UI Elements
Entity createBar(RenderSystem* renderer, vec2 position, int amount, BAR_TYPE type);
Entity createFrame(RenderSystem* renderer, vec2 position, FRAME_TYPE type);
Entity createWeaponIndicator(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID weapon_texture);
Entity createPowerupIndicator(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID powerup_texture);
Entity createPointingArrow(RenderSystem* renderer, Entity player, Entity target);

// the spaceship departing 
Entity createSpaceshipDepart(RenderSystem* renderer); 
// Tool tips for ease of use 
Entity createHelp(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

Entity createQuestItem(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

/// <summary>
/// Creates a camera pointed at the given spot.
/// </summary>
/// <param name="pos">Position of the camera in world space</param>
/// <returns>The camera entity</returns>
Entity createCamera(vec2 pos);

Entity createText(RenderSystem* renderer, vec2 position, std::string str, float scale=1.f, vec3 color={1.f, 1.f, 1.f});


// Muzzle flash 
Entity createMuzzleFlash(RenderSystem* renderer, vec2 position);


// the ending screen text 
Entity createEndingTextPopUp(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID texture);

