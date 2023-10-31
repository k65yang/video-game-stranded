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

        // Apply new terrain speed effect if the mob enters a new cell

        Mob& mob_obj = registry.mobs.get(mob);

        if (entered_new_cell(mob) && mob_obj.type != MOB_TYPE::GHOST) {
            // Get the cell the mob was previously in and the new cell the mob is in
            Mob& mob_mob = registry.mobs.get(mob);
            Entity prev_mob_cell = mob_mob.curr_cell;
            Motion& mob_motion = registry.motions.get(mob);
            Entity new_mob_cell = terrain->get_cell(mob_motion.position);
            
            // Update cell mob is currently in
            mob_mob.curr_cell = new_mob_cell;

            apply_new_terrain_speed_effect(mob, prev_mob_cell, new_mob_cell);
        }

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

    // Check what kind of mob it is, for example ghosts don't use A* since they can go over collidable cells
    Mob& curr = registry.mobs.get(mob);
    if (curr.type == MOB_TYPE::GHOST) {
        // Execute BFS
        if (!BFS(player_cell, mob_cell, predecessor)) {
            printf("BFS PATH from mob in cell %d to player in cell %d could not be found\n", mob_cell, player_cell);
            assert(false);
        }
    }
    else {
        // Has a collider, needs A*
        if (!A_star(player_cell, mob_cell, predecessor)) {
            printf("A* PATH from mob in cell %d to player in cell %d could not be found\n", mob_cell, player_cell);
            assert(false);
        }
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
    std::queue<Entity> bfs_queue;

    std::vector<bool> visited;

    for (int i = 0; i < terrain->size_x * terrain->size_y; i++) {
        visited.push_back(false);
        predecessor.push_back(-1);
    }

    visited[terrain->get_cell_index(mob_cell)] = true;
    bfs_queue.push(mob_cell);

    while (!bfs_queue.empty()) {
        Entity curr = bfs_queue.front();
        int curr_cell_index = terrain->get_cell_index(curr);
        bfs_queue.pop();

        std::vector<Entity> neighbors;
        // BFS is only used by ghost, so we can ignore colliders here.
        terrain->get_accessible_neighbours(curr, neighbors, true);

        for (Entity neighbor : neighbors) {
            int neighbor_cell_index = terrain->get_cell_index(neighbor);

            if (!visited[neighbor_cell_index]) {
                visited.at(neighbor_cell_index) = true;
                predecessor.at(neighbor_cell_index) = curr_cell_index;
                bfs_queue.push(neighbor);

                if (player_cell == neighbor) {
                    return true;
                }
            }
        }
    }

    return false;
};

bool PathfindingSystem::A_star(Entity player_cell, Entity mob_cell, std::vector<int>& predecessor)
{
    // Resusable values
    Motion& player_cell_motion = registry.motions.get(player_cell);

    // Initialize open priority queue for A*
    // Open is a min priority queue of pairs (x, y) ordered by x, where y = the index of a cell and x = f (f = g + h, 
    // the total cost of the cell)
    std::priority_queue<
        std::pair<float, int>, 
        std::vector<std::pair<float, int>>, 
        std::greater<std::pair<float, int>>
    > open;

    // Initialize closed array for A*
    // closed is an array of booleans where closed[i] indicates whether the cell at index i in the world grid
    // has been processed during A*
    std::vector<bool> closed;

    // Initialize g array for A*
    // g is an array of floats where g[i] indicates the cost to move from the mob cell (i.e. the starting cell) to
    // the cell at index i in the world grid
    std::vector<float> g;

    // Initialize values in closed to false as all cells start out as open
    // Initialize values in predecessor to -1 as predecessors for cells start out unknown
    // Initialize values in g to -1 as cost for cells start out unknown
    for (int i = 0; i < terrain->size_x * terrain->size_y; i++) {
        closed.push_back(false);
        predecessor.push_back(-1);
        g.push_back(-1);
    }  

    // Start A* from the mob cell so add it to open and set its g to 0
    int mob_cell_index = terrain->get_cell_index(mob_cell);
    open.push(std::make_pair(0, mob_cell_index));
    g[mob_cell_index] = 0;

    // A* algorithm
    while (!open.empty()) {
        // Get and remove top cell (i.e. cell with min total cost) from open
        std::pair<float, int> p = open.top();
        open.pop();
        int curr_cell_index = p.second;
        Entity curr = terrain->get_cell(curr_cell_index);

        // Set cell to closed
        closed.at(curr_cell_index) = true;

        // Stop A* if the cell is the one the player is in
        if (curr == player_cell) {
            return true;
        }

        // Get neighbors of cell
        std::vector<Entity> neighbors;
        terrain->get_accessible_neighbours(curr, neighbors, false);

        for (Entity neighbor : neighbors) {
            int neighbor_cell_index = terrain->get_cell_index(neighbor);

            // Skip neighbor if it is closed
            if (closed[neighbor_cell_index]) {
                continue;
            }

            // Calculate costs
            Motion& neighbor_motion = registry.motions.get(neighbor);
            float neighbor_g = g[curr_cell_index] + (1.0f * (1/terrain->get_terrain_speed_ratio(neighbor)));
            float neighbor_h = distance(neighbor_motion.position, player_cell_motion.position);
            float neighbor_f = neighbor_g + neighbor_h;

            // Do not add neighbor to open if it is already in open and came from a path with lower cost
            if (g[neighbor_cell_index] != -1 && g[neighbor_cell_index] < neighbor_g) {
                continue;
            }

            // Add neighbor to open and set predecessor and g
            open.push(std::make_pair(neighbor_f, neighbor_cell_index));
            predecessor.at(neighbor_cell_index) = curr_cell_index;
            g.at(neighbor_cell_index) = neighbor_g;
        }
    }

    return false;
}

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

