#include "pathfinding_system.hpp"

float ELAPSED = 0;
int MOB_DIRECTION = 1;  // Default to facing up
int UP = 0; 
int DOWN = 1; 
int LEFT = 2;
int RIGHT = 3; 
int DIRECTION_CHANGE = 0;
const int MAX_NUM_CELLS_TO_SEARCH = 200;

void PathfindingSystem::init(TerrainSystem* terrain_arg, PowerupSystem* powerup_system_arg)
{
    this->terrain = terrain_arg;
    this->powerup_system = powerup_system_arg;
};
    
void PathfindingSystem::step(float elapsed_ms) 
{
    Entity player = registry.players.entities[0]; 

    for (Entity mob : registry.mobs.entities) {
        Mob& mob_mob = registry.mobs.get(mob);
      
       

        // Stop mob from tracking the player if mob is tracking the player and:
        // 1) player is not in the aggro range of the mob, or
        // 2) mob is in the same cell as the player
        // 3) invisible powerup is activate
        if (powerup_system->disable_pathfinding_invisible_powerup || (mob_mob.is_tracking_player && (!is_player_in_mob_aggro_range(player, mob) || same_cell(player, mob)))) {
            stop_tracking_player(mob);
        }

        // Find new path from mob to player if:
        // 1) mob is not tracking the player and player is within aggro range of mob and mob is not in the same cell as the player already, or
        // 2) mob is tracking the player and the player has moved
        // 3) and invisible powerup is not activated
        if (!powerup_system->disable_pathfinding_invisible_powerup && ((!mob_mob.is_tracking_player && is_player_in_mob_aggro_range(player, mob) && !same_cell(player, mob)) ||
            (mob_mob.is_tracking_player && has_player_moved(player, mob))
            )) {
            if (!mob_mob.is_tracking_player) {
                mob_mob.is_tracking_player = true;
            }

            std::deque<Entity> new_path = find_shortest_path(player, mob);
            Path& mob_path = registry.paths.get(mob);
            mob_path.path = new_path;

            update_velocity_to_next_cell(mob, elapsed_ms);
        }
        
        

        // Get the previous location of the mob 
        float prev_loc_x = registry.motions.get(mob).position[0];
        float prev_loc_y = registry.motions.get(mob).position[1];

        

        // Apply new terrain speed effect if the mob enters a new cell
        if (entered_new_cell(mob) && mob_mob.type != MOB_TYPE::GHOST) {
            // Get the cell the mob was previously in and the new cell the mob is in
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

        

        if (registry.animations.has(mob)) {
            auto& animation = registry.animations.get(mob);

            // Update prev_mframex for the next step
            int prev_mframex = DIRECTION_CHANGE;

            // Adjust this for mob animation speed
            ELAPSED += elapsed_ms;

            // Get the current location of the mob
            float curr_loc_x = registry.motions.get(mob).position[0];
            float curr_loc_y = registry.motions.get(mob).position[1];

            float dx = curr_loc_x - prev_loc_x;
            float dy = curr_loc_y - prev_loc_y;

            // Check Mobs movement and update mob direction 
            if (dx > 0 && dy == 0)
                MOB_DIRECTION = RIGHT; // Mob is moving to the right
            else if (dx < 0 && dy == 0)
                MOB_DIRECTION = LEFT;// Mob is moving to the left
            else if (dy > 0 && dx == 0)
                MOB_DIRECTION = DOWN;// Mob is moving down
            else if (dy < 0 && dx == 0)
                MOB_DIRECTION = UP; // Mob is moving up

            if (dx > 0 && dy > 0)
                MOB_DIRECTION = RIGHT;// Mob is moving down and to the right
            else if (dx > 0 && dy < 0)
                MOB_DIRECTION = UP;// Mob is moving up and to the right
            else if (dx < 0 && dy > 0)
                MOB_DIRECTION = LEFT;// Mob is moving down and to the left
            else if (dx < 0 && dy < 0)
                MOB_DIRECTION = UP;// Mob is moving up and to the left

            // Calculate the direction based on the change in positions
            if (prev_loc_x < curr_loc_x)
                DIRECTION_CHANGE = 1;// Mob moved right
            else if (prev_loc_x > curr_loc_x)
                DIRECTION_CHANGE = 2; // Mob moved left
            else if (prev_loc_y < curr_loc_y)
                DIRECTION_CHANGE = 3;// Mob moved down
            else if (prev_loc_y > curr_loc_y)
                DIRECTION_CHANGE = 4;// Mob moved up


            // Check for a change in direction
            if (DIRECTION_CHANGE != prev_mframex) {
                // Direction changed, so reset frame x
                DIRECTION_CHANGE = 0;
                animation.framex = 0;
            }


            // Update mobs's direction
            animation.framey = MOB_DIRECTION;

            if (ELAPSED > 50) {
                // Update walking animation
                animation.framex = (animation.framex + 1) % 7;
                ELAPSED = 0.0f; // Reset the timer
            }
        }

        

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
    // predecessor is a map of key, value pairs where the key corresponds to an index of a cell in the world grid 
    // and the value represents the index of the immediate predecessor of the cell found during pathfinding
    std::unordered_map<int, int> predecessor;

    // Check what kind of mob it is, for example ghosts don't use A* since they can go over collidable cells
    Mob& mob_mob = registry.mobs.get(mob);
    if (mob_mob.type == MOB_TYPE::GHOST) {
        if (!BFS(player_cell, mob_cell, predecessor)) {
            printf("BFS PATH from mob in cell %d to player in cell %d could not be found\n", mob_cell, player_cell);
            assert(false);
        }
    } else {
        if (!A_star(player_cell, mob_cell, predecessor)) {
            printf("A* PATH from mob in cell %d to player in cell %d could not be found\n", mob_cell, player_cell);
            assert(false);
        }
    }

    // Get shortest path by backtracking through predecessors
    std::deque<Entity> path;
    path.push_front(player_cell);
    int index = terrain->get_cell_index(player_cell);
    while (predecessor.count(index) == 1) {
        path.push_front(terrain->get_cell(predecessor[index]));
        index = predecessor[index];
    }

    return path;
};

bool PathfindingSystem::BFS(Entity player_cell, Entity mob_cell, std::unordered_map<int, int>& predecessor)
{
    // Accumulator for number of cells searched
    int num_cells_searched = 0;

    // Initialize queue for BFS
    std::queue<Entity> bfs_queue;

    // Initialize visited array for BFS
    // visited is a map of key, value pairs where the key corresponds to the index of a cell in the world grid
    // and the value indicates whether the cell has been visited during the BFS
    std::unordered_map<int, bool> visited;

    // Start BFS from mob cell so mark it as visited and add to BFS queue
    visited[terrain->get_cell_index(mob_cell)] = true;
    bfs_queue.push(mob_cell);

    // BFS algorithm
    while (!bfs_queue.empty()) {
        // Stop search if going for too long
        if (num_cells_searched++ > MAX_NUM_CELLS_TO_SEARCH) {
            return true;
        }

        // Get and remove first cell from queue
        Entity curr = bfs_queue.front();
        int curr_cell_index = terrain->get_cell_index(curr);
        bfs_queue.pop();

        // Get neighbors of cell
        std::vector<Entity> neighbors;
        terrain->get_accessible_neighbours(curr, neighbors, true); // BFS is only used by ghost, so we can ignore colliders here

        for (Entity neighbor : neighbors) {
            int neighbor_cell_index = terrain->get_cell_index(neighbor);

            // Set cell as visited, save its predecessor, and add it to the BFS queue if cell has not been visited yet
            if (visited.count(neighbor_cell_index) == 0) {
                visited[neighbor_cell_index] = true;
                predecessor[neighbor_cell_index] = curr_cell_index;
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

bool PathfindingSystem::A_star(Entity player_cell, Entity mob_cell, std::unordered_map<int, int>& predecessor)
{
    // Limit number of cells to search
    int num_cells_searched = 0;

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
    // closed is a map of key, value pairs where the key corresponds to the index of a cell in the world grid
    // and the value indicates whether the cell has been processed during A*
    std::unordered_map<int, bool> closed;

    // Initialize g array for A*
    // g is a map of key, value pairs where the key corresponds to the index of a cell in the world grid
    // and the value indicates the cost to move from the mob cell (i.e. the starting cell) to the cell
    std::unordered_map<int, float> g;

    // Start A* from the mob cell so add it to open and set its g to 0
    int mob_cell_index = terrain->get_cell_index(mob_cell);
    open.push(std::make_pair(0, mob_cell_index));
    g[mob_cell_index] = 0;

    // A* algorithm
    while (!open.empty()) {
        // Stop search if going for too long
        if (num_cells_searched++ > MAX_NUM_CELLS_TO_SEARCH) {
            return true;
        }

        // Get and remove top cell (i.e. cell with min total cost) from open
        std::pair<float, int> p = open.top();
        open.pop();
        int curr_cell_index = p.second;
        Entity curr = terrain->get_cell(curr_cell_index);

        // Set cell to closed
        closed[curr_cell_index] = true;

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
            if (closed.count(neighbor_cell_index) == 1) {
                continue;
            }

            // Calculate costs
            Motion& neighbor_motion = registry.motions.get(neighbor);
            float neighbor_g = g[curr_cell_index] + (1.0f * (1/terrain->get_terrain_speed_ratio(neighbor)));
            float neighbor_h = distance(neighbor_motion.position, player_cell_motion.position);
            float neighbor_f = neighbor_g + neighbor_h;

            // Do not add neighbor to open if it is already in open and came from a path with lower cost
            if (g.count(neighbor_cell_index) == 1 && g[neighbor_cell_index] < neighbor_g) {
                continue;
            }

            // Add neighbor to open and set predecessor and g
            open.push(std::make_pair(neighbor_f, neighbor_cell_index));
            predecessor[neighbor_cell_index] = curr_cell_index;
            g[neighbor_cell_index] = neighbor_g;
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
    Entity curr_cell = terrain->get_cell(mob_motion.position);
    Path& mob_path = registry.paths.get(mob);
    Entity next_cell = mob_path.path.front();

    // Check if mob's current cell is the next cell
    return curr_cell == next_cell;
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
    // Get the motion, mob, path, and current cell of the mob
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
    Path& mob_path = registry.paths.get(mob);
    Motion& mob_motion = registry.motions.get(mob);

    // Stop mob from tracking the player if it has reached the last cell in its path
    if (mob_path.path.size() <= 1) {
        stop_tracking_player(mob);
        return;
    }

    // Get and remove previous cell in the path
    Entity prev_cell = mob_path.path.front();
    mob_path.path.pop_front();

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