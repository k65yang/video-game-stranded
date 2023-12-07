
#define GL3W_IMPLEMENTATION
#pragma GCC diagnostic ignored "-Wswitch"
#include <gl3w.h>

// stlib
#include <chrono>
#include <iostream>
// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "terrain_system.hpp"
#include "pathfinding_system.hpp"
#include "weapons_system.hpp"
#include "particle_system.hpp"
#include "mob_system.hpp"
#include "spaceship_home_system.hpp"
#include "quest_system.hpp"
#include "tutorial_system.hpp"
#include "start_screen_system.hpp"
#include "common.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	TerrainSystem terrain_system;
	PathfindingSystem pathfinding_system;
	WeaponsSystem weapons_system;
	ParticleSystem particle_system;
	MobSystem mob_system;
	AudioSystem audio_system;
	SpaceshipHomeSystem spaceship_home_system;
	StartScreenSystem start_screen_system;
	QuestSystem quest_system;
	TutorialSystem tutorial_system;

	// Initializing window
	GLFWwindow* window = world_system.create_window();

	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// Initialize systems needed to display the start screen
	audio_system.init();		
	render_system.init(window);
	start_screen_system.init(window, &render_system, &terrain_system);

	// Load terrain mesh into the GPU
	std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS> orientation_map;
	terrain_system.generate_orientation_map(orientation_map);	// Gets all the tiles with directional textures
	render_system.initializeTerrainBuffers(orientation_map);

	auto t = Clock::now();
	float total_elapsed_time = 0.f;
	while(!start_screen_system.is_finished()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		auto now = Clock::now();
		float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;
		total_elapsed_time += elapsed_ms;
		
		start_screen_system.step(elapsed_ms);
		render_system.drawStartScreens();
	}
	// start_screen_system.~StartScreenSystem(); // destructor not working properly, segfaulting...

	// Initialize all other systems
	weapons_system.init(&render_system, &physics_system);
	mob_system.init(&render_system, &terrain_system, &physics_system);
	quest_system.init(&render_system);
	spaceship_home_system.init(&render_system, &weapons_system, &quest_system);
	tutorial_system.init(&render_system);
	world_system.init(
		&render_system, 
		&terrain_system, 
		&weapons_system, 
		&physics_system, 
		&mob_system, 
		&audio_system, 
		&spaceship_home_system, 
		&quest_system,
		&tutorial_system
	);

	pathfinding_system.init(&terrain_system);
	particle_system.init(&render_system);

	// variable timestep loop
	while (!world_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// Pause game when player is in spaceship home  
		if (!spaceship_home_system.isHome()) {
			world_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			terrain_system.step(elapsed_ms);
			pathfinding_system.step(elapsed_ms);
			weapons_system.step(elapsed_ms);
			particle_system.step(elapsed_ms);
			mob_system.step(elapsed_ms);
			quest_system.step(elapsed_ms);
			world_system.handle_collisions();
		} else {
			spaceship_home_system.step(elapsed_ms);
		}

		tutorial_system.step(elapsed_ms);

		render_system.draw();
	}

	return EXIT_SUCCESS;
}
