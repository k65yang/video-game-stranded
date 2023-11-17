// internal
#include "render_system.hpp"

#include <array>
#include <fstream>

#include "../ext/stb_image/stb_image.h"

// This creates circular header inclusion, that is quite bad.
#include "tiny_ecs_registry.hpp"

// stlib
#include <iostream>
#include <sstream>

// World initialization
bool RenderSystem::init(GLFWwindow* window_arg)
{
	this->window = window_arg;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	if (frame_buffer_width_px != window_width_px)
	{
		printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
		printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
		printf("window width_height = %d,%d\n", window_width_px, window_height_px);
	}

	// Hint: Ask your TA for how to setup pretty OpenGL error callbacks. 
	// This can not be done in mac os, so do not enable
	// it unless you are on Linux or Windows. You will need to change the window creation
	// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	gl_has_errors();

	initScreenTexture();
    initializeGlTextures();
	initializeGl3DTextures();
	initializeGlEffects();
	initializeGlGeometryBuffers();

	return true;
}

/// <summary>
/// Adds a new quad and places it in the vertex and index buffer.
/// Used for batch rendering.
/// </summary>
/// <param name="modelMatrix"></param>
/// <param name="texture"></param>
/// <param name="vertices"></param>
/// <param name="indicies"></param>
template <class T>
void RenderSystem::make_quad(mat3 modelMatrix, uint16_t texture_id, std::vector<BatchedVertex>& vertices, 
	std::vector<T>& indicies, uint8_t flags, uint8_t frameValue) {

	makeQuadVertices(modelMatrix, texture_id, vertices, flags, frameValue);

	int i = vertices.size() / 4 - 1;
	for (uint x : { 0, 3, 1, 1, 3, 2 }) {
		indicies.push_back(i * 4 + x);
	}
}

void RenderSystem::makeQuadVertices(glm::mat3& modelMatrix, uint16_t texture_id, std::vector<BatchedVertex>& vertices, 
	uint8_t flags, uint8_t frameValue)
{
	BatchedVertex quad[4];

	// TODO: properly handle GL_LINEAR behaviour in texture atlasses instead of this hacky method.
	// Basically, we're definining the "edge" texels/pixels of a tile
	// as a "border" for GL_LINEAR filtering to use
	quad[0].position = { -0.5f, 0.5f, 1.f };
	quad[0].texCoords = { terrain_texel_offset.x, terrain_sheet_uv.y - terrain_texel_offset.y };

	quad[1].position = { 0.5f, 0.5f, 1.f };
	quad[1].texCoords = { terrain_sheet_uv.x - terrain_texel_offset.x, terrain_sheet_uv.y - terrain_texel_offset.y };

	quad[2].position = { 0.5f, -0.5f, 1.f };
	quad[2].texCoords = { terrain_sheet_uv.x - terrain_texel_offset.x, terrain_texel_offset.y };

	quad[3].position = { -0.5f, -0.5f, 1.f };
	quad[3].texCoords = { terrain_texel_offset.x , terrain_texel_offset.y };

	if (flags & DIRECTIONAL) {
		ORIENTATIONS ori = static_cast<ORIENTATIONS>(frameValue);	// in the blind forest
		vec2 index = terrain_atlas_offsets.at(ori);
		vec2 uv_zero = quad[3].texCoords + index * terrain_sheet_uv;
		vec2 uv_one = quad[1].texCoords + index * terrain_sheet_uv;
		if (mirror_horizontal_orientations.count(ori))
			std::swap(uv_zero.y, uv_one.y);
		if (mirror_vertical_orientations.count(ori))
			std::swap(uv_zero.x, uv_one.x);
		if (rotate_orientations.count(ori)) {

			quad[1].texCoords = { uv_zero.x, uv_one.y };
			quad[2].texCoords = uv_one;
			quad[3].texCoords = { uv_one.x, uv_zero.y };
			quad[0].texCoords = uv_zero;
		}
		else {
			quad[0].texCoords = { uv_zero.x, uv_one.y };
			quad[1].texCoords = uv_one;
			quad[2].texCoords = { uv_one.x, uv_zero.y };
			quad[3].texCoords = uv_zero;
		}
	}

	for (BatchedVertex& v : quad) {
		v.position = modelMatrix * v.position;
		v.texIndex = texture_id;
		v.flags = flags;
		v.frameValue = frameValue;
		vertices.push_back(v);
	}
}

