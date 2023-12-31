#pragma once
#include "common.hpp"
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include "../ext/stb_image/stb_image.h"
#include <map>

// Player component

const int PLAYER_MAX_FOOD = 100; // change to 100 later
const int PLAYER_MAX_HEALTH = 100;
const int SPACESHIP_MAX_HEALTH_STORAGE = 100;
const int SPACESHIP_MAX_FOOD_STORAGE = 200;
const int SPACESHIP_MAX_AMMO_STORAGE = 100;

enum class QUEST_ITEM_STATUS {
	NOT_FOUND = 0,
	FOUND = NOT_FOUND + 1,
	SUBMITTED = FOUND + 1
};

enum class RESOURCE_TYPE {
	AMMO = 0,
	FOOD = AMMO + 1,
	HEALTH = FOOD + 1
};

enum class TUTORIAL_TYPE {
	QUEST_ITEM_TUTORIAL = 0,
	SPACESHIP_HOME_TUTORIAL = QUEST_ITEM_TUTORIAL + 1,
	GAME_SAVED = SPACESHIP_HOME_TUTORIAL + 1,
	GAME_LOADED = GAME_SAVED + 1
};

enum class ITEM_TYPE {
	QUEST_ONE = 0,
	QUEST_TWO = QUEST_ONE + 1,
	QUEST_THREE = QUEST_TWO + 1,
	QUEST_FOUR = QUEST_THREE + 1,
	WEAPON_NONE = QUEST_FOUR + 1,
	WEAPON_SHURIKEN = WEAPON_NONE + 1,
	WEAPON_CROSSBOW = WEAPON_SHURIKEN + 1,
	WEAPON_SHOTGUN = WEAPON_CROSSBOW + 1,
	WEAPON_MACHINEGUN = WEAPON_SHOTGUN + 1,
	FOOD = WEAPON_MACHINEGUN + 1,
	WEAPON_UPGRADE = FOOD + 1,
	POWERUP_SPEED = WEAPON_UPGRADE + 1,
	POWERUP_HEALTH = POWERUP_SPEED + 1,
	POWERUP_INVISIBLE = POWERUP_HEALTH + 1,
	POWERUP_INFINITE_BULLET = POWERUP_INVISIBLE + 1,
	UPGRADE = POWERUP_INFINITE_BULLET + 1
};

enum class BAR_TYPE {
	HEALTH_BAR = 0,
	FOOD_BAR = HEALTH_BAR + 1,
	AMMO_BAR = FOOD_BAR + 1,
	HEALTH_STORAGE_BAR = AMMO_BAR + 1,
	FOOD_STORAGE_BAR = HEALTH_STORAGE_BAR + 1,
	AMMO_STORAGE_BAR = FOOD_STORAGE_BAR + 1,
};

enum class FRAME_TYPE {
	HEALTH_FRAME = 0, 
	FOOD_FRAME = HEALTH_FRAME + 1,
	STORAGE_FRAME = FOOD_FRAME + 1,
};

enum class POWERUP_TYPE {
	
	SPEED = 0,
	HEALTH_REGEN = SPEED + 1,
	INVISIBLE = HEALTH_REGEN + 1,
	INFINITE_BULLET = INVISIBLE + 1,

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
	bool is_home = false;
	bool has_collected_quest_item = false;
	bool has_entered_spaceship = false;
	float current_speed = 5.0f; // change to 5 later
};



struct QuestItemIndicator {
	ITEM_TYPE quest_item;
};

struct Inventory {
	std::map<ITEM_TYPE, QUEST_ITEM_STATUS> quest_items {
		{ITEM_TYPE::QUEST_ONE, QUEST_ITEM_STATUS::NOT_FOUND},
		{ITEM_TYPE::QUEST_TWO, QUEST_ITEM_STATUS::NOT_FOUND},
	};
};

struct Powerup {
	POWERUP_TYPE type;
	float duration_ms = 0.0f;
};


// Knockback effect for player from mobs
struct PlayerKnockbackEffect {
	float duration_ms;				
	float elapsed_knockback_time_ms;
};

