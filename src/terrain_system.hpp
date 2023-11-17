#pragma once
#include <cmath>
#include <random>
#include <map>
#include <unordered_map>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

// The underlying terrain grid square
class TerrainSystem
{
private:
	const enum ori_index : uint8_t {
		TOP, RIGHT, BOTTOM, LEFT, TR, BR, BL, TL, n
	};

public:
	// size of each respective axes (absolute)
	int size_x, size_y;
	
	TerrainSystem() { entity_grid = nullptr; terraincell_grid = nullptr; }

	~TerrainSystem() {
		registry.terrainCells.clear();
		delete[] terraincell_grid;

		for (uint i = 0; i < size_x * size_y; i++) {
			registry.remove_all_components_of(entity_grid[i]);
		}

		delete[] entity_grid;
	}	

	// Look-up table for terrain type slow ratios
	std::map<TERRAIN_TYPE, float> terrain_type_to_speed_ratio = {
		{TERRAIN_TYPE::AIR, 1.f},
		{TERRAIN_TYPE::GRASS, 1.f},
		{TERRAIN_TYPE::ROCK, 1.f},
		{TERRAIN_TYPE::SAND, 0.5f},
		{TERRAIN_TYPE::MUD, 0.3f},
		{TERRAIN_TYPE::SHALLOW_WATER, 0.25f},
		{TERRAIN_TYPE::DEEP_WATER, 0.0625f}
	};

	/// <summary>
	/// Initializes the world grid with the given size. Each axis should preferably be odd.
	/// </summary>
	/// <param name="x">The size of the map in the x axis (preferably odd)</param>
	/// <param name="y">The size of the map in the y axis (preferably odd)</param>
	/// <param name="renderer">The main renderer</param>
	void init(const unsigned int x, const unsigned int y, RenderSystem* renderer);

	/// <summary>
	/// Initializes the world grid with the given a map name. Map must exist.
	/// </summary>
	/// <param name="map_name">The name of the map to be read in PROJECT_SOURCE_DIR/data/maps/</param>
	/// <param name="renderer">The main renderer</param>
	void init(const std::string& map_name, RenderSystem* renderer);

	/// <summary>
	/// Used for cells that may have ticking logic. futureproofing.
	/// </summary>
	/// <param name="delta_time">The time since the last frame in milliseconds</param>
	void step(float delta_time);

	/// <summary>
	/// Returns the entity associated with the cell at the given position
	/// </summary>
	/// <param name="position">The position of the cell</param>
	/// <returns>The terrain cell's entity</returns>
	Entity get_cell(vec2 position);

	/// <summary>
	/// Returns the entity associated with the cell at the given position
	/// </summary>
	/// <param name="x">The x position of the cell</param>
	/// <param name="y">The y position of the cell</param>
	/// <returns>The terrain cell's entity</returns>
	Entity get_cell(int x, int y);

	/// <summary>
	/// Returns the entity associated with the cell at the given index
	/// </summary>
	/// <param name="index">The index of the cell in the world grid</param>
	/// <returns>The terrain cell's entity</returns>
	Entity get_cell(int index);

	/// <summary>
	/// Returns the index in the grid for the given cell
	/// </summary>
	/// <param name="cell">The cell</param>
	/// <returns>The index of the cell in the grid</returns>
	int get_cell_index(Entity cell);

	/// <summary>
	/// Gets the speed ratio of the terrain of a cell
	/// </summary>
	/// <param name="cell">The cell entity</param>
    /// <returns>Returns the terrain speed ratio of the cell</returns>
    float get_terrain_speed_ratio(Entity cell);

	/// <summary>
	/// Return true if the tile should have a collider
	/// </summary>
	bool is_impassable(Entity tile) {
		assert(registry.terrainCells.has(tile));
		return terraincell_grid[tile - entityStart] & TERRAIN_FLAGS::COLLIDABLE; 
	}
	bool is_impassable(vec2 position) { return is_impassable((int)std::round(position.x), (int)std::round(position.y)); };
	bool is_impassable(int x, int y) { return terraincell_grid[to_array_index(x, y)] & TERRAIN_FLAGS::COLLIDABLE; }

	/// <summary>
	/// Returns true if the tile should not be spawnable to items or mobs
	/// </summary>
	/// <param name="tile">The tile entity</param>
	bool is_invalid_spawn(Entity tile) {
		assert(registry.terrainCells.has(tile));
		uint32_t flag = terraincell_grid[tile - entityStart] & (COLLIDABLE | ALLOW_SPAWNS);
		if (flag & TERRAIN_FLAGS::COLLIDABLE)
			return true;
		return flag ^ TERRAIN_FLAGS::ALLOW_SPAWNS;
	}

