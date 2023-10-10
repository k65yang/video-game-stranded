#include "terrain_system.hpp"

void TerrainSystem::step(float delta_time)
{
}

Entity TerrainSystem::get_cell(vec2 position)
{
	return get_cell((int)position.y, (int)position.x);
}

Entity TerrainSystem::get_cell(int x, int y)
{
	assert(grid != nullptr);
	assert(abs(x) <= size_x / 2);
	assert(abs(y) <= size_y / 2);
	return grid[to_array_index(x, y)].entity;
}

void TerrainSystem::get_accessible_neighbours(Entity cell, std::vector<Entity>& buffer, bool checkPathfind)
{
	assert(grid != nullptr);
	assert(registry.terrainCells.has(cell));
	int cell_index = cell - entityStart;
	unsigned int filter = TERRAIN_FLAGS::COLLIDABLE;

	if (checkPathfind)
		filter |= TERRAIN_FLAGS::DISABLE_PATHFIND;

	int indices[4] = { 
		cell_index - 1,			// Left cell
		cell_index + 1,			// Right cell
		cell_index + size_x,	// Bottom cell
		cell_index - size_x		// Top cell
	};

	for (int i : indices) {
		// check bounds
		if (i > 0 && i < size_x * size_y) {
			Cell& cell = grid[i];
			if (!cell.flags & filter) {	// YES WE WANT BITWISE shut up shut up shut up
				// If a cell is not collidable and pathfinding is not disabled, add to buffer
				buffer.push_back(cell.entity);
			}
		}
	}
}

unsigned int TerrainSystem::to_array_index(int x, int y)
{
	// Honestly have no idea why it's inverted...
	x = size_x / 2 + x;
	y = size_y / 2 + y;
	return (x * size_y + y);
}

vec2 TerrainSystem::to_world_coordinates(const int index)
{
	return {(index % size_x) - size_x / 2,
			index / size_x - size_y / 2 };
}

RenderRequest TerrainSystem::make_render_request(TerrainCell& cell) {
	return {
		TEXTURE_ASSET_ID::TERRAIN_GRASS,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER_ID::LAYER_1,
	};
}


void TerrainSystem::init(const unsigned int x, const unsigned int y, const RenderSystem* renderer)
{
	if (grid != nullptr) {		// if grid is allocated, deallocate
		delete[] grid;
		grid = nullptr;
	}

	size_x = x;
	size_y = y;
	grid = new Cell[x * y]();	// 1D 2-dimensional array so we can guarantee that
								// the entire world is in the same memory block.
	entityStart = grid[0].entity;

	for (int i = 0; i < x * y; i++) {
		Entity& entity = grid[i].entity;
		registry.terrainCells.emplace(entity);
		registry.motions.emplace(entity);
		Motion& motion = registry.motions.get(entity);

		motion.position = to_world_coordinates(i);

		TerrainCell& cell = registry.terrainCells.get(entity);

		// TODO: Insert to 'terrainRenderRequests' to make rendering much more optimized
		// by only rendering the entire terrain in 1 draw call.
		registry.renderRequests.insert(entity, make_render_request(cell));
	}
}
