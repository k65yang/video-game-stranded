#include "pathfinding_system.hpp"

void PathfindingSystem::init(TerrainSystem* terrain_arg)
{
    this->terrain = terrain_arg;
};
    
void PathfindingSystem::step(float elapsed_ms) 
{
    // TODO: better way to get player?
    Entity player = registry.players.entities[0]; 

    // printf("++++++++PLAYER++++++++\n");
    // printf("Player: %d\n", player);
    // printf("Player cell index: %d\n", terrain->get_cell_index(terrain->get_cell(registry.motions.get(player).position)));

    // printf("++++++++MOBS++++++++\n");
    for (Entity mob : registry.mobs.entities) {
        Mob& mob_mob = registry.mobs.get(mob);

        // printf("Mob: %d\n", mob);
        // printf("Mob cell index: %d\n", terrain->get_cell_index(terrain->get_cell(registry.motions.get(mob).position)));
        // printf("Mob is_tracking_player: %d\n", mob_mob.is_tracking_player);
        // printf("Mob position before (x): %f\n", registry.motions.get(mob).position[0]);
        // printf("Mob position before (y): %f\n", registry.motions.get(mob).position[1]);
        // printf("Mob velocity before (dx): %f\n", registry.motions.get(mob).velocity[0]);
        // printf("Mob velocity before (dy): %f\n", registry.motions.get(mob).velocity[1]);

        // Find new path from mob to player if mob is not tracking the player and not in the same cell
        // as the player already
        if (!mob_mob.is_tracking_player && !same_cell(player, mob)) {
            mob_mob.is_tracking_player = true;
            
            std::deque<Entity> new_path = find_shortest_path(player, mob);
            Path& mob_path = registry.paths.get(mob);
            mob_path.path = new_path;
        }

        // Path& mob_path = registry.paths.get(mob);
        // std::stack<Entity> path_copy = mob_path.path;
        // while (!path_copy.empty()) {
        //     printf("Path (cell index): %d\n", terrain->get_cell_index(path_copy.top()));
        //     path_copy.pop();
        // }

        // Update velocity of mob if they are tracking the player and reached the next cell in their path
        if (mob_mob.is_tracking_player && reached_next_cell(mob)) {
            update_velocity_to_next_cell(mob, elapsed_ms);
        }

        // printf("Mob position after (x): %f\n", registry.motions.get(mob).position[0]);
        // printf("Mob position after (y): %f\n", registry.motions.get(mob).position[1]);
        // printf("Mob velocity after (dx): %f\n", registry.motions.get(mob).velocity[0]);
        // printf("Mob velocity after (dy): %f\n", registry.motions.get(mob).velocity[1]);

        // printf("\n");
    }
};


std::deque<Entity> PathfindingSystem::find_shortest_path(Entity player, Entity mob)
{
    // Get the cells the player and mob are in
    Motion& player_motion = registry.motions.get(player);
    Motion& mob_motion = registry.motions.get(mob);
    Entity player_cell = terrain->get_cell(player_motion.position);
    Entity mob_cell = terrain->get_cell(mob_motion.position);

    // Initialize predecessor array for BFS
    // predecessor is an array of ints which correspond to indices of cells in the world grid. predecessor[i] 
    // represents the immediate predecessor of the cell at index i in the world grid found during the BFS
    std::vector<int> predecessor;

    // Execute BFS
    if (!BFS(player_cell, mob_cell, predecessor)) {
        printf("Path from mob in cell %d to player in cell %d could not be found\n", mob_cell, player_cell);
        assert(false);
    }

    // Get shortest path by backtracking through predecessors
    std::deque<Entity> path;
    path.push_front(player_cell);
    int crawl = terrain->get_cell_index(player_cell);
    while (predecessor.at(crawl) != -1) {
        path.push_front(terrain->get_cell(predecessor.at(crawl)));
        crawl = predecessor.at(crawl);
    }

    return path;
};


