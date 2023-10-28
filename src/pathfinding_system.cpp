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

        // printf("size of mobSlowEffect registry: %i\n", registry.mobSlowEffects.components.size());
        // printf("Mob: %d\n", mob);
        // printf("Mob cell index: %d\n", terrain->get_cell_index(terrain->get_cell(registry.motions.get(mob).position)));
        // printf("Mob position before (x): %f\n", registry.motions.get(mob).position[0]);
        // printf("Mob position before (y): %f\n", registry.motions.get(mob).position[1]);
        // printf("Mob velocity before (dx): %f\n", registry.motions.get(mob).velocity[0]);
        // printf("Mob velocity before (dy): %f\n", registry.motions.get(mob).velocity[1]);

        // Stop mob from tracking the player if mob is tracking the player and has reached the next cell in their path and:
        // 1) player is not in the aggro range of the mob, or
        // 2) mob is in the same cell as the player
        if (mob_mob.is_tracking_player && reached_next_cell(mob) && (!is_player_in_mob_aggro_range(player, mob) || same_cell(player, mob))) {
            stop_tracking_player(mob);
        }

        // Find new path from mob to player if:
        // 1) mob is not tracking the player and player is within aggro range of mob and mob is not in the same cell as the player already, or
        // 2) mob is tracking the player and has reached the next cell in their path and the player has moved
        if ((!mob_mob.is_tracking_player && is_player_in_mob_aggro_range(player, mob) && !same_cell(player, mob)) ||
            (mob_mob.is_tracking_player && reached_next_cell(mob) && has_player_moved(player, mob))
        ) {
            if (!mob_mob.is_tracking_player) {
                mob_mob.is_tracking_player = true;
            }
            
            std::deque<Entity> new_path = find_shortest_path(player, mob);
            Path& mob_path = registry.paths.get(mob);
            mob_path.path = new_path;
        }

        // printf("Path for mob %d: ", mob);
        // Path& mob_path = registry.paths.get(mob);
        // std::deque<Entity> path_copy = mob_path.path;
        // while (!path_copy.empty()) {
        //     printf("%d ", terrain->get_cell_index(path_copy.front()));
        //     path_copy.pop_front();
        // }
        // printf("\n");

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
    int index = terrain->get_cell_index(player_cell);
    while (predecessor.at(index) != -1) {
        path.push_front(terrain->get_cell(predecessor.at(index)));
        index = predecessor.at(index);
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
    // Get cells the player and mob are in
    Motion& player_motion = registry.motions.get(player);
    Motion& mob_motion = registry.motions.get(mob);
    Entity player_cell = terrain->get_cell(player_motion.position);
    Entity mob_cell = terrain->get_cell(mob_motion.position);

    // Check if player is in the same cell as the mob
    return player_cell == mob_cell;
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
    // Get the cell the player is in and the cell the mob believes the player is in
    Motion& player_motion = registry.motions.get(player);
    Entity curr_cell_of_player = terrain->get_cell(player_motion.position);
    Path& mob_path = registry.paths.get(mob);
    Entity expected_cell_of_player = mob_path.path.back();

    // Check if the cell the player is in and the cell the mob believes the player is in are different
    return curr_cell_of_player != expected_cell_of_player;
}

void PathfindingSystem::stop_tracking_player(Entity mob) 
{
    // Get the motion, mob, and path of the mob
    Motion& mob_motion = registry.motions.get(mob);
    Mob& mob_mob = registry.mobs.get(mob);
    Path& mob_path = registry.paths.get(mob);

    // Set mob's velocity to 0, tracking player flag to false, and clear the path 
    mob_motion.velocity = {0.f, 0.f};
    mob_mob.is_tracking_player = false;
    mob_path.path.clear();
}

bool PathfindingSystem::is_player_in_mob_aggro_range(Entity player, Entity mob) {
    // Get the player and mob motion and aggro range of the mob
    Motion& player_motion = registry.motions.get(player);
    Motion& mob_motion = registry.motions.get(mob);
    Mob& mob_mob = registry.mobs.get(mob);
    float mob_aggro_range = mob_mob.aggro_range;

    // Calculate the distance between the player and mob
    float dist = distance(mob_motion.position, player_motion.position);

    // printf("Distance between player and mob %d: %f\n", mob, dist);
    // printf("Player is in aggro range of mob? %d\n", dist <= mob_aggro_range);

    // Check if the player is within the aggro range of the mob
    return dist <= mob_aggro_range;
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
        stop_tracking_player(mob);
        return;
    }

    // Get angle to next cell in the path 
    Entity next_cell = mob_path.path.front();
    Motion& next_cell_motion = registry.motions.get(next_cell);
    float angle = atan2(next_cell_motion.position.y - mob_motion.position.y, next_cell_motion.position.x - mob_motion.position.x);

    // Update the velocity of the mob based on angle. If mob is slowed, apply the slow effect to the velocity
    Mob& mob_mob = registry.mobs.get(mob);
    float mob_speed_ratio = mob_mob.speed_ratio;
    mob_motion.velocity[0] = cos(angle) * mob_speed_ratio;
    mob_motion.velocity[1] = sin(angle) * mob_speed_ratio;

    if (registry.mobSlowEffects.has(mob)) {
        MobSlowEffect& mob_mobSlowEffect = registry.mobSlowEffects.get(mob);
        float mob_slow_ratio = mob_mobSlowEffect.slow_ratio;

        if (mob_mobSlowEffect.applied) {
            mob_mobSlowEffect.initial_velocity = mob_motion.velocity;
            mob_motion.velocity = mob_motion.velocity * mob_slow_ratio;
        }
    }
};