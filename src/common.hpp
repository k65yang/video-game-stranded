#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3

// glm hashing functions
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

using namespace glm;

#include "tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"

const std::string map_ext = "smap";
const std::string loaded_map_name = "test";
const unsigned int savefile_version = 1;

inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string terrain_texture_path(const std::string& name) { return data_path() + "/textures/terrain/" + std::string(name);};

inline std::string map_path_builder(const std::string& name) {
	return data_path() + "/maps/" + std::string(name) + '.' + std::string(map_ext);
};

#define SMAP_PADDING_BYTES 32

static int window_width_px;
static int window_height_px;
// 
// Actual screen resolution stores iniside render_system as window_resolution
const ivec2 target_resolution = { 1200, 800 };	// alternative: { 1620, 1080 }
const ivec2 aspect_ratio = { 3, 2 };			// Remember to reduce this to lowest form!
const bool windowed_mode = true;

const int tile_size_px = 50;		// Represents how many pixels a tile occupies in a row and column

const unsigned short world_size_x = 64;
const unsigned short world_size_y = 64;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef TWO_PI
#define TWO_PI 2 * M_PI
#endif	TWO_PI

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void scale(int scale);
	void scale(float scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();

template <typename T>
void write_to_file(std::ofstream& file, T& data);

template <typename T>
void read_from_file(std::ofstream& file, T& data);


template<typename T>
inline void write_to_file(std::ofstream& file, T& data)
{
	file.write((char*)&data, sizeof(T));
}

template<typename T>
inline void read_from_file(std::ifstream& file, T& data)
{
	file.read((char*)&data, sizeof(T));
}

