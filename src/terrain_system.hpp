#pragma once
#include <cmath>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

// The underlying terrain grid square
class TerrainSystem
{
public:
	// size of each respective axes (absolute)
	int size_x, size_y;
	
	TerrainSystem() { grid = nullptr; }

	~TerrainSystem() {
		delete[] grid;
	}

	/// <summary>
	/// Represents one cell in the grid, internal uses for now. Exposed to public
	/// in case you have some use for it.
	/// </summary>
	class Cell {
	public:
		Entity entity;

		// STRUCTURE:
		// [bits 31-2 are reserved, disable pathfind, collidable]
		//							    1st bit		    0th bit
		// 
		// We're using unsigned int flags because it is the largest data structure that
		// remains aligned with Entity
		unsigned int flags;
	};

	

	/// <summary>
	/// Initializes the world grid with the given size. Each axis should preferably be odd.
	/// </summary>
	/// <param name="x">The size of the map in the x axis (preferably odd)</param>
	/// <param name="y">The size of the map in the y axis (preferably odd)</param>
	/// <param name="renderer">The main renderer</param>
	void init(const unsigned int x, const unsigned int y, RenderSystem* renderer);

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
	/// Return true if the tile should have a collider
	/// </summary>
	bool isImpassable(Entity tile) {
		assert(registry.terrainCells.has(tile));
		return grid[tile - entityStart].flags & TERRAIN_FLAGS::COLLIDABLE; 
	}
	bool isImpassable(vec2 position) { isImpassable((int)position.x, (int)position.y); };
	bool isImpassable(int x, int y) { return grid[to_array_index(x, y)].flags & TERRAIN_FLAGS::COLLIDABLE; }

	/// <summary>
	/// Returns valid, non-collidable neighbours.
	/// </summary>
	/// <param name="cell">The origin cell.</param>
	/// <param name="buffer">A vector buffer</param>
	void get_accessible_neighbours(Entity cell, std::vector<Entity>& buffer, bool checkPathfind = false);

	/// <summary>
	/// Updates the values for a tile. This includes rendering data.
	/// </summary>
	/// <param name="cell">The tile's entity</param>
	void update_tile(Entity tile) {
		RenderRequest& r = registry.terrainRenderRequests.get(tile);
		renderer->changeTerrainData(tile, tile - entityStart, r);
	}

private:
	Cell* grid;	// PLEASE DO NOT EXPOSE THIS UNLESS YOU KNOW WHAT YOU ARE DOING
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

	/// <summary>
	/// Builds a RenderRequest component for this cell.
	/// </summary>
	RenderRequest make_render_request(TerrainCell& cell);
};