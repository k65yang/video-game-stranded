
#define GL3W_IMPLEMENTATION
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
#include "mob_system.hpp"
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
	MobSystem mob_system;

	// Initializing window
	GLFWwindow* window = world_system.create_window();

	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	render_system.init(window);
	weapons_system.init(&render_system);
	mob_system.init(&render_system, &terrain_system);
	world_system.init(&render_system, &terrain_system, &weapons_system, &physics_system, &mob_system);

	// Load terrain mesh into the GPU
	std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS> orientation_map;
	terrain_system.generate_orientation_map(orientation_map);	// Gets all the tiles with directional textures
	render_system.initializeTerrainBuffers(orientation_map);

	pathfinding_system.init(&terrain_system);
	

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// this pauses the world system when player is at home  
		if (!world_system.is_home()) {
			world_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			terrain_system.step(elapsed_ms);
			pathfinding_system.step(elapsed_ms);
			weapons_system.step(elapsed_ms);
			mob_system.step(elapsed_ms);
			world_system.handle_collisions();
			}
		render_system.draw();

		//else do home step function

	}

	return EXIT_SUCCESS;
}