void RenderSystem::initializeTerrainBuffers(std::unordered_map<unsigned int, ORIENTATIONS>& directional_tex_orientations)
{
	std::vector<BatchedVertex> vertices;
	// We need 32-bit indices because 16-bit indices limits us to 
	// sqrt[2^16 / (number of indices per quad = 6)] = ~104 x 104 grid
	// The new maximum should be ~26,754 x 26,754.
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i < registry.terrainCells.entities.size(); i++) {
		Entity e = registry.terrainCells.entities[i];
		TerrainCell& cell = registry.terrainCells.components[i];
		uint8_t flags = directional_terrain.count(cell.terrain_type) ? DIRECTIONAL : 0;
		bool has_orientation = flags & DIRECTIONAL;
		uint8_t frameValue = has_orientation ? directional_tex_orientations[i] : 0;
		mat3 modelMatrix = createModelMatrix(e);	// preprocess transform matrices because
													// we can't really have per-mesh transforms so let's just bake them in!

		make_quad(modelMatrix, cell.terrain_type, vertices, indices, flags, frameValue);
	}

	bindVBOandIBO(GEOMETRY_BUFFER_ID::TERRAIN, vertices, indices);
	is_terrain_mesh_loaded = true;
}

void RenderSystem::initializeGlTextures()
{
    glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

    for(uint i = 0; i < texture_paths.size(); i++)
    {
		const std::string& path = texture_paths[i];
		ivec2& dimensions = texture_dimensions[i];

		stbi_uc* data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_has_errors();
		stbi_image_free(data);
    }
	gl_has_errors();
}

// Holy hell thank you to these:
// https://www.khronos.org/opengl/wiki/Array_Texture
// https://stackoverflow.com/questions/72390151/how-to-use-a-2d-texture-array-in-opengl
void RenderSystem::initializeGl3DTextures() {
	glGenTextures(1, &texture_array);
	gl_has_errors();
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);
	gl_has_errors();

	uint n = terrain_texture_paths.size();
	if (n == 0) {
		std::cout << "There are no terrain textures loaded! Please fill in 'terrain_texture_paths'." << std::endl;
		assert(n > 0);	// no textures!!
	}

	// Since we don't know how big our textures are going to be, we have to
	// read the first texture.
	ivec2 dimensions;
	stbi_uc* data;
	data = stbi_load(terrain_texture_paths[0].c_str(), &dimensions.x, &dimensions.y, NULL, 4);
	
	// Can't load the first texture
	if (data == NULL)
	{
		const std::string message = "Could not load the file " + terrain_texture_paths[0] + ".";
		fprintf(stderr, "%s", message.c_str());
		assert(false);
	}

	// Tell GPU to allocate the 3d texture
	// We are now officially limited to OpenGL 4.2+ LMAO
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, dimensions.x, dimensions.y, n);
	gl_has_errors();


	// Put the actual texture data at the 0th depth

	// The numbers Mason, what do they mean?:
	// The 4 zeroes: 
	// The first zero: the "layer" of this element (complicated stuff I don't want to get into)
	// The next two zeros: The x,y offset inside the uv space where the data is placed
	// The zero after: Specifies that this texture is the i-th element in the array	
	// 
	// The reason why we're using glTexSubImage3D is because we've already "allocated" the space for the texture.
	// We're just modifying that space, hence the 'Sub' in 'SubImage'.
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, dimensions.x, dimensions.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
	gl_has_errors();

	// Free the data pointer since it allocated to heap.
	// It will be reassigned in the loop.
	stbi_image_free(data);
	data = nullptr;

	for (uint i = 1; i < n; i++)
	{
		const std::string& path = terrain_texture_paths[i];
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}

		// Assign the i-th texture
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, dimensions.x, dimensions.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		gl_has_errors();
		stbi_image_free(data);
		data = nullptr;
	}
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl_has_errors();
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{
	for(uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
		const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}

// One could merge the following two functions as a template function...
template <class T, class U>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<U> indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
	gl_has_errors();
}

