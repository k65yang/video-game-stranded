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
using namespace glm;

#include "tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"

const std::string map_ext = "smap";
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
const int window_width_px = 1200;
const int window_height_px = 800;
const int tile_size_px = 50;		// Represents how many pixels a tile occupies in a row or column

const unsigned short world_size_x = 13;
const unsigned short world_size_y = 13;

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
void write_to_file(std::ofstream& file, T& data, uint32_t num = 1);

template<typename T>
inline void write_to_file(std::ofstream& file, T& data, uint32_t num)
{
	file.write((char*)&data, sizeof(T) * num);
}
