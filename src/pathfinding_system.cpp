#include "pathfinding_system.hpp"

void PathfindingSystem::init(TerrainSystem* terrain_arg)
{
    this->terrain = terrain_arg;
};
    
void PathfindingSystem::step(float elapsed_ms) 
{
    // TODO: better way to get player?
    Entity player = registry.players.entities[0]; 

    printf("++++++++PLAYER++++++++\n");
    printf("Player: %d\n", player);
    printf("Player cell: %d\n", terrain->get_cell(registry.motions.get(player).position));

    printf("++++++++MOBS++++++++\n");
    for (Entity mob : registry.mobs.entities) {
        Mob& mob_mob = registry.mobs.get(mob);

        printf("Mob: %d\n", mob);
        printf("Mob cell: %d\n", terrain->get_cell(registry.motions.get(mob).position));
        printf("Mob is_tracking_player: %d\n", mob_mob.is_tracking_player);
        printf("Mob velocity before (dx): %f\n", registry.motions.get(mob).velocity[0]);
        printf("Mob velocity before (dy): %f\n", registry.motions.get(mob).velocity[1]);

        // Find new path from mob to player if mob is not tracking the player and not in the same cell
        // as the player already
        if (!mob_mob.is_tracking_player && !same_cell(player, mob)) {
            mob_mob.is_tracking_player = true;
            std::stack<Entity> new_path = find_shortest_path(player, mob);
            Path& mob_path = registry.paths.get(mob);
            mob_path.path = new_path;
        }

        Path& mob_path = registry.paths.get(mob);
        std::stack<Entity> path_copy = mob_path.path;
        while (!path_copy.empty()) {
            printf("Path (as cell indices): %d\n", terrain->get_cell_index(path_copy.top()));
            path_copy.pop();
        }

        // Update velocity of mob if they are tracking the player and reached the next cell in their path
        if (mob_mob.is_tracking_player && reached_next_cell(mob)) {
            update_velocity_to_next_cell(mob, elapsed_ms);
        }

        printf("Mob velocity after (dx): %f\n", registry.motions.get(mob).velocity[0]);
        printf("Mob velocity after (dy): %f\n", registry.motions.get(mob).velocity[1]);

        printf("\n");
    }
};


std::stack<Entity> PathfindingSystem::find_shortest_path(Entity player, Entity mob)
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
    std::stack<Entity> path;
    path.push(player_cell);
    int crawl = terrain->get_cell_index(player_cell);
    while (predecessor.at(crawl) != -1) {
        path.push(terrain->get_cell(predecessor.at(crawl)));
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

        printf("Current cell index: %d\n", curr_cell_index);

        // Get neighbors of cell
        std::vector<Entity> neighbors;
        terrain->get_accessible_neighbours(curr, neighbors);

        for (Entity neighbor : neighbors) {
            int neighbor_cell_index = terrain->get_cell_index(neighbor);

            printf("Neighbor cell index: %d\n", neighbor_cell_index);
            printf("Neighbor is visited before: %d\n", visited.at(neighbor_cell_index));
            printf("Neighbor predecessor before: %d\n", predecessor.at(neighbor_cell_index));

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

            printf("Neighbor is visited after: %d\n", visited.at(neighbor_cell_index));
            printf("Neighbor predecessor after: %d\n", predecessor.at(neighbor_cell_index));
        }
    }

    return false;
};

bool PathfindingSystem::same_cell(Entity player, Entity mob)
{
    Motion& player_motion = registry.motions.get(player);
    Motion& mob_motion = registry.motions.get(mob);
    return terrain->get_cell(player_motion.position) == terrain->get_cell(mob_motion.position);
};


bool PathfindingSystem::reached_next_cell(Entity mob)
{
    return true;
};


void PathfindingSystem::update_velocity_to_next_cell(Entity mob, float elapsed_ms)
{
    
};