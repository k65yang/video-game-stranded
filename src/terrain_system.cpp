#include "terrain_system.hpp"
#include "cstring"

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
	for (int i = 0; i < size_x * size_y; i++) {
		Entity& entity = entity_grid[i];
		Motion& motion = registry.motions.emplace(entity);
		motion.position = to_world_coordinates(i);
		TerrainCell& cell = registry.terrainCells.emplace(entity, terraincell_grid[i]);
	}
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

void TerrainSystem::save_grid(const std::string& name)
{
	const std::string path = map_path_builder(name);
	std::ofstream file(path.c_str(), std::ios::binary | std::ios::out);
	file.write(map_ext.c_str(), map_ext.size());
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

std::vector<vec2> TerrainSystem::get_mob_spawn_locations(std::map<ZONE_NUMBER,int> num_per_zone) {
    std::vector<vec2> result;
    std::default_random_engine rng;

    for (const auto& kv : num_per_zone) {
        ZONE_NUMBER zone = kv.first;
        int num_mobs = kv.second;

        // Throw error if invalid zone
        if (zone_radius_map.count(zone) == 0)
            throw(std::runtime_error("Error: Cannot spawn mob in invalid zone"));

		// Keep generating random locations until you meet the num_mobs quota
		// Be sure to check if the current location is in the zone we want
        while(num_mobs) {
            result.push_back(get_random_terrain_location(zone));
			num_mobs--;
        }
    }
    return result;
}

vec2 TerrainSystem::get_random_terrain_location() {
	// Get a random zone
	std::random_device rd;
	std::default_random_engine rng(rd());
	std::uniform_int_distribution<int> distribution(0, ZONE_COUNT-1);
	ZONE_NUMBER random_zone = static_cast<ZONE_NUMBER>(distribution(rng));

	// Uncomment for debug
	// printf("zone: %i\n", random_zone);

	// Get a random location in that random zone
	return get_random_terrain_location(random_zone);
}

vec2 TerrainSystem::get_random_terrain_location(ZONE_NUMBER zone) {
	vec2 position;

	// Determine the location range
	int range_x = zone_radius_map[zone] * 2 > size_x ? size_x : zone_radius_map[zone] * 2;
	int range_y = zone_radius_map[zone] * 2 > size_y ? size_y : zone_radius_map[zone] * 2;

	// set up the random number generator
	std::random_device rng;
    std::mt19937 generator(rng());  // Seed the generator
    std::uniform_int_distribution<> distribution_x(-range_x/2 + 1, range_x/2 - 1);
	std::uniform_int_distribution<> distribution_y(-range_y/2 + 1, range_y/2 - 1);

	// Store the radius of the current and previous zones
	float curr_zone_radius = (float) zone_radius_map[zone];
	float prev_zone_radius;
	if (zone == ZONE_0)
		prev_zone_radius = 0.f;
	else
		prev_zone_radius = (float) zone_radius_map[static_cast<ZONE_NUMBER>(zone-1)];


	// Get unused spawn location within the given zone
	while (true) {
		position.x = distribution_x(generator);
		position.y = distribution_y(generator);

		// Skip locations that are covered by spaceship
		if (position.x <= 1 && position.x >= -1 && position.y <= 2 && position.y >= -2) {
			continue;
		}

		// Skip locations that are not accessible
		if (is_invalid_spawn(position))
			continue;

		// Skip locations that is not within the current zone
		float distance = sqrt(pow(position.x,2) + pow(position.y,2));
		if (distance > curr_zone_radius || distance < prev_zone_radius)
			continue;

		if (!is_terrain_location_used(position)) {
			// Uncomment for debug
			// printf("position.x: %f, position.y: %f, distance: %f \n", position.x, position.y, distance);
			break;
		}
	}

	// Add terrain location to used terrain locations
	used_terrain_locations.insert(position);
	// printf("position x: %f, position y: %f \n", position.x, position.y);
	return position;
}

bool TerrainSystem::is_terrain_location_used(vec2 position) {
	// for (vec2 p : used_terrain_locations) {
	// 	if (p.x == position.x && p.y == position.y) {
	// 		return true;
	// 	}
	// }

	// return false;
	return used_terrain_locations.count(position);
}
