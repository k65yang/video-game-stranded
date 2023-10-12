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

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer, TerrainSystem* terrain_arg);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void camera_controls(int action, int key);
	void on_mouse_move(vec2 pos);

	// TODO: improve input checking
	// This helps prevent button-up actions from happening if game is reset in-between
	// button-down and button-up states
	int key_downs = 0;

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
	Entity main_camera;
	Entity fow;

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Movement with velocity 
	enum MovementKeyIndex {
		LEFT = 0,
		RIGHT = LEFT + 1,
		UP = RIGHT + 1,
		DOWN = UP + 1
		};

	int key_to_index(int key);
	void player_movement(int key, int action);    // player movement helper
	void reset_key(int key);
	bool keyDown[4];    // see MovementKeyIndex for context

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
	bool check_spawn_location_used(vec2 position);

	/// <summary>
	/// Gets a random position in the map that has not already been used as a spawn location	
	///	</summary>
	/// <returns>A position that </returns>
	vec2 get_random_spawn_location();
};
