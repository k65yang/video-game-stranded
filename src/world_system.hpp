#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "terrain_system.hpp"
#include "weapons_system.hpp"
#include "mob_system.hpp"
#include "physics_system.hpp"
#include "save.hpp"
#include "audio_system.hpp"
#include "spaceship_home_system.hpp"
#include "quest_system.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Movement with velocity 
	enum InputKeyIndex {
		LEFT = 0,
		RIGHT = LEFT + 1,
		UP = RIGHT + 1,
		DOWN = UP + 1,
		CAMERA_LEFT = DOWN + 1,
		CAMERA_RIGHT = CAMERA_LEFT + 1,
		CAMERA_UP = CAMERA_RIGHT + 1,
		CAMERA_DOWN = CAMERA_UP + 1,
		KEYS = CAMERA_DOWN + 1
	};

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(
		RenderSystem* renderer, 
		TerrainSystem* terrain_arg, 
		WeaponsSystem* weapons_system_arg, 
		PhysicsSystem* physics_system_arg,
		MobSystem* mob_system_arg, 
		AudioSystem* audio_system_arg, 
		SpaceshipHomeSystem* spaceship_home_system_arg,
		QuestSystem* quest_system_arg
	);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	/// <summary>
	/// Generic movement controller function for entities that move via player input
	/// </summary>
	/// <param name="motion">The motion component of the entity that will be controlled</param>
	/// <param name="indexStart">The 'InputKeyIndex::XX_LEFT' associated with the left-moving key</param>
	/// <param name="invertDirection">Should the directions be inverted?</param>
	/// <param name="useAbsoluteVelocity">Is player velocity dependent on the direction the player is facing?</param>
	void handle_movement(Motion& motion, InputKeyIndex indexStart, bool invertDirection = false, bool useAbsoluteVelocity = true);

	// Check for collisions
	void handle_collisions();
	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void process_editor_controls(int action, int key);
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mods);
	vec2 interpolate(vec2 p1, vec2 p2, float param);

	// restart level
	void restart_game();

	void load_game(json j);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	RenderSystem* renderer;
	TerrainSystem* terrain;
	WeaponsSystem* weapons_system;
	MobSystem* mob_system;
	PhysicsSystem* physics_system;
	AudioSystem* audio_system;
	SpaceshipHomeSystem* spaceship_home_system;
	QuestSystem* quest_system;
	float current_speed;
	float next_turtle_spawn;
	float next_fish_spawn;
	Entity player_salmon;
	Entity player_equipped_weapon;
	Entity main_camera;
	Entity fow;
	Entity health_bar;
	Entity health_frame; 
	Entity food_frame; 
	Entity food_bar;
	Entity weapon_indicator;
	Entity powerup_indicator;
	Entity ammo_indicator; 
	Entity spaceship;
	Entity spaceship_home; 
	Entity help_bar;
	bool tooltips_on = true;

	bool user_has_first_weapon = false;
	bool user_has_powerup = false;

	int current_tooltip = 0;
	std::vector<TEXTURE_ASSET_ID> tooltips = { TEXTURE_ASSET_ID::HELP_ONE, TEXTURE_ASSET_ID::HELP_TWO, TEXTURE_ASSET_ID::HELP_THREE, TEXTURE_ASSET_ID::HELP_FOUR };

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Vector to keep track of locations where an item/mob has been spawned
	std::vector<vec2> used_spawn_locations;

	// Map editor fields
	TERRAIN_TYPE editor_terrain = AIR; // Changes clicked tiles to this during debug mode
	uint16_t editor_flag = 0; // Changes clicked tiles' flags to the given value.
	bool editor_place_tile = false;

	/// <summary>
	/// Repeatedly replaces the clicked tile into editor specifications if applicable
	/// </summary>
	void map_editor_routine();

	/// <summary>
	/// Spawns ITEM_LIMIT items randomly across the map
	///	</summary>
	void spawn_items();

	void load_spawned_items_mobs(json& j);

	/// <summary>
	/// Maps the GLFW key into a InputKeyIndex as an int
	/// </summary>
	/// <param name="key">A GLFW-defined key ie. 'GLFW_KEY_...'</param>
	/// <returns>A InputKeyIndex value as an int</returns>
	int key_to_index(int key);

	/// <summary>
	/// Updates the state of the keys in InputKeyIndex and keyDown[KEYS]
	/// </summary>
	/// <param name="key">A GLFW-defined key ie. 'GLFW_KEY_...'</param>
	/// <param name="action">A GLFW-defined key ie. 'GLFW_PRESSED' </param>
	void update_key_presses(int key, int action);

	/// <summary>
	/// Sets camera follow mode to true if no camera-control buttons are pressed
	/// </summary>
	void update_camera_follow();

	/// <summary>
	///  Sets the player's facing direction based on the cursor angle (aiming direction)
	/// </summary>
	void updatePlayerDirection();

	/// <summary>
	// Handles various aspects related to player movement, 
	// including updating the walking animation and tracking the total distance traveled.
	/// </summary>
	/// <param name="elapsed_ms_since_last_update"></param>
	void handlePlayerMovement(float elapsed_ms_since_last_update);
	bool keyDown[KEYS];    // Uses InputKeyIndex values as index
};
