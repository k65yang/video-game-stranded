#include "terrain_system.hpp"

void TerrainSystem::init(const unsigned int x, const unsigned int y, RenderSystem* renderer)
{
	this->renderer = renderer;

	if (terraincell_grid != nullptr) {		// if grid is allocated, deallocate
		registry.terrainCells.clear();
		delete[] terraincell_grid;
		terraincell_grid = nullptr;
	}

	if (entity_grid != nullptr) {			// if grid is allocated, deallocate
		for (uint i = 0; i < size_x * size_y; i++) {
			registry.remove_all_components_of(entity_grid[i]);
		}
		delete[] entity_grid;
		entity_grid = nullptr;
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
			terraincell_grid[i] = ((uint32)TERRAIN_TYPE::ROCK << 16) | TERRAIN_FLAGS::COLLIDABLE;
		}
		else {
			terraincell_grid[i] = ((uint32_t)TERRAIN_TYPE::GRASS) << 16;
		}

		TerrainCell& cell = registry.terrainCells.emplace(entity, terraincell_grid[i]);
	}

	std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS> orientation_map;
	renderer->initializeTerrainBuffers(orientation_map);
}

void TerrainSystem::init(const std::string& map_name, RenderSystem* renderer)
{
	this->renderer = renderer;

	if (terraincell_grid != nullptr) {		// if grid is allocated, deallocate
		registry.terrainCells.clear();
		delete[] terraincell_grid;
		terraincell_grid = nullptr;
	}

	if (entity_grid != nullptr) {			// if grid is allocated, deallocate
		for (uint i = 0; i < size_x * size_y; i++) {
			registry.remove_all_components_of(entity_grid[i]);
		}
		delete[] entity_grid;
		entity_grid = nullptr;
	}

	load_grid(map_name);	// Load map from file
	entity_grid = new Entity[size_x * size_y]();
	entityStart = entity_grid[0];

	// Bind entities to respective TerrainCell
	for (unsigned int i = 0; i < size_x * size_y; i++) {
		Entity& entity = entity_grid[i];
		Motion& motion = registry.motions.emplace(entity);
		motion.position = to_world_coordinates(i);
		TerrainCell& cell = registry.terrainCells.emplace(entity, terraincell_grid[i]);
	}

	std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS> orientation_map;
	generate_orientation_map(orientation_map);
	renderer->initializeTerrainBuffers(orientation_map);
}

void TerrainSystem::step(float delta_time)
{
}

Entity TerrainSystem::get_cell(vec2 position)
{
	// round x and y value before casting for more accurate mapping of position to cell
	return get_cell((int)std::round(position.x), (int)std::round(position.y));
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

float TerrainSystem::get_terrain_speed_ratio(Entity cell)
{
    // Get terrain type of cell
	TerrainCell& c = registry.terrainCells.get(cell);

	return terrain_type_to_speed_ratio[c.terrain_type];
}

void TerrainSystem::get_accessible_neighbours(Entity cell, std::vector<Entity>& buffer, bool ignoreColliders, bool checkPathfind)
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

	filter_neighbouring_indices(cell_index, indices);

	for (int i = 0; i < 8; i++) {
		int index = indices[i];

		// Skip cell if index is -1
		if (index < 0) {
			continue;
		}

		Entity& tile = entity_grid[index];
		unsigned int cell = terraincell_grid[index];

		// If a cell is not collidable and pathfinding is not disabled, add to buffer
		if (ignoreColliders) {
			buffer.push_back(tile);
		}
		else {
			if (!(cell & filter)) {
				buffer.push_back(tile);
			}
		}
	}
}

void TerrainSystem::filter_neighbouring_indices(int cell_index, int indices[8])
{
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
}

void TerrainSystem::save_grid(const std::string& name)
{
	const std::string path = map_path_builder(name);
	std::ofstream file(path.c_str(), std::ios::binary | std::ios::out);
	file.write(map_ext.c_str(), size(map_ext));
	assert(file.is_open() && "Map file cannot be created or modified.");
	write_to_file(file, savefile_version);
	write_to_file(file, size_x);
	write_to_file(file, size_y);

	// Add 32 bytes of padding
	file.seekp(SMAP_PADDING_BYTES, std::ios::cur);

	file.write((char*)terraincell_grid, sizeof(uint32_t) * size_x * size_y);

	file.close();
}

void TerrainSystem::load_grid(const std::string& name)
{
	const std::string path = map_path_builder(name);
	std::ifstream file(path.c_str(), std::ios::binary);
	assert(file.is_open() && "Map file cannot be found.");
	char ext[5] = "";
	unsigned int save_version;

	file.read((char*)&ext, 4);

	if (strcmp(ext, map_ext.c_str()) != 0) {
		assert(false && "Map file header does not match with expected file format.");
	}

	read_from_file(file, save_version);
	read_from_file(file, size_x);
	read_from_file(file, size_y);

	// Read past 32 bytes of padding
	file.seekg(SMAP_PADDING_BYTES, std::ios::cur);
	if (terraincell_grid == nullptr) {
		terraincell_grid = new uint32_t[size_x * size_y];
	}

	file.read((char*)terraincell_grid, sizeof(uint32_t) * size_x * size_y);
	file.close();
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
	return { (index % size_x) - size_x / 2,
			index / size_x - size_y / 2 };
}

bool TerrainSystem::matches(uint16_t current_type, int index) {
	if (index < 0) return true;
	uint16_t cell_type = terraincell_grid[index] >> 16;
	return !(current_type ^ cell_type);
}

