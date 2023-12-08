#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include <unordered_set>

#include <ft2build.h>
#include FT_FREETYPE_H

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
public:
	/// TODO: add more orientations
	enum ORIENTATIONS : uint8 {
		ISOLATED,
		EDGE_BOTTOM,
		EDGE_TOP,
		EDGE_LEFT,
		EDGE_RIGHT,
		CORNER_OUTER_TOP_LEFT,
		CORNER_OUTER_TOP_RIGHT,
		CORNER_OUTER_BOTTOM_LEFT,
		CORNER_OUTER_BOTTOM_RIGHT,
		CORNER_INNER_TOP_LEFT,
		CORNER_INNER_TOP_RIGHT,
		CORNER_INNER_BOTTOM_LEFT,
		CORNER_INNER_BOTTOM_RIGHT,
		DOUBLE_EDGE_VERTICAL,
		DOUBLE_EDGE_HORIZONTAL,
		DOUBLE_EDGE_END_TOP,
		DOUBLE_EDGE_END_BOTTOM,
		DOUBLE_EDGE_END_LEFT,
		DOUBLE_EDGE_END_RIGHT,
		INSIDE,
		ORIENTATIONS_COUNT
	};
private:
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path

	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SALMON, mesh_path("salmon.obj")),

		  // mesh for player collider
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER_MESH, mesh_path("player_standing_V9.obj")),

		  // mesh for player collider
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::MOB001_MESH, mesh_path("MOB001.obj"))

		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("player_spritesheet.png"),
			textures_path("player_standing.png"),
			textures_path("player_particle.png"),
			textures_path("mob_spritesheet.png"),
			textures_path("red_block.png"),
			textures_path("weapon_upgrade.png"),
			textures_path("food.png"),
			textures_path("weapon_shuriken.png"),
			textures_path("weapon_crossbow.png"),
			textures_path("crossbow_arrow.png"),
			textures_path("weapon_shotgun.png"),
			textures_path("weapon_machine_gun.png"),
			textures_path("icon_no_weapon.png"),
			textures_path("icon_shuriken.png"),
			textures_path("icon_crossbow.png"),
			textures_path("icon_shotgun.png"),
			textures_path("icon_machine_gun.png"),
			textures_path("icon_powerup_speed.png"),
			textures_path("icon_powerup_health.png"),
			textures_path("powerup_health.png"),
			textures_path("powerup_speed.png"),
			textures_path("spaceship.png"),
			textures_path("spaceship_home.png"),
			textures_path("blue_block.png"),
			textures_path("brown_block.png"),
			textures_path("black_block.png"),
			textures_path("bar_frame.png"),
			textures_path("health_empty_bar.png"),
			textures_path("food_empty_bar.png"),
			textures_path("spaceship_home_health.png"),
			textures_path("spaceship_home_ammo.png"),
			textures_path("spaceship_home_food.png"),
			textures_path("q1_not_found.png"),
			textures_path("q1_found.png"),
			textures_path("q1_submitted.png"),
			textures_path("q2_not_found.png"),
			textures_path("q2_found.png"),
			textures_path("q2_submitted.png"),
			textures_path("q3_not_found.png"),
			textures_path("q3_found.png"),
			textures_path("q3_submitted.png"),
			textures_path("q4_not_found.png"),
			textures_path("q4_found.png"),
			textures_path("q4_submitted.png"),
			textures_path("q1.png"),
			textures_path("q2.png"),
			textures_path("q3.png"),
			textures_path("q4.png"),
			textures_path("q1_built.png"),
			textures_path("q2_built.png"),
			textures_path("q3_built.png"),
			textures_path("q4_built.png"),
			textures_path("help_button.png"),
			textures_path("help_dialog.png"),
			textures_path("quest_item_tutorial_dialog.png"),
			textures_path("spaceship_home_tutorial_dialog.png"),
			textures_path("ghost.png"),
			textures_path("spaceship_depart.png"),
			textures_path("brute.png"),
			textures_path("disruptor.png"),
			textures_path("death_text_f.png"),
			textures_path("death_text_h.png"),
			textures_path("victory_text.png"),
			textures_path("heart_particle.png"),
			// Starting screen textures
			textures_path("start_screen.png"),
			textures_path("intro_screen.png"),
			textures_path("start_button.png"),
			textures_path("start_button_hover.png"),
			textures_path("muzzle_flash_sheet.png"),
			textures_path("ship_arrow.png"),

			textures_path("shuriken_side_icon.png"),
			textures_path("crossbow_side_icon.png"),
			textures_path("shotgun_side_icon.png"),
			textures_path("machinegun_side_icon.png"),


	};

	// How big one terrain spritesheet is
	const ivec2 terrain_sheet_size =	{ 300, 200 };
	// Number of "frames" per sprite sheet
	const ivec2 terrain_sheet_n =		{ terrain_sheet_size.x / tile_size_px,
										  terrain_sheet_size.y / tile_size_px };
	// How big one "frame" is in that spritesheet is

	// TODO: properly handle GL_LINEAR behaviour in texture atlasses instead of this hacky method.
	// Basically, we're definining the "edge" texels/pixels of a tile
	// as a "border" for GL_LINEAR filtering to use
	const vec2	terrain_sheet_uv =		{ (float)tile_size_px / terrain_sheet_size.x,
										  (float)tile_size_px  / terrain_sheet_size.y };
	const vec2 terrain_texel_offset =	{ 1.f / terrain_sheet_size.x, 
										  1.f / terrain_sheet_size.y };

	// This is used for generating the texture array for batched renders
	// NOTE: all images must have the same dimensions.
	const std::array<std::string, TERRAIN_TYPE::TERRAIN_COUNT> terrain_texture_paths = {
		terrain_texture_path("0.png"),
		terrain_texture_path("1.png"),
		terrain_texture_path("2.png"),
		terrain_texture_path("3.png"),
		terrain_texture_path("4.png"),
		terrain_texture_path("5.png"),
		terrain_texture_path("6.png"),
	};

	// Keep these in case we want some orientations to be flipped textures
	const std::unordered_set<ORIENTATIONS> mirror_vertical_orientations = {
		
	};

	const std::unordered_set<ORIENTATIONS> mirror_horizontal_orientations = {
		
	};

	const std::unordered_set<ORIENTATIONS> rotate_orientations = {
		
	};

	// Think of these like grid positions in the terrain atlas
	const std::unordered_map<ORIENTATIONS, ivec2> terrain_atlas_offsets = {
		{ISOLATED,						{0, 0}},
		{EDGE_BOTTOM,					{1, 3}},
		{EDGE_TOP,						{1, 1}},
		{EDGE_LEFT,						{0, 2}},
		{EDGE_RIGHT,					{2, 2}},
		{CORNER_OUTER_TOP_LEFT,			{0, 1}},
		{CORNER_OUTER_TOP_RIGHT,		{2, 1}},
		{CORNER_OUTER_BOTTOM_LEFT,		{0, 3}},
		{CORNER_OUTER_BOTTOM_RIGHT,		{2, 3}},
		{CORNER_INNER_TOP_LEFT,			{3, 1}},
		{CORNER_INNER_TOP_RIGHT,		{5, 1}},
		{CORNER_INNER_BOTTOM_LEFT,		{3, 3}},
		{CORNER_INNER_BOTTOM_RIGHT,		{5, 3}},
		{DOUBLE_EDGE_VERTICAL,			{1, 0}},
		{DOUBLE_EDGE_HORIZONTAL,		{2, 0}},
		{DOUBLE_EDGE_END_TOP,			{4, 1}},
		{DOUBLE_EDGE_END_BOTTOM,		{4, 3}},
		{DOUBLE_EDGE_END_LEFT,			{3, 2}},
		{DOUBLE_EDGE_END_RIGHT,			{5, 2}},
		{INSIDE,						{1, 2}},
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("pebble"),
		shader_path("spritesheet"), 
		shader_path("salmon"),
		shader_path("textured"),
		shader_path("fog"),
		shader_path("terrain"),
		shader_path("particle"),
		shader_path("textureParticle"),
		shader_path("text")};


	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	// fog of war related variable

	// enable/disable fog of war
	int enableFow = 1;

	// visibil distance limit for mobs and items 
	float fow_radius = 4.5f;

	// darken factor for outside fog of war
	float fow_darken_factor = 0.25f;


	// Stores the current actual window resolution. You may use this instead of the slower
	// glfwGetVideoMode(glfwGetPrimaryMonitor()) method.
	ivec2 window_resolution;

	/// <summary>
	/// Binds and allocates a given vertex and index buffer under the "index" that is gid in GPU memory.
	/// </summary>
	/// <typeparam name="T">Vertex type</typeparam>
	/// <typeparam name="U">Index type (bit depth)</typeparam>
	/// <param name="gid">The "index" to load from the CPU-side geometry buffer.</param>
	/// <param name="vertices">The vertex buffer</param>
	/// <param name="indices">The index buffer</param>
	template <class T, class U>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<U> indices);

	/// <summary>
	/// Generates the vertex and index buffers for batch rendering terrain.
	/// </summary>
	void initializeTerrainBuffers(std::unordered_map<unsigned int, ORIENTATIONS>& directional_tex_orientations);

	void initializeGlTextures();

	/// <summary>
	/// Generates 2D texture arrays for batch rendering.
	/// </summary>
	void initializeGl3DTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };
	void initializeSpriteSheetQuad(GEOMETRY_BUFFER_ID gid, int numberOfRowSprites, float numberOfColumnSprites);

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader

	bool initScreenTexture();

	// Initialize text vertex buffers and text vao
	void initializeTextRenderer();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities when in the world
	void draw();

	// Draw all entities when in the start screens
	void drawStartScreens();

	mat3 createModelMatrix(Entity entity);
	mat3 createScaledProjectionMatrix();
	mat3 createUnscaledProjectionMatrix();

	/// <summary>
	/// Modifies the terrain vertex buffer to regenerate rendering values for a specific tile.
	/// </summary>
	/// <param name="cell">The tile</param>
	/// <param name="i">The tile's index in the Cell array</param>
	/// <param name="data">The updated render request</param>
	void changeTerrainData(Entity cell, unsigned int i, TerrainCell& data, uint8_t frameValue = 0);

	/// <summary>
	/// Releases the frame buffer. Make sure you call initializeTerrainBuffers again.
	/// </summary>
	void empty_terrain_buffer();

	// Render text to screen using freetype
	// Code based off: https://learnopengl.com/In-Practice/Text-Rendering
	void renderText(std::string text, float x, float y, float scale, glm::vec3 color, const mat3& projection_matrix, const mat3& view_matrix);

	// Do not modify this. READ ONLY!!
	bool is_terrain_mesh_loaded = false;

	void drawParticles(Entity entity ,const mat3& view_matrix, const mat3& projection);