// Inaccuracy effect for player from mobs
struct PlayerInaccuracyEffect {
	float duration_ms;				
	float elapsed_inaccuracy_time_ms;
	float inaccuracy_percent;
};

// The weapon
struct Weapon {
	ITEM_TYPE weapon_type = ITEM_TYPE::WEAPON_NONE;
	bool can_fire;
	float fire_rate;                     // controls fire rate, the interval between weapon shots
	float elapsed_last_shot_time_ms;     // controls fire rate, the time that the weapon was fired last
	float projectile_velocity;           // speed of projectiles of this weapon
	int projectile_damage;               // weapon damage
	float knockback_force;				 // magnitude of knockback on enemy
	int ammo_count;						 // Ammo
	int level;							 // Weapon level
};

struct SpaceshipHome {
	int health_storage;
	int food_storage;
	int ammo_storage;
};

struct Spaceship {
};

// The projectile
struct Projectile {
	int damage;
	Entity weapon; // link the projectile to the weapon
};

enum class MOB_TYPE {
	SLIME = 0,
	GHOST = SLIME + 1,
	BRUTE = GHOST + 1,
	DISRUPTOR = BRUTE + 1,
	TURRET = DISRUPTOR + 1,
};

// Mob component
struct Mob {
	bool is_tracking_player = false;
	int damage;
	int aggro_range;
	int health;
	float speed_ratio;
	Entity curr_cell;
	MOB_TYPE type;
	Entity health_bar;
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

struct Text {
	std::string str;
	vec3 color;
	float scale;
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
	bool hide_ui = 0;
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
	float timer_ms = 5000.f;
};

struct Tutorial {
	float timer_ms = 5000.f;
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

enum ZONE_NUMBER {
	ZONE_0 = 0,
	ZONE_1 = 1,
	ZONE_2 = 2,
	ZONE_3 = 3,
	ZONE_4 = 4,
	ZONE_5 = 5,
	ZONE_COUNT = ZONE_5 + 1,
};


enum TERRAIN_TYPE : uint16_t {
	AIR = 0,
	GRASS = AIR + 1,
	ROCK = GRASS + 1,
	SAND = ROCK + 1,
	MUD = SAND + 1,
	SHALLOW_WATER = MUD + 1,
	DEEP_WATER = SHALLOW_WATER + 1,
	TERRAIN_COUNT = DEEP_WATER + 1
};

// Specifies which textures are directional
const std::unordered_set<TERRAIN_TYPE> directional_terrain = {
	ROCK,
};

enum TERRAIN_FLAGS : uint32_t {
	COLLIDABLE =			0b1,
	DISABLE_PATHFIND =		0b10,
	ALLOW_SPAWNS =			0b100,
};

enum TERRAIN_MASKS : uint32_t {
	FLAGS = 0xFF,			// first 8 bits
	TYPES = 0xFFFF0000,		// last 16 bits
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
		from_uint32(terrain_cell);
	}

