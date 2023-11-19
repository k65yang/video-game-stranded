#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
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
			//textures_path("player.png"),
			textures_path("player_spritesheet.png"),
			textures_path("mob_spritesheet.png"),
			textures_path("redblock.png"),
			textures_path("fow mask.png"),
			textures_path("item.png"),
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
			textures_path("spaceship.png"), 
			textures_path("blueblock.png"),
			textures_path("help1.png"),
			textures_path("help2.png"),
			textures_path("help3.png"),
			textures_path("help4.png"),
			textures_path("help_weapon.png"),
			textures_path("q1_not_found.png"),
			textures_path("q1_found.png"),
			textures_path("q2_not_found.png"),
			textures_path("q2_found.png"),
			textures_path("q1.png"),
			textures_path("q2.png"),
			textures_path("ghost.png"),
			textures_path("redblock.png"),	// TODO: find texture for brute mob type
	};

	// This is used for generating the texture array for batched renders
	// NOTE: all images must have the same dimensions.
	const std::array<std::string, TERRAIN_TYPE::TERRAIN_COUNT> terrain_texture_paths = {
		terrain_texture_path("0.png"),
		terrain_texture_path("grass_3.png"),
		terrain_texture_path("stone_3.png"),
		terrain_texture_path("sand_3.png"),
		terrain_texture_path("mud_2.png"),
		terrain_texture_path("shallow_water_1.png"),
		terrain_texture_path("deep_water_1.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("pebble"),
		shader_path("spritesheet"), 
		shader_path("salmon"),
		shader_path("textured"),
		shader_path("water"),
		shader_path("terrain"),};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

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
	void initializeTerrainBuffers();

	void initializeGlTextures();

	/// <summary>
	/// Generates 2D texture arrays for batch rendering.
	/// </summary>
	void initializeGl3DTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createModelMatrix(Entity entity);
	mat3 createProjectionMatrix();

	/// <summary>
	/// Modifies the terrain vertex buffer to regenerate rendering values for a specific tile.
	/// </summary>
	/// <param name="cell">The tile</param>
	/// <param name="i">The tile's index in the Cell array</param>
	/// <param name="data">The updated render request</param>
	void changeTerrainData(Entity cell, unsigned int i, TerrainCell& data);

	// Do not modify this. READ ONLY!!
	bool is_terrain_mesh_loaded = false;

private:
	// Internal vertex data structure used for batched rendering
	struct BatchedVertex {
		vec3 position;
		vec2 texCoords;
		uint16_t texIndex;
		uint16_t flags;	// reserved for animated textures, etc.
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
	void make_quad(mat3 modelMatrix, uint16_t texture_id, std::vector<BatchedVertex>& vertices, std::vector<T>& indicies);

	void makeQuadVertices(glm::mat3& modelMatrix, uint16_t texture_id, std::vector<RenderSystem::BatchedVertex>& vertices);

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