bool PathfindingSystem::entered_new_cell(Entity mob)
{
    // Get the cell the mob is expected to be in and the actual cell the mob is in 
    Mob& mob_mob = registry.mobs.get(mob);
    Entity expected_mob_cell = mob_mob.curr_cell;
    Motion& mob_motion = registry.motions.get(mob);
    Entity actual_mob_cell = terrain->get_cell(mob_motion.position);

    // Check if mob has entered a new cell
    return expected_mob_cell != actual_mob_cell;
}

void PathfindingSystem::apply_new_terrain_speed_effect(Entity mob, Entity prev_cell, Entity new_cell)
{
    // Get previous and new terrain speed ratios
    float prev_terrain_speed_ratio = terrain->get_terrain_speed_ratio(prev_cell);
    float new_terrain_speed_ratio = terrain->get_terrain_speed_ratio(new_cell);

    // Remove previous terrain speed effect and apply new terrain speed effect
    Motion& mob_motion = registry.motions.get(mob);
    mob_motion.velocity /= prev_terrain_speed_ratio;
    mob_motion.velocity *= new_terrain_speed_ratio;

    // If mob is slowed by a weapon, update the initial velocity so mob moves at correct speed after weapon slow ends
    if (registry.mobSlowEffects.has(mob)) {
        MobSlowEffect& mob_mobSlowEffect = registry.mobSlowEffects.get(mob);
        float mob_slow_ratio = mob_mobSlowEffect.slow_ratio;

        if (mob_mobSlowEffect.applied) {
            mob_mobSlowEffect.initial_velocity = mob_motion.velocity / mob_slow_ratio;
        }
    }

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

    // Update the velocity of the mob based on angle, the mob's speed ratio, and the terrain's speed ratio
    Mob& mob_mob = registry.mobs.get(mob);
    float mob_speed_ratio = mob_mob.speed_ratio;
    float terrain_speed_ratio = terrain->get_terrain_speed_ratio(prev_cell);
    mob_motion.velocity[0] = cos(angle) * mob_speed_ratio * terrain_speed_ratio;
    mob_motion.velocity[1] = sin(angle) * mob_speed_ratio * terrain_speed_ratio;

    // If mob is slowed by a weapon, apply the slow effect to the velocity
    if (registry.mobSlowEffects.has(mob)) {
        MobSlowEffect& mob_mobSlowEffect = registry.mobSlowEffects.get(mob);
        float mob_slow_ratio = mob_mobSlowEffect.slow_ratio;

        if (mob_mobSlowEffect.applied) {
            mob_mobSlowEffect.initial_velocity = mob_motion.velocity;
            mob_motion.velocity = mob_motion.velocity * mob_slow_ratio;
        }
    }
};