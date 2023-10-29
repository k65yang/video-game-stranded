#include "terrain_system.hpp"

void TerrainSystem::step(float delta_time)
{
}

Entity TerrainSystem::get_cell(vec2 position)
{
	// round x and y value before casting for more accurate mapping of position to cell
	return get_cell((int) std::round(position.x), (int) std::round(position.y)); 
}

Entity TerrainSystem::get_cell(int x, int y)
{
	assert(entity_grid != nullptr);
	assert(abs(x) <= size_x / 2);
	assert(abs(y) <= size_y / 2);
	return entity_grid[to_array_index(x, y)];
}

Entity TerrainSystem::get_cell(int index)
{
	assert(entity_grid != nullptr);
	assert(index >= 0);
	assert(index < size_x * size_y);
	return entity_grid[index];
}

int TerrainSystem::get_cell_index(Entity cell) 
{
	int cell_index = cell - entityStart;
	assert(cell_index >= 0);
	assert(cell_index < size_x * size_y);
	return cell_index;
}

void TerrainSystem::get_accessible_neighbours(Entity cell, std::vector<Entity>& buffer, bool checkPathfind)
{
	assert(entity_grid != nullptr);
	assert(terraincell_grid != nullptr);
	assert(registry.terrainCells.has(cell));
	int cell_index = cell - entityStart;
	unsigned int filter = TERRAIN_FLAGS::COLLIDABLE;	// check if tile is collidable

	if (checkPathfind)
		filter |= TERRAIN_FLAGS::DISABLE_PATHFIND;	// check if tile has pathfinding disabled

	int indices[8] = { 
		cell_index - size_x,		// Top cell
		cell_index + 1,				// Right cell
		cell_index + size_x,		// Bottom cell
		cell_index - 1,				// Left cell
		cell_index - size_x + 1,	// Top-right cell
		cell_index + size_x + 1,	// Bottom-right cell
		cell_index + size_x - 1,	// Bottom-left cell
		cell_index - size_x - 1,	// Top-left cell
	};

	// Check bounds
	// if cell on left edge, skip top-left, left, or bottom-left neighbors
	if (cell_index % size_x == 0) {
		indices[7] = -1;
		indices[3] = -1;
		indices[6] = -1;
	}
	// if cell on right edge, skip top-right, right, or bottom-right neighbors
	if (cell_index % size_x == size_x - 1) {
		indices[4] = -1;
		indices[1] = -1;
		indices[5] = -1;
	}
	// if cell on top edge, skip top-left, top, top-right neighbors
	if (cell_index < size_x) {
		indices[7] = -1;
		indices[0] = -1;
		indices[4] = -1;
	}
	// if cell on bottom edge, skip bottom-left, bottom, bottom-right neighbors
	if (cell_index >= (size_x - 1) * size_y && cell_index < size_x * size_y) {
		indices[6] = -1;
		indices[2] = -1;
		indices[5] = -1;
	}

	for (int i = 0; i < 8; i++) {
		int index = indices[i];

		// Skip cell if index is -1
		if (index < 0) {
			continue;
		}

		Entity& tile = entity_grid[index];
		unsigned int cell = terraincell_grid[index];

		// If a cell is not collidable and pathfinding is not disabled, add to buffer
		if (!(cell & filter)) {	
			buffer.push_back(tile);
		}
	}
}

void TerrainSystem::save_grid(const std::string& name)
{
	std::string path = map_path_builder(name);
	const uint64_t padding = 0;

	std::ofstream file(path.c_str(), std::ios::binary | std::ios::out);
	file.write(map_ext.c_str(), size(map_ext));
	write_to_file(file, savefile_version);
	write_to_file(file, size_x);
	write_to_file(file, size_y);

	// Add 32 bytes of padding
	for (uint i = 0; i < 4; i++)
		write_to_file(file, padding);



	file.close();
}

void TerrainSystem::load_grid(const std::string& name)
{
}

unsigned int TerrainSystem::to_array_index(int x, int y)
{
	assert(abs(x) <= size_x / 2);
	assert(abs(y) <= size_y / 2);
	x = size_x / 2 + x;
	y = size_y / 2 + y;
	return (y * size_x + x);
}

vec2 TerrainSystem::to_world_coordinates(const int index)
{
	assert(index >= 0 && index < size_x * size_y);
	return {(index % size_x) - size_x / 2,
			index / size_x - size_y / 2 };
}

void TerrainSystem::init(const unsigned int x, const unsigned int y, RenderSystem* renderer)
{
	this->renderer = renderer;

	if (entity_grid != nullptr) {		// if grid is allocated, deallocate
		delete[] entity_grid;
		entity_grid = nullptr;
	}

	if (terraincell_grid != nullptr) {		// if grid is allocated, deallocate
		delete[] terraincell_grid;
		terraincell_grid = nullptr;
	}

	size_x = x;
	size_y = y;

	entity_grid = new Entity[x * y]();			// 1D 2-dimensional array so we can guarantee that
	terraincell_grid = new uint32_t[x * y];	// the entire world is in the same memory block.
	entityStart = entity_grid[0];

	for (int i = 0; i < x * y; i++) {
		Entity& entity = entity_grid[i];
		Motion& motion = registry.motions.emplace(entity);
		motion.position = to_world_coordinates(i);
		if (i % x == 0 || i % x == x - 1 ||
			i / y == 0 || i / y == y - 1) {
			terraincell_grid[i] |= ((uint32)TERRAIN_TYPE::ROCK << 16) | TERRAIN_FLAGS::COLLIDABLE;
		}
		else {
			terraincell_grid[i] = ((uint32_t)TERRAIN_TYPE::GRASS) << 16;
		}

		TerrainCell& cell = registry.terrainCells.emplace(entity, terraincell_grid[i]);
	}
}
