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
	/// <param name="index_start">The 'InputKeyIndex::XX_LEFT' associated with the left-moving key</param>
	/// <param name="invertDirection">Should the directions be inverted?</param>
	void handle_movement(Motion& motion, InputKeyIndex index_start, bool invertDirection = false);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

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
};