private:
	// Freetype stuff
	typedef struct {
		unsigned int textureID;  // ID handle of the glyph texture
		glm::ivec2   size;       // Size of glyph
		glm::ivec2   bearing;    // Offset from baseline to left/top of glyph
		unsigned int advance;    // Offset to advance to next glyph
	} Character;

	std::map<char, Character> characters;

	// Define VAOs here
	GLuint global_vao, text_vao;
	
	// Internal vertex data structure used for batched rendering
	struct BatchedVertex {
		vec3 position;
		vec2 texCoords;
		uint16_t texIndex;
		uint8_t flags;		// reserved for animated textures, etc.
		uint8 frameValue;	// Gives orientation, what random texture to use, etc.
	};

	enum VERTEX_FLAGS : uint8_t {
		DIRECTIONAL = 0b1,
		RANDOM = 0b10,
	};

	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& view_matrix, const mat3& projection);
	void drawToScreen();

	/// <summary>
	/// Adds 4 vertices and 6 indices to make a quad. Does not check for overlapping vertices.
	/// </summary>
	/// <typeparam name="T">Index buffer integer type</typeparam>
	/// <param name="modelMatrix">The transform matrix of the quad</param>
	/// <param name="texture">Texture ID assigned to this quad</param>
	/// <param name="vertices">The vertex buffer</param>
	/// <param name="indicies">The index buffer</param>
	template <class T>
	void make_quad(mat3 modelMatrix, uint16_t texture_id, std::vector<BatchedVertex>& vertices, std::vector<T>& indicies,
		uint8_t flags = 0, uint8_t frameValue = 0);

	void makeQuadVertices(glm::mat3& modelMatrix, uint16_t texture_id, std::vector<RenderSystem::BatchedVertex>& vertices,
		uint8_t flags = 0, uint8_t frameValue = 0);

	/// <summary>
	/// Batch-draws the terrain layer.
	/// </summary>
	/// <param name="view_2D">The camera view matrix</param>
	/// <param name="projection_2D">The screen projection matrix</param>
	void drawTerrain(const mat3& view_2D, const mat3& projection_2D);
	// void drawFOW();
	//void load(); 
	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	// Used for terrain batch rendering. This is a 2D array of terrain textures.
	GLuint texture_array;

	Entity screen_state_entity;

	
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
