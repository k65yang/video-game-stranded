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
    std::vector<Entity> predecessor;

    // Execute BFS
    if (!BFS(player_cell, mob_cell, predecessor)) {
        printf("Path from mob in cell %d to player in cell %d could not be found", mob_cell, player_cell);
        assert(false);
    }

    // Get shortest path by backtracking through predecessors
    std::stack<Entity> path;
    int crawl = terrain->get_cell_index(player_cell);
    while (predecessor.at(crawl) != -1) {
        path.push(predecessor.at(crawl));
        crawl = terrain->get_cell_index(predecessor.at(crawl));
    }

    return path;
};


bool PathfindingSystem::BFS(Entity player_cell, Entity mob_cell, std::vector<Entity> predecessor)
{
    
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