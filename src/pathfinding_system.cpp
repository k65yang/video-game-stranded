#include "pathfinding_system.hpp"

void PathfindingSystem::init(TerrainSystem* terrain_arg)
{
    this->terrain = terrain_arg;
};
    
void PathfindingSystem::step(float elapsed_ms) 
{
    // TODO: better way to get player?
    Entity player = registry.players.entities[0]; 

    // printf("Player: %d\n", player);

    for (Entity entity : registry.mobs.entities) {
        Mob& mob = registry.mobs.get(entity);

        // printf("Mob: %d\n", entity);
        // printf("is_tracking_player: %d\n", mob.is_tracking_player);

        // Find new path from mob to player if mob is not tracking the player yet
        if (!mob.is_tracking_player) {
            mob.is_tracking_player = true;
            std::stack<Entity> new_path = find_shortest_path(player, entity);
            Path& mob_path = registry.paths.get(entity);
            mob_path.path = new_path;
        }

        // Update velocity of mob if they are tracking the player and reached the next cell in their path
        if (mob.is_tracking_player && reached_next_cell(entity)) {
            update_velocity_to_next_cell(entity, elapsed_ms);
        }
    }
};


std::stack<Entity> PathfindingSystem::find_shortest_path(Entity player, Entity mob)
{
    return std::stack<Entity>();
};


void PathfindingSystem::BFS(Entity player_cell, Entity mob_cell, Entity predecessor[])
{
    
};


bool PathfindingSystem::reached_next_cell(Entity mob)
{
    return true;
};


void PathfindingSystem::update_velocity_to_next_cell(Entity mob, float elapsed_ms)
{
    
};