void RenderSystem::initializeGlMeshes()
{
	for (uint i = 0; i < mesh_paths.size(); i++)
	{
		// Initialize meshes
		GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
		std::string name = mesh_paths[i].second;
		Mesh::loadFromOBJFile(name, 
			meshes[(int)geom_index].vertices,
			meshes[(int)geom_index].vertex_indices,
			meshes[(int)geom_index].original_size);

		bindVBOandIBO(geom_index,
			meshes[(int)geom_index].vertices, 
			meshes[(int)geom_index].vertex_indices);

		// adjustment for player mesh vertices (invert y and some offset)
		if (geom_index == GEOMETRY_BUFFER_ID::PLAYER_MESH) {
			
			for (int j = 0; j < meshes[(int)geom_index].vertices.size(); j++)
			{
				meshes[(int)geom_index].vertices[j].position.y = (meshes[(int)geom_index].vertices[j].position.y * -1.f) - 0.5f;
			}
		}

		// adjustment for mob mesh
		if (geom_index == GEOMETRY_BUFFER_ID::MOB001_MESH) {

			for (int j = 0; j < meshes[(int)geom_index].vertices.size(); j++)
			{
				meshes[(int)geom_index].vertices[j].position.y = (meshes[(int)geom_index].vertices[j].position.y * -1.f);
			}
		}

	}
}

void RenderSystem::initializeGlGeometryBuffers()
{
	// Vertex Buffer creation.
	glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	// Index Buffer creation.
	glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

	// Index and Vertex buffer data initialization.
	initializeGlMeshes();

	//////////////////////////
	// Initialize sprite
	// The position corresponds to the center of the texture.
	std::vector<TexturedVertex> textured_vertices(4);
	textured_vertices[0].position = { -1.f/2, +1.f/2, 0.f };
	textured_vertices[1].position = { +1.f/2, +1.f/2, 0.f };
	textured_vertices[2].position = { +1.f/2, -1.f/2, 0.f };
	textured_vertices[3].position = { -1.f/2, -1.f/2, 0.f };
	textured_vertices[0].texcoord = { 0.f, 1.f };
	textured_vertices[1].texcoord = { 1.f, 1.f };
	textured_vertices[2].texcoord = { 1.f, 0.f };
	textured_vertices[3].texcoord = { 0.f, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> textured_indices = { 0, 3, 1, 1, 3, 2 };
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);

	////////////////////////
	// Initialize pebble
	std::vector<ColoredVertex> pebble_vertices;
	std::vector<uint16_t> pebble_indices;
	constexpr float z = -0.1f;
	constexpr int NUM_TRIANGLES = 62;

	for (int i = 0; i < NUM_TRIANGLES; i++) {
		const float t = float(i) * M_PI * 2.f / float(NUM_TRIANGLES - 1);
		pebble_vertices.push_back({});
		pebble_vertices.back().position = { 0.1 * cos(t), 0.1 * sin(t), z };
		pebble_vertices.back().color = { 0.8, 0.8, 0.8 };
	}
	pebble_vertices.push_back({});
	pebble_vertices.back().position = { 0, 0, 0 };
	pebble_vertices.back().color = { 0.8, 0.8, 0.8 };
	for (int i = 0; i < NUM_TRIANGLES; i++) {
		pebble_indices.push_back((uint16_t)i);
		pebble_indices.push_back((uint16_t)((i + 1) % NUM_TRIANGLES));
		pebble_indices.push_back((uint16_t)NUM_TRIANGLES);
	}
	int geom_index = (int)GEOMETRY_BUFFER_ID::PEBBLE;
	meshes[geom_index].vertices = pebble_vertices;
	meshes[geom_index].vertex_indices = pebble_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::PEBBLE, meshes[geom_index].vertices, meshes[geom_index].vertex_indices);

	//////////////////////////////////
	// Initialize debug line
	std::vector<ColoredVertex> line_vertices;
	std::vector<uint16_t> line_indices;

	constexpr float depth = 0.5f;
	constexpr vec3 red = { 0.8,0.1,0.1 };

	// Corner points
	line_vertices = {
		{{-0.5,-0.5, depth}, red},
		{{-0.5, 0.5, depth}, red},
		{{ 0.5, 0.5, depth}, red},
		{{ 0.5,-0.5, depth}, red},
	};

	// Two triangles
	line_indices = {0, 1, 3, 1, 2, 3};
	
	geom_index = (int)GEOMETRY_BUFFER_ID::DEBUG_LINE;
	meshes[geom_index].vertices = line_vertices;
	meshes[geom_index].vertex_indices = line_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::DEBUG_LINE, line_vertices, line_indices);

	///////////////////////////////////////////////////////
	// Initialize screen triangle (yes, triangle, not quad; its more efficient).
	std::vector<vec3> screen_vertices(3);
	screen_vertices[0] = { -1, -6, 0.f };
	screen_vertices[1] = { 6, -1, 0.f };
	screen_vertices[2] = { -1, 6, 0.f };

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> screen_indices = { 0, 1, 2 };
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);


	//////////////////////////
