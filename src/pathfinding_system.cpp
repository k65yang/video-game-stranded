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
    return std::stack<Entity>();
};


void PathfindingSystem::BFS(Entity player_cell, Entity mob_cell, Entity predecessor[])
{
    
};

bool PathfindingSystem::same_cell(Entity player, Entity mob)
{
    return true;
};


bool PathfindingSystem::reached_next_cell(Entity mob)
{
    return true;
};


void PathfindingSystem::update_velocity_to_next_cell(Entity mob, float elapsed_ms)
{
    
};