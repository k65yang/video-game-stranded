#pragma once
#include <cmath>
#include <random>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

// The underlying terrain grid square
class TerrainSystem
{
private:
	/// <summary>
	/// Corresponds to the underlying index each element in every indices[orientations_n_indices] you see in TerrainSystem
	/// </summary>
	enum ori_index : uint8_t {
		TOP, RIGHT, BOTTOM, LEFT, TR, BR, BL, TL, orientations_n_indices
	};

	inline void deallocate_terrain_grid(uint32_t*& terraincell_grid_pointer)	{ // cursed type 
		delete[] terraincell_grid_pointer;
		terraincell_grid_pointer = nullptr;
	}

	inline void deallocate_terrain_grid() {
		deallocate_terrain_grid(terraincell_grid);
	}

	inline void deallocate_entity_grid(Entity*& entity_grid_pointer, size_t x, size_t y) {

		for (size_t i = 0; i < x * y; i++) {
			registry.remove_all_components_of(entity_grid_pointer[i]);
		}
		delete[] entity_grid_pointer;
		entity_grid_pointer = nullptr;
	}

	inline void deallocate_entity_grid() {
		deallocate_entity_grid(entity_grid, size_x, size_y);
	}

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
	std::unordered_map<TERRAIN_TYPE, float> terrain_type_to_speed_ratio = {
		{TERRAIN_TYPE::AIR, 1.f},
		{TERRAIN_TYPE::GRASS, 1.f},
		{TERRAIN_TYPE::ROCK, 1.f},
		{TERRAIN_TYPE::SAND, 0.7f},
		{TERRAIN_TYPE::MUD, 0.55f},
		{TERRAIN_TYPE::SHALLOW_WATER, 0.4f},
		{TERRAIN_TYPE::DEEP_WATER, 0.25f}
	};

	// Look-up table for zone boundaries. Zones are circular.
	// Key is zone number. Value is the radius from the spaceship
	std::unordered_map<ZONE_NUMBER, int> zone_radius_map = {
		{ZONE_0, 10},
		{ZONE_1, 17},
		{ZONE_2, 40},
		{ZONE_3, 64},
		{ZONE_4, 85},
		// Make the radius be the hypoteneuse of the map to encapsulate the entire map as a circle
		{ZONE_5, ceil(sqrt((size_x * size_x / 4) + (size_y * size_y / 4)))},
		
	};

	/// @brief Function to get randomized spawn locations per zone
	/// @param num_per_zone Map specifying how many mobs to spawn per zone
	/// @return List of spawn locations
	std::vector<vec2> get_mob_spawn_locations(std::map<ZONE_NUMBER,int> num_per_zone);

	/// <summary>
	/// Initializes the world grid with the given size. Each axis should preferably be odd.
	/// </summary>
	/// <param name="x">The size of the map in the x axis</param>
	/// <param name="y">The size of the map in the y axis</param>
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
	/// Rounds a given float into the nearest integer.
	/// </summary>
	/// <param name="x">The given float</param>
	/// <returns>The nearest integer</returns>
	inline int quantize_f(float x) {
		return std::round(x);
	}

	/// <summary>
	/// Rounds a given vec2 into the nearest integers.
	/// </summary>
	/// <param name="position">Any given vec2</param>
	/// <returns>An ivec2 containing two rounded integers.</returns>
	inline ivec2 quantize_vec2(vec2 position) {
		return glm::round(position);
	}

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

	/// @brief Get a valid random terrain location anywhere on the map that is not used
	/// @param @overload zone: The zone to get the random terrain location.
	/// @return A vec2 of the random position
	vec2 get_random_terrain_location();
	vec2 get_random_terrain_location(ZONE_NUMBER zone);

	bool is_terrain_location_used(vec2 position);

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
		if (abs(x) > size_x / 2 || abs(x) > size_y / 2)
			return true;
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

	/// <summary>
	/// Checks each orientations_n_indices indices of cell_index's adjacent tiles. Sets them to -1 if they are out of range.
	/// </summary>
	/// <param name="cell_index">The index of the middle tile</param>
	/// <param name="indices">The indices of all orientations_n_indices adjacent tiles. See ori_index for order.</param>
	inline void filter_neighbouring_indices(int cell_index, int indices[orientations_n_indices]);

	/// <summary>
	/// Generates a map of type {cell_index, ORIENTATIONS} where:
	///		The key is the index (for terraincell_grid) of a tile with directional terrain type.
	///		The value is its tile orientation.
	/// </summary>
	/// <param name="map">Map to write to</param>
	void generate_orientation_map(std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS>& map);

