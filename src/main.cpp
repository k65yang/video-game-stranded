
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
#include "common.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

using Clock = std::chrono::high_resolution_clock;

typedef struct {
	unsigned int	textureId;	// texture id storing character
	glm::ivec2		size;		// size of char
	glm::ivec2		bearing;	// distance from origin to top left of char
	unsigned int	advance;	// distance from origin to next origin (1/64th pixels)
} Character;

std::map<char, Character> chars;

// Entry point
int main()
{

	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return -1;
	}
	
	FT_Face face;
	if (FT_New_Face(ft, "C:/Users/venus/source/repos/Team13-Stranded/fonts/arial.ttf", 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		return -1;
	}


	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	//// load first 128 characters of ASCII set
	//for (unsigned char c = 0; c < 128; c++) {
	//	// load glyph
	//	if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
	//		continue;
	//	}

	//	// generate texture
	//	GLuint texture;
	//	glGenTextures(1, &texture);
	//	glBindTexture(GL_TEXTURE_2D, texture);
	//	glTexImage2D(
	//		GL_TEXTURE_2D,
	//		0,
	//		GL_RED,
	//		face->glyph->bitmap.width,
	//		face->glyph->bitmap.rows,
	//		0,
	//		GL_RED,
	//		GL_UNSIGNED_BYTE,
	//		face->glyph->bitmap.buffer
	//	);

	//	// set texture parameters
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	// Print character information for debugging
	//	std::cout << "Character: " << static_cast<int>(c)
	//		<< ", Glyph width: " << face->glyph->bitmap.width
	//		<< ", Glyph rows: " << face->glyph->bitmap.rows
	//		<< std::endl;
	//	// store character texture for use
	//	chars[c] = {
	//		texture,
	//		glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
	//		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
	//		(unsigned int)face->glyph->advance.x
	//	};
	//}
	//glBindTexture(GL_TEXTURE_2D, 0);
	//// destroy FreeType once we're finished
	//FT_Done_Face(face);
	//FT_Done_FreeType(ft);

	//unsigned int VAO, VBO;

	//// configure VAO/VBO for texture quads
	//// -----------------------------------
	//glGenVertexArrays(1, &VAO);
	//glGenBuffers(1, &VBO);
	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

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

	// Initializing window
	GLFWwindow* window = world_system.create_window();

	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	audio_system.init();
	render_system.init(window);
	weapons_system.init(&render_system);
	mob_system.init(&render_system, &terrain_system);
	world_system.init(&render_system, &terrain_system, &weapons_system, &physics_system, &mob_system, &audio_system);

	// Load terrain mesh into the GPU
	std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS> orientation_map;
	terrain_system.generate_orientation_map(orientation_map);	// Gets all the tiles with directional textures
	render_system.initializeTerrainBuffers(orientation_map);

	pathfinding_system.init(&terrain_system);
	particle_system.init(&render_system);

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
			particle_system.step(elapsed_ms);
			mob_system.step(elapsed_ms);
			world_system.handle_collisions();
		}
		render_system.draw();

		//else do home step function

	}

	return EXIT_SUCCESS;
}
