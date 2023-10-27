#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include <deque>
#include "../ext/stb_image/stb_image.h"

// Player component

const int PLAYER_MAX_FOOD = 100;
const int PLAYER_MAX_HEALTH = 100;

enum class ITEM_TYPE {
	QUEST = 0,
	WEAPON_NONE = QUEST + 1,
	WEAPON_SHURIKEN = WEAPON_NONE + 1,
	WEAPON_CROSSBOW = WEAPON_SHURIKEN + 1,
	WEAPON_SHOTGUN = WEAPON_CROSSBOW + 1,
	WEAPON_MACHINEGUN = WEAPON_SHOTGUN + 1,
	FOOD = WEAPON_MACHINEGUN + 1,
	UPGRADE = FOOD + 1,
};

// TODO: cool idea for later is to have a customizable difficulty that adjusts food and health.
struct Player
{
	int health = PLAYER_MAX_HEALTH;
	float health_decrease_time = 0; // in 
	float food_decrease_time = 0; 
	int decrease_health_to = PLAYER_MAX_HEALTH;
	int decrease_food_to = PLAYER_MAX_FOOD;
	float iframes_timer = 0; // in ms
	int food = PLAYER_MAX_FOOD;
	int framex = 0; 
	int framey = 1; 

};

// The weapon
struct Weapon {
	ITEM_TYPE weapon_type;
	bool can_fire;
	float fire_rate;                     // controls fire rate, the interval between weapon shots
	float elapsed_last_shot_time_ms;     // controls fire rate, the time that the weapon was fired last
	float projectile_velocity;           // speed of projectiles of this weapon
	int projectile_damage;               // weapon damage
};

// The projectile
struct Projectile {
	int damage;
	Entity weapon; // link the projectile to the weapon
};

// Mob component
struct Mob {
	bool is_tracking_player = false;
	int damage;
	int health = 100000;
};

// Slowing effect for mobs from weapons
struct MobSlowEffect {
	bool applied;
	vec2 initial_velocity;
	float duration_ms;				// how long effect lasts
	float elapsed_slow_time_ms;		// how long effect has been active
	float slow_ratio;				// how much slowing
};

// Structure to store the path for a mob
struct Path {
	std::deque<Entity> path;
};

struct Item {
	ITEM_TYPE data;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 1.f, 1.f };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	vec2 MTV; // minimal translation vector for collision resolution
	float overlap; // magnitude of collision depth

	Collision(Entity& other_entity, float overlap, vec2 MTV) { 
		this->other_entity = other_entity;
		this->overlap = overlap;
		this->MTV = MTV;
		};

};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float timer_ms = 3000.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = { 1,1 };
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// Contains information for use in the camera rendering pipeline
struct Camera
{
	bool mode_follow;
};


enum TERRAIN_TYPE : uint16_t {
	AIR = 0,
	GRASS = AIR + 1,
	ROCK = GRASS + 1,
	TERRAIN_COUNT = ROCK + 1
};

enum TERRAIN_FLAGS : uint32_t {
	COLLIDABLE = 0b1,
	DISABLE_PATHFIND = 0b10,
};

/// <summary>
/// Struct representation of TerrainSystem->Cell's integers.
/// Mainly for reading purposes.
/// Make sure you call TerrainSystem->update_cell if you want to update the cell.
/// </summary>
struct TerrainCell 
{
	TERRAIN_TYPE terrain_type;
	uint16_t flag; 

	TerrainCell(TERRAIN_TYPE terrain_type, int flag) {
		this->terrain_type = terrain_type;
		this->flag = flag;
	};

	TerrainCell(uint32_t terrain_cell) {
		this->flag = (uint16_t)terrain_cell;
		this->terrain_type = static_cast<TERRAIN_TYPE>((uint16_t)(terrain_cell >> 16));
	}

	operator unsigned int() {
		return ((uint32_t)terrain_type << 16) | (uint32_t)flag;
	}
};

// component for entity that have collision, size is the width/height of bounding box
struct Collider
{
	vec2 position;  //position in world coord, used in collision detection. This needs to be updated by physics::step
	std::vector<vec2> points;  // in local coord
	std::vector<vec2> normals; 
	mat2 rotation;	// might need for later when we have entity(mobs) that can rotate its sprites
	vec2 scale; // used for AABB broad phase detection
	int flag;  // for filtering 
};

struct BoundaryBlock {

};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	PLAYER = 0,
	MOB = PLAYER + 1,
	REDBLOCK = MOB + 1,
	FOW = REDBLOCK + 1,
	ITEM = FOW + 1,
	FOOD = ITEM + 1,
	WEAPON_SHURIKEN = FOOD + 1,
	WEAPON_CROSSBOW = WEAPON_SHURIKEN + 1,
	WEAPON_ARROW = WEAPON_CROSSBOW + 1,
	WEAPON_SHOTGUN = WEAPON_ARROW + 1,
	WEAPON_MACHINEGUN = WEAPON_SHOTGUN + 1,
	SPACESHIP = WEAPON_MACHINEGUN + 1,
	BLUEBLOCK = SPACESHIP + 1,
	TEXTURE_COUNT = BLUEBLOCK + 1,
	//PLAYER_SPRITE_SHEET = TEXTURE_COUNT +1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	PLAYER = PEBBLE + 1,
	SALMON = PLAYER + 1,
	TEXTURED = SALMON + 1,
	WATER = TEXTURED + 1,
	TERRAIN = WATER + 1,
	EFFECT_COUNT = TERRAIN + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SPRITE = SALMON + 1,
	PEBBLE = SPRITE + 1,
	DEBUG_LINE = PEBBLE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	// adding player geometry_buffer_ID
	PLAYER_SPRITE = SCREEN_TRIANGLE +1, 
	TERRAIN = PLAYER_SPRITE + 1,
	GEOMETRY_COUNT = TERRAIN + 1
};

enum class RENDER_LAYER_ID {
	LAYER_1 = 0,
	LAYER_2 = LAYER_1 + 1,
	LAYER_3 = LAYER_2 + 1,
	LAYER_COUNT = LAYER_3 + 1

};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER_ID layer_id = RENDER_LAYER_ID::LAYER_COUNT;
};