bool PathfindingSystem::BFS(Entity player_cell, Entity mob_cell, std::vector<int>& predecessor)
{
    // Initialize queue for BFS
    std::queue<Entity> bfs_queue;

    // Initialize visited array for BFS
    // visited is an array of booleans where visited[i] indicates whether the cell at index i in the world grid
    // has been visited during the BFS
    std::vector<bool> visited;

    // Initialize values in visited false as all cells start out unvisited
    // Initialize values in predecessor to -1 as predecessors for cells start out unknown
    for (int i = 0; i < terrain->size_x * terrain->size_y; i++) {
        visited.push_back(false);
        predecessor.push_back(-1);
    }  

    // Start BFS from mob cell so mark it as visited and add to BFS queue
    visited[terrain->get_cell_index(mob_cell)] = true;
    bfs_queue.push(mob_cell);

    // BFS algorithm
    while (!bfs_queue.empty()) {
        // Get and remove first cell from queue
        Entity curr = bfs_queue.front();
        int curr_cell_index = terrain->get_cell_index(curr);
        bfs_queue.pop();

        // Get neighbors of cell
        std::vector<Entity> neighbors;
        terrain->get_accessible_neighbours(curr, neighbors);

        for (Entity neighbor : neighbors) {
            int neighbor_cell_index = terrain->get_cell_index(neighbor);

            // Set cell as visited, save its predecessor, and add it to the BFS queue if cell has not been visited yet
            if (!visited[neighbor_cell_index]) {
                visited.at(neighbor_cell_index) = true;
                predecessor.at(neighbor_cell_index) = curr_cell_index;
                bfs_queue.push(neighbor);

                // Stop BFS if the cell is the one the player is in
                if (player_cell == neighbor) {
                    return true;
                }
            }
        }
    }

    return false;
};

bool PathfindingSystem::same_cell(Entity player, Entity mob)
{
    // Get player and mob motion
    Motion& player_motion = registry.motions.get(player);
    Motion& mob_motion = registry.motions.get(mob);

    // Check if player is in the same cell as the mob
    return terrain->get_cell(player_motion.position) == terrain->get_cell(mob_motion.position);
};


bool PathfindingSystem::reached_next_cell(Entity mob)
{
    // Get the next cell in the path and the cell the mob is in
    Motion& mob_motion = registry.motions.get(mob);
    Path& mob_path = registry.paths.get(mob);
    Entity next_cell = mob_path.path.front();

    // Calculate the distance between the mob and the center of the next cell
    Motion& next_cell_motion = registry.motions.get(next_cell);
    float dist = distance(mob_motion.position, next_cell_motion.position);

    // printf("Next cell index: %d\n", terrain->get_cell_index(next_cell));
    // printf("Next cell position (x): %f\n", next_cell_motion.position.x);
    // printf("Next cell position (y): %f\n", next_cell_motion.position.y);
    // printf("Distance between mob and next cell: %f\n", dist);

    // Check if mob is close to the center of the next cell
    return dist < 0.01;
};

bool PathfindingSystem::has_player_moved(Entity player, Entity mob) 
{
    return true;
}

void PathfindingSystem::update_velocity_to_next_cell(Entity mob, float elapsed_ms)
{
    // Get and remove previous cell in the path
    Path& mob_path = registry.paths.get(mob);
    Entity prev_cell = mob_path.path.front();
    mob_path.path.pop_front();

    // Set the position of the mob to the previous cell to keep mob in the middle of the path
    Motion& mob_motion = registry.motions.get(mob);
    Motion& prev_cell_motion = registry.motions.get(prev_cell);
    mob_motion.position = prev_cell_motion.position;

    // Stop mob from tracking the player if there are no more cells in its path
    if (mob_path.path.empty()) {
        Mob& mob_mob = registry.mobs.get(mob);
        mob_mob.is_tracking_player = false;
        mob_motion.velocity = {0.f, 0.f};
        return;
    }

    // Get next cell in the path and update the velocity of the mob
    Entity next_cell = mob_path.path.front();
    Motion& next_cell_motion = registry.motions.get(next_cell);
    float angle = atan2(next_cell_motion.position.y - mob_motion.position.y, next_cell_motion.position.x - mob_motion.position.x);
    mob_motion.velocity[0] = cos(angle) * 0.5;
    mob_motion.velocity[1] = sin(angle) * 0.5;
};