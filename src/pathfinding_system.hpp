#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "terrain_system.hpp"

// A pathfinding system for mobs
class PathfindingSystem
{
public:
	PathfindingSystem() {}

	/// <summary>
	/// Steps pathfinding ahead by elapsed_ms milliseconds
	/// </summary>
	/// <param name="elapsed_time">the time (in milliseconds) since the last frame</param>
	void step(float elapsed_ms);

private:
    /// <summary>
	/// Finds the shortest path between a mob and the player
    /// Reference: https://www.geeksforgeeks.org/shortest-path-unweighted-graph/
	/// </summary>
	/// <param name="player">The player entity</param>
	/// <param name="mob">The mob entity</param>
    /// <returns>
    /// Returns the shortest path as a stack of Entities which correspond to cells in the world grid. 
    /// The cell the mob is in at the top of the stack and the cell the player is in at the bottom of the stack
    /// </returns>
    std::stack<Entity> find_shortest_path(Entity player, Entity mob);

    /// <summary>
	/// Conducts a BFS over the world grid cells, starting from the cell the mob is in, to find the shortest path 
    /// to the cell the player is in
    /// Reference: https://www.geeksforgeeks.org/shortest-path-unweighted-graph/
	/// </summary>
	/// <param name="player_cell">The cell the player is in</param>
	/// <param name="mob_cell">The cell the mob is in</param>
	/// <param name="predecessor">
    /// An array of Entities which correspond to cells in the world grid. predecessor[i] represents the immediate 
    /// predecessor of the cell at index i in the world grid found during the BFS
    /// </param>
    void BFS(Entity player_cell, Entity mob_cell, Entity predecessor[]);

    /// <summary>
	/// Checks if the mob has reached the next cell in its path
	/// </summary>
	/// <param name="mob">The mob entity to check</param>
    /// <returns>Returns true if the mob has reached its next cell, false otherwise</returns>
    bool reachedNextCell(Entity mob);

    /// <summary>
	/// Updates a mob’s velocity to move to the next cell in its path
	/// </summary>
	/// <param name="mob">The mob entity to whose velocity should be updated</param>
	/// <param name="elapsed_ms">The time (in milliseconds) since the last frame</param>
    void updateVelocityToNextCell(Entity mob, float elapsed_ms);
};