	/// <summary>
	/// Updates the values for a tile. This includes rendering data and TerrainCell data.
	/// </summary>
	/// <param name="tile">The tile's entity</param>
	/// <param name="also_update_neighbours">Set to True if this tile's neighbours should also be updated</param>
	void update_tile(Entity tile, bool also_update_neighbours = false) {
		TerrainCell& cell = registry.terrainCells.get(tile);
		return update_tile(tile, cell, also_update_neighbours);
	}

	/// <summary>
	/// Updates the values for a tile. This includes rendering data and TerrainCell data.
	/// </summary>
	/// <param name="tile">The tile's entity</param>
	/// <param name="cell">The tile's TerrainCell component</param>
	/// <param name="also_update_neighbours">Set to True if this tile's neighbours should also be updated</param>
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
				int indices[orientations_n_indices] = {
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

	/// <summary>
	/// Deletes the old map and generates one anew with size x, y.
	/// Already-defined map data is inserted into the new map.
	/// </summary>
	/// <param name="x">The size of the map in the x axis</param>
	/// <param name="y">The size of the map in the y axis</param>
	void expand_map(const int new_x, const int new_y);

	/// @brief Reset the terrain system when the game resets
	void resetTerrainSystem() {
		used_terrain_locations.clear();
	}

	/// <summary>
	/// Checks every tile and sets their flags appropriately. Useful after messing with the map editor.
	/// </summary>
	void clean_map_tiles() {
		for (unsigned i = 0; i < size_x * size_y; i++) {
			uint32_t& cell = terraincell_grid[i];
			TERRAIN_TYPE type = static_cast<TERRAIN_TYPE>(cell >> 16);
			if (type == ROCK || type == SHALLOW_WATER || type == DEEP_WATER)
				cell &= ~(ALLOW_SPAWNS);
			if (type == ROCK)
				cell &= ~(DISABLE_PATHFIND);
			if (type == GRASS || type == MUD || type == SAND)
				cell |= ALLOW_SPAWNS;
			if (type == GRASS || type == MUD || type == SAND || type == SHALLOW_WATER || type == DEEP_WATER) {
				cell &= ~(COLLIDABLE);
			}
		}
	}

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
	/// Returns the index used for 'grid' with the given x and y world coordinates
	/// </summary>
	/// <param name="x">The x position of the cell</param>
	/// <param name="y">The y position of the cell</param>
	/// <returns>An index for 'grid' corresponding to that position</returns>
	unsigned int to_array_index(ivec2 position);

	/// <summary>
	/// Converts a cell's index into world coordinates.
	/// </summary>
	/// <param name="index">The index to a cell in 'grid'</param>
	/// <returns>The cell's world position</returns>
	vec2 to_world_coordinates(const int index);

	/// <summary>
	/// Checks if the given terrain type and terrain_cell[index] has the same terrain type
	/// </summary>
	bool matches_terrain_type(uint16_t current, int index);

	/// <summary>
	/// Checks that 2 given lisst of ori_index-indexed tiles either has matching terrain or non-matching terrain.
	/// </summary>
	/// <param name="current">The terrain type of the queried cell</param>
	/// <param name="indices">The indices of all 8 adjacent tiles. See ori_index for order.</param>
	/// <param name="match_list">List of ori_index neighbouring tiles that must be the same type</param>
	/// <param name="reject_list">List of ori_index neighbouring tiles that must NOT match</param>
	/// <returns>True if every neighbour in match_list matches AND every neighbour in reject_list do not match</returns>
	bool match_adjacent_terrain(uint16_t current, int indices[orientations_n_indices], std::initializer_list<uint8_t> match_list, std::initializer_list<uint8_t> reject_list = {});

	/// <summary>
	/// Assuming that the given tile is a directional type, returns its appropriate orientation.
	/// </summary>
	/// <param name="centre_index">The index of the queried tile.</param>
	/// <returns>The tile's orientation as RenderSystem::ORIENTATIONS</returns>
	RenderSystem::ORIENTATIONS find_tile_orientation(int centre_index);

	/// <summary>
	/// Assuming that the given tile is a directional type, returns its appropriate orientation.
	/// </summary>
	/// <param name="current">The terrain type of the queried cell</param>
	/// <param name="indices">The indices of all 8 adjacent tiles. See ori_index for order.</param>
	/// <returns>The tile's orientation as RenderSystem::ORIENTATIONS</returns>
	RenderSystem::ORIENTATIONS find_tile_orientation(uint16_t current, int indices[orientations_n_indices]);

	// Map to keep track of locations where an item/mob has been spawned
	std::unordered_set<vec2> used_terrain_locations;
};