	/// <summary>
	/// Returns true if the tile should not be spawnable to items or mobs
	/// </summary>
	/// <param name="x">An x position in world space</param>
	/// <param name="y">An y position in world space</param>
	bool is_invalid_spawn(int x, int y) {
		uint32_t flag = terraincell_grid[to_array_index(x, y)] & (COLLIDABLE | ALLOW_SPAWNS);
		if (flag & TERRAIN_FLAGS::COLLIDABLE)
			return true;
		return flag ^ TERRAIN_FLAGS::ALLOW_SPAWNS;
	}

	/// <summary>
	/// Returns true if the tile should not be spawnable to items or mobs
	/// </summary>
	/// <param name="position">A position</param>
	/// <returns></returns>
	bool is_invalid_spawn(vec2 position) { return is_invalid_spawn((int)std::round(position.x), (int)std::round(position.y)); }


	/// <summary>
	/// Returns valid, non-collidable neighbours.
	/// </summary>
	/// <param name="cell">The origin cell.</param>
	/// <param name="buffer">A vector buffer</param>
	void get_accessible_neighbours(Entity cell, std::vector<Entity>& buffer, bool ignoreColliders, bool checkPathfind = false);

	inline void filter_neighbouring_indices(int cell_index, int indices[8]);

	/// <summary>
	/// Updates the values for a tile. This includes rendering data and TerrainCell data.
	/// </summary>
	/// <param name="tile">The tile's entity</param>
	void update_tile(Entity tile, bool also_update_neighbours = false) {
		TerrainCell& cell = registry.terrainCells.get(tile);
		return update_tile(tile, cell, also_update_neighbours);
	}

	/// <summary>
	/// Updates the values for a tile. This includes rendering data and TerrainCell data.
	/// </summary>
	/// <param name="tile">The tile's entity</param>
	/// <param name="cell">The tile's TerrainCell component</param>
	void update_tile(Entity tile, TerrainCell& cell, bool also_update_neighbours = false) {
		int i = tile - entityStart;		
		terraincell_grid[i] = cell;
		uint8_t frame_value = 0;

		// Evaluate a direction if the terrain type is directional.
		if (directional_terrain.count(cell.terrain_type)) {
			frame_value = find_tile_orientation(i);
		}

		// We may have tiles changed during world_system.init() at startup so we need to check!
		if (renderer->is_terrain_mesh_loaded) {
			renderer->changeTerrainData(tile, tile - entityStart, cell, frame_value);
			if (also_update_neighbours) {
				// We also need to update the adjacent cells
				int indices[8] = {
				i - size_x,		// Top cell
				i + 1,			// Right cell
				i + size_x,		// Bottom cell
				i - 1,			// Left cell
				i - size_x + 1,	// Top-right cell
				i + size_x + 1,	// Bottom-right cell
				i + size_x - 1,	// Bottom-left cell
				i - size_x - 1,	// Top-left cell
				};
				filter_neighbouring_indices(i, indices);

				for (int j : indices) {
					if (j < 0)
						continue;
					update_tile(entity_grid[j]);
				}
			}
		}
	}

	/// <summary>
	/// Saves the current contents of terraincell_grid into a .smap file.
	/// </summary>
	/// <param name="name">The name of the map</param>
	void save_grid(const std::string& name);

	/// <summary>
	/// Loads the named map from PROJECT_SOURCE_DIR/data/maps.
	/// </summary>
	/// <param name="name">The name of the map to be loaded</param>
	void load_grid(const std::string& name);

private:
	// PLEASE DO NOT EXPOSE THESE UNLESS YOU KNOW WHAT YOU ARE DOING

	// Massive bag of entities that will hold every tile.
	Entity* entity_grid;

	// Compressed data of every TerrainCell instance. Used for backend calculations.
	uint32_t* terraincell_grid;

	RenderSystem* renderer;

	// The id of the first entity made in this iteration
	// IMPORTANT: this assumes no other entities can be made
	// between the call to init() and when init() goes out of scope!!
	unsigned int entityStart;

	/// <summary>
	/// Returns the index used for 'grid' with the given x and y world coordinates
	/// </summary>
	/// <param name="x">The x position of the cell</param>
	/// <param name="y">The y position of the cell</param>
	/// <returns>An index for 'grid' corresponding to that position</returns>
	unsigned int to_array_index(int x, int y);

	/// <summary>
	/// Converts a cell's index into world coordinates.
	/// </summary>
	/// <param name="index">The index to a cell in 'grid'</param>
	/// <returns>The cell's world position</returns>
	vec2 to_world_coordinates(const int index);

	bool matches(uint16_t current, int index);

	bool check_match(uint16_t current, int indices[8], std::initializer_list<uint8_t> match_list, std::initializer_list<uint8_t> reject_list = {});

	RenderSystem::ORIENTATIONS find_tile_orientation(int centre_index);

	RenderSystem::ORIENTATIONS find_tile_orientation(uint16_t current, int indices[8]);

	void generate_orientation_map(std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS>& map);
};