// Initialize player sprite
// The position corresponds to the center of the texture.
	float player_frame_w = 0.125f; 
	float player_frame_h = 0.2f;
	std::vector<TexturedVertex> player_vertices(4);
	player_vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
	player_vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
	player_vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
	player_vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };
	player_vertices[3].texcoord = { 0.f, 0.f };
	player_vertices[2].texcoord = { player_frame_w, 0.f };
	player_vertices[1].texcoord = { player_frame_w, player_frame_h};
	player_vertices[0].texcoord = { 0.f, player_frame_h};

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> player_indices = { 0, 3, 1, 1, 3, 2 };

	bindVBOandIBO(GEOMETRY_BUFFER_ID::PLAYER_SPRITE, player_vertices, player_indices);

	// Initialize mob sprite
// The position corresponds to the center of the texture.
	float mob_frame_w = 0.142857f;
	float mob_frame_h = 0.25;
	std::vector<TexturedVertex> mob_vertices(4);
	mob_vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
	mob_vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
	mob_vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
	mob_vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };
	mob_vertices[3].texcoord = { 0.f, 0.f };
	mob_vertices[2].texcoord = { mob_frame_w, 0.f };
	mob_vertices[1].texcoord = { mob_frame_w, mob_frame_h };
	mob_vertices[0].texcoord = { 0.f, mob_frame_h };

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> mob_indices = { 0, 3, 1, 1, 3, 2 };

	bindVBOandIBO(GEOMETRY_BUFFER_ID::MOB_SPRITE, mob_vertices, mob_indices);

}

RenderSystem::~RenderSystem()
{
	// Don't need to free gl resources since they last for as long as the program,
	// but it's polite to clean after yourself.
	glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &off_screen_render_buffer_color);
	glDeleteTextures(1, &texture_array);
	glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	gl_has_errors();

	for(uint i = 0; i < effect_count; i++) {
		glDeleteProgram(effects[i]);
	}
	// delete allocated resources
	glDeleteFramebuffers(1, &frame_buffer);
	gl_has_errors();

	// remove all entities created by the render system
	while (registry.renderRequests.entities.size() > 0)
	    registry.remove_all_components_of(registry.renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{
	registry.screenStates.emplace(screen_state_entity);

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &framebuffer_width, &framebuffer_height);  // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	glGenTextures(1, &off_screen_render_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	gl_has_errors();

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char* vs_src = vs_str.c_str();
	const char* fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	glAttachShader(out_program, vertex);
	glAttachShader(out_program, fragment);
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	glDetachShader(out_program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}

