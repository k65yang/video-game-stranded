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
	assert(x < size_x);
	assert(y < size_y);
	return grid[to_array_index(x, y)];
}

char TerrainSystem::get_accessible_neighbours(Entity cell, Entity* buffer)
{
	// TODO: Return all accessible, non-collidable neighbours
	assert(buffer != nullptr);
	assert(registry.terrainCells.has(cell));
	int cell_index = entityStart - cell;
	return 0;
}

unsigned int TerrainSystem::to_array_index(int x, int y)
{
	x = size_x / 2 + x;
	y = size_y / 2 + y;
	return (y * size_x + x);
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
	grid = new Entity[x * y]();	// 1D 2-dimensional array so we can guarantee that
								// the entire world is in the same memory block.
	entityStart = grid[0];

	for (int i = 0; i < x * y; i++) {
		Entity& entity = grid[i];
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