bool TerrainSystem::check_match(uint16_t current, int indices[8], 
	std::initializer_list<uint8_t> match_list, 
	std::initializer_list<uint8_t> reject_list) {
	for (int i : match_list) {
		assert(i < 8);
		i = indices[i];
		if (!matches(current, i))
			return false;
	}

	for (int i : reject_list) {
		assert(i < 8);
		i = indices[i];
		if (matches(current, i))
			return false;
	}

	return true;
}

RenderSystem::ORIENTATIONS TerrainSystem::find_tile_orientation(int centre_index) {
	int indices[8] = {
	centre_index - size_x,		// Top cell
	centre_index + 1,			// Right cell
	centre_index + size_x,		// Bottom cell
	centre_index - 1,			// Left cell
	centre_index - size_x + 1,	// Top-right cell
	centre_index + size_x + 1,	// Bottom-right cell
	centre_index + size_x - 1,	// Bottom-left cell
	centre_index - size_x - 1,	// Top-left cell
	};
	filter_neighbouring_indices(centre_index, indices);
	return find_tile_orientation((uint16_t)(terraincell_grid[centre_index] >> 16), indices);
}

RenderSystem::ORIENTATIONS TerrainSystem::find_tile_orientation(uint16_t current, int indices[8]) {
	// Surrounded and inner corners.
	if (check_match(current, indices, { TOP, BOTTOM, LEFT, RIGHT })) {
		if (check_match(current, indices, {}, { BL, BR })) {
			return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_VERTICAL;
		}
		else if (check_match(current, indices, {}, { TL, TR })) {
			return RenderSystem::RenderSystem::ORIENTATIONS::INSIDE;
		}
		else if (check_match(current, indices, { TR }, { BR })) {
			return RenderSystem::ORIENTATIONS::CORNER_INNER_BOTTOM_RIGHT;
		}
		else if (check_match(current, indices, { TL }, { BL })) {
			return RenderSystem::ORIENTATIONS::CORNER_INNER_BOTTOM_LEFT;
		}
		else if (check_match(current, indices, { BR }, { TR })) {
			return RenderSystem::ORIENTATIONS::CORNER_INNER_TOP_RIGHT;
		}
		else if (check_match(current, indices, { BL }, { TL })) {
			return RenderSystem::ORIENTATIONS::CORNER_INNER_TOP_LEFT;
		}
		else {
			return RenderSystem::ORIENTATIONS::INSIDE;
		}
	}

	// Traditional edges
	else if (check_match(current, indices, { TOP, BOTTOM, LEFT }, { RIGHT })) {
		return RenderSystem::ORIENTATIONS::EDGE_RIGHT;
	}
	else if (check_match(current, indices, { TOP, BOTTOM, RIGHT }, { LEFT })) {
		return RenderSystem::ORIENTATIONS::EDGE_LEFT;
	}
	else if (check_match(current, indices, { RIGHT, LEFT, TOP }, { BOTTOM })) {
		return RenderSystem::ORIENTATIONS::EDGE_BOTTOM;
	}
	else if (check_match(current, indices, { RIGHT, LEFT, BOTTOM }, { TOP })) {
		return RenderSystem::ORIENTATIONS::EDGE_TOP;
	}

	// Outer corners
	else if (check_match(current, indices, { RIGHT, BOTTOM }, { TOP, LEFT })) {
		return RenderSystem::ORIENTATIONS::CORNER_OUTER_TOP_LEFT;
	}
	else if (check_match(current, indices, { RIGHT, TOP }, { BOTTOM, LEFT })) {
		return RenderSystem::ORIENTATIONS::CORNER_OUTER_BOTTOM_LEFT;
	}
	else if (check_match(current, indices, { LEFT, BOTTOM }, { TOP, RIGHT })) {
		return RenderSystem::ORIENTATIONS::CORNER_OUTER_TOP_RIGHT;
	}
	else if (check_match(current, indices, { LEFT, TOP }, { BOTTOM, RIGHT })) {
		return RenderSystem::ORIENTATIONS::CORNER_OUTER_BOTTOM_RIGHT;
	}

	// Double edges
	else if (check_match(current, indices, { RIGHT, LEFT }, { TOP, BOTTOM })) {
		return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_HORIZONTAL;
	}
	else if (check_match(current, indices, { TOP, BOTTOM }, { RIGHT, LEFT })) {
		return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_VERTICAL;
	}

	// Double edged "tips"
	else if (check_match(current, indices, { TOP }, { RIGHT, LEFT, BOTTOM })) {
		return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_END_BOTTOM;
	}
	else if (check_match(current, indices, { BOTTOM }, { RIGHT, LEFT, TOP })) {
		return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_END_TOP;
	}
	else if (check_match(current, indices, { LEFT }, { RIGHT, TOP, BOTTOM })) {
		return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_END_RIGHT;
	}
	else if (check_match(current, indices, { RIGHT }, { LEFT, TOP, BOTTOM })) {
		return RenderSystem::ORIENTATIONS::DOUBLE_EDGE_END_LEFT;
	}
	// Default case
	return RenderSystem::ORIENTATIONS::EDGE_BOTTOM;	// default texture is bottom face remember
}

void TerrainSystem::generate_orientation_map(std::unordered_map<unsigned int, RenderSystem::ORIENTATIONS>& map)
{
	for (int cell_index = 0; cell_index < size_x * size_y; cell_index++) {
		uint16_t current = terraincell_grid[cell_index] >> 16;
		if (directional_terrain.count(static_cast<TERRAIN_TYPE>(current))) {
			map.insert({ cell_index, find_tile_orientation(cell_index)});
		}
	}
}