	void from_uint32(uint32_t terrain_cell) {
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

// component for entity using sprite sheet animation
struct Animation {
	int framex = 0; // row index on the sprite from sprite sheet
	int framey = 0; // column index 

	float frame_dimension_w = 1.0f;
	float frame_dimension_h = 1.0f;
	// frame dimension on width and height are set up under initializeGlGeometryBuffers() from render_system_init.cpp
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
	PLAYER_STANDING = PLAYER + 1,
	PLAYER_PARTICLE = PLAYER_STANDING + 1,
	SLIME = PLAYER_PARTICLE + 1,
	RED_BLOCK = SLIME + 1,
	WEAPON_UPGRADE = RED_BLOCK + 1,
	FOOD = WEAPON_UPGRADE + 1,
	WEAPON_SHURIKEN = FOOD + 1,
	WEAPON_CROSSBOW = WEAPON_SHURIKEN + 1,
	WEAPON_ARROW = WEAPON_CROSSBOW + 1,
	WEAPON_SHOTGUN = WEAPON_ARROW + 1,
	WEAPON_MACHINEGUN = WEAPON_SHOTGUN + 1,
	ICON_NO_WEAPON = WEAPON_MACHINEGUN + 1,
	ICON_SHURIKEN = ICON_NO_WEAPON + 1,
	ICON_CROSSBOW = ICON_SHURIKEN + 1,
	ICON_SHOTGUN = ICON_CROSSBOW + 1,
	ICON_MACHINE_GUN = ICON_SHOTGUN + 1,
	ICON_POWERUP_SPEED = ICON_MACHINE_GUN + 1,
	ICON_POWERUP_HEALTH = ICON_POWERUP_SPEED + 1,
	POWERUP_HEALTH = ICON_POWERUP_HEALTH + 1,
	POWERUP_SPEED = POWERUP_HEALTH + 1,
	POWERUP_INVISIBLE = POWERUP_SPEED + 1,
	POWERUP_INFINITE_BULLET = POWERUP_INVISIBLE + 1,
	SPACESHIP = POWERUP_INFINITE_BULLET + 1,
	SPACESHIP_HOME = SPACESHIP + 1,
	BLUE_BLOCK = SPACESHIP_HOME + 1,
	BROWN_BLOCK = BLUE_BLOCK + 1,
	BLACK_BLOCK = BROWN_BLOCK + 1,
	STORAGE_FRAME = BLACK_BLOCK + 1,
	HEALTH_FRAME = STORAGE_FRAME + 1,
	FOOD_FRAME = HEALTH_FRAME + 1,
	SPACESHIP_HOME_HEALTH = FOOD_FRAME + 1,
	SPACESHIP_HOME_AMMO = SPACESHIP_HOME_HEALTH + 1,
	SPACESHIP_HOME_FOOD = SPACESHIP_HOME_AMMO + 1,
	QUEST_1_NOT_FOUND = SPACESHIP_HOME_FOOD + 1,
	QUEST_1_FOUND = QUEST_1_NOT_FOUND + 1,
	QUEST_1_SUBMITTED = QUEST_1_FOUND + 1,
	QUEST_2_NOT_FOUND = QUEST_1_SUBMITTED + 1,
	QUEST_2_FOUND = QUEST_2_NOT_FOUND + 1,
	QUEST_2_SUBMITTED = QUEST_2_FOUND + 1,
	QUEST_3_NOT_FOUND = QUEST_2_SUBMITTED + 1,
	QUEST_3_FOUND = QUEST_3_NOT_FOUND + 1,
	QUEST_3_SUBMITTED = QUEST_3_FOUND + 1,
	QUEST_4_NOT_FOUND = QUEST_3_SUBMITTED + 1,
	QUEST_4_FOUND = QUEST_4_NOT_FOUND + 1,
	QUEST_4_SUBMITTED = QUEST_4_FOUND + 1,
	QUEST_1_ITEM = QUEST_4_SUBMITTED + 1,
	QUEST_2_ITEM = QUEST_1_ITEM +1,
	QUEST_3_ITEM = QUEST_2_ITEM + 1,
	QUEST_4_ITEM = QUEST_3_ITEM + 1,
	QUEST_1_BUILT = QUEST_4_ITEM + 1,
	QUEST_2_BUILT = QUEST_1_BUILT + 1,
	QUEST_3_BUILT = QUEST_2_BUILT + 1,
	QUEST_4_BUILT = QUEST_3_BUILT + 1,
	HELP_BUTTON = QUEST_4_BUILT + 1,
	HELP_DIALOG = HELP_BUTTON + 1,
	QUEST_ITEM_TUTORIAL_DIALOG = HELP_DIALOG + 1,
	SPACESHIP_HOME_TUTORIAL_DIALOG = QUEST_ITEM_TUTORIAL_DIALOG + 1,
	GHOST = SPACESHIP_HOME_TUTORIAL_DIALOG + 1,
	SPACESHIP_DEPART = GHOST + 1, 
	BRUTE = SPACESHIP_DEPART + 1,
	DISRUPTOR = BRUTE + 1,
	DEATH_TEXT_F = DISRUPTOR + 1,
	DEATH_TEXT_H = DEATH_TEXT_F + 1,
	VICTORY_TEXT = DEATH_TEXT_H + 1,
	HEART_PARTICLE = VICTORY_TEXT + 1,
	START_SCREEN_ONE = HEART_PARTICLE + 1,
	START_SCREEN_TWO = START_SCREEN_ONE + 1,
	START_BUTTON = START_SCREEN_TWO + 1,
	START_BUTTON_HOVER = START_BUTTON + 1,
	MUZZLE_SHEET = START_BUTTON_HOVER + 1,
	POINTING_ARROW = MUZZLE_SHEET + 1,
	SIDEICON_SHURIKEN = POINTING_ARROW + 1,
	SIDEICON_CROSSBOW = SIDEICON_SHURIKEN + 1,
	SIDEICON_SHOTGUN = SIDEICON_CROSSBOW + 1,
	SIDEICON_MACHINEGUN = SIDEICON_SHOTGUN + 1,
	TEXTURE_COUNT = SIDEICON_MACHINEGUN + 1,



};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PEBBLE = COLOURED + 1,
	SPRITESHEET = PEBBLE + 1,
	SALMON = SPRITESHEET + 1,
	TEXTURED = SALMON + 1,
	FOG = TEXTURED + 1,
	TERRAIN = FOG + 1,
	PARTICLE = TERRAIN + 1,
	TEXTUREPARTICLE = PARTICLE + 1,
	TEXT = TEXTUREPARTICLE + 1,
	EFFECT_COUNT = TEXT + 1

};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SPRITE = SALMON + 1,
	PEBBLE = SPRITE + 1,
	DEBUG_LINE = PEBBLE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	// adding player geometry_buffer_ID
	PLAYER_SPRITE = SCREEN_TRIANGLE + 1,
	// adding mobs geo_buffer_ID 
	MOB_SPRITE = PLAYER_SPRITE + 1,
	SPACESHIP_DEPART_SPRITE = MOB_SPRITE + 1,
	TERRAIN = SPACESHIP_DEPART_SPRITE + 1,
	PLAYER_MESH = TERRAIN + 1,
	MOB001_MESH = PLAYER_MESH + 1,
	TEXT = MOB001_MESH + 1,
	MUZZLEFLASH_SPRITE = TEXT + 1,
	GEOMETRY_COUNT = MUZZLEFLASH_SPRITE + 1

};

enum class RENDER_LAYER_ID {
	LAYER_1 = 0,
	LAYER_2 = LAYER_1 + 1,
	LAYER_3 = LAYER_2 + 1,      
	LAYER_4 = LAYER_3 + 1,      // UI elements
	LAYER_5 = LAYER_4 + 1,      
	LAYER_COUNT = LAYER_5 + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER_ID layer_id = RENDER_LAYER_ID::LAYER_COUNT;
};

struct InstancedRenderRequest {
	std::vector<Entity> entities;
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER_ID layer_id = RENDER_LAYER_ID::LAYER_COUNT;
};

// Particle effects
const int NUM_PARTICLES = 200;

// Struct describing particles 
struct Particle {
	bool active = false;
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::TEXTURE_COUNT; //HARDCODED TEMPORARY TO THIS FOR NOW
	float lifeTime = 1000.0f; // in ms
	float lifeTimeRemaining = 0.0f;
	float sizeBegin, sizeEnd;

}; 

struct ParticleTemplate {
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::TEXTURE_COUNT; //HARDCODED TEMPORARY TO THIS FOR NOW
	bool active = false;
	float lifeTime = 1000.0f; // in ms
	float lifeTimeRemaining = 1000.0f;
	float sizeBegin = 1.0f;
	float sizeEnd = 0.0f;
	vec2 position = {0.f, 0.f};
	vec2 velocity = { 0.f, 0.f };
	vec4 color = vec4{ 1.0f };

};

struct PointingArrow {

	PointingArrow(Entity target) {
		this->target = target;
	}

	Entity target;
	vec2 radius_offset = { 5, 0 };
};