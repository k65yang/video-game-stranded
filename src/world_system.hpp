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
	void init(RenderSystem* renderer, TerrainSystem* terrain_arg);

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
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mods);
	vec2 interpolate(vec2 p1, vec2 p2, float param);

	// restart level
	void restart_game();

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	vec2 zone1_boundary_size = {15,15};

	// Game state
	RenderSystem* renderer;
	TerrainSystem* terrain;
	float current_speed;
	float next_turtle_spawn;
	float next_fish_spawn;
	Entity player_salmon;
	Entity player_equipped_weapon;
	Entity main_camera;
	Entity fow;
	Entity health_bar;
	Entity food_bar;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Random item and mob spawning 
	// Limits on the number of items and mobs
	const int ITEM_LIMIT = 4;
	const int MOB_LIMIT = 2;

	// Vector to keep track of locations where an item/mob has been spawned
	std::vector<vec2> used_spawn_locations;

	/// <summary>
	/// Spawns ITEM_LIMIT items randomly across the map
	///	</summary>
	void spawn_items();

	/// <summary>
	/// Spawns MOB_LIMIT mobs randomly across the map
	///	</summary>
	void spawn_mobs();

	/// <summary>
	/// Checks if a position has already been used as a spawn location
	///	</summary>
	/// <param name="position">The position to check</param>
	/// <returns>True if the position has been used as a spawn location, false otherwise</returns>
	bool is_spawn_location_used(vec2 position);

	/// <summary>
	/// Gets a random position in the map that has not already been used as a spawn location	
	///	</summary>
	/// <returns>A position that </returns>
	vec2 get_random_spawn_location();

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
	bool keyDown[KEYS];    // Uses InputKeyIndex values as index

	// calculate the corrected position based on size of fraction.
	// 

	/// <summary>
	/// helper function for terrain collision response, calculates the corrected player position
	/// </summary>
	/// <param name="position"> pass in either player.position.x or y</param>
	float positionCorrection(float position);

	/// <summary>
	/// Creates a weapon and adds it to the weapons registry. Auto-equips the created weapon.
	/// NOTE: Does not check if the weapon exists already. 
	///       (If we do not limit weapon spawns we can be create/equip the same weapon multiple times)
	/// </summary>
	/// <param name="weapon_type"> The type of weapon to be created</param>
	Entity createAndEquipWeapon(ITEM_TYPE weapon_type);
};
