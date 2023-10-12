#pragma once
#include <queue>
#include <vector>
#include <deque>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "terrain_system.hpp"

// A pathfinding system for mobs
class PathfindingSystem
{
public:
	PathfindingSystem() {};

	/// <summary>
	/// Initializes path finding system
	/// </summary>
	/// <param name="terrain_arg">Pointer to the terrain system</param>
	void init(TerrainSystem* terrain_arg);

	/// <summary>
	/// Steps pathfinding ahead by elapsed_ms milliseconds
	/// </summary>
	/// <param name="elapsed_time">the time (in milliseconds) since the last frame</param>
	void step(float elapsed_ms);

private:
	/// Terrain system pointer
	TerrainSystem* terrain;

    /// <summary>
	/// Finds the shortest path between a mob and the player
    /// Reference: https://www.geeksforgeeks.org/shortest-path-unweighted-graph/
	/// </summary>
	/// <param name="player">The player entity</param>
	/// <param name="mob">The mob entity</param>
    /// <returns>
    /// Returns the shortest path as a deque of Entities which correspond to cells in the world grid. 
    /// Initially, the cell the mob is in is at the front of the deque and the cell the player is in is at the end of 
	/// the deque
    /// </returns>
    std::deque<Entity> find_shortest_path(Entity player, Entity mob);

    /// <summary>
	/// Conducts a BFS over the world grid cells, starting from the cell the mob is in, to find the shortest path 
    /// to the cell the player is in
    /// Reference: https://www.geeksforgeeks.org/shortest-path-unweighted-graph/
	/// </summary>
	/// <param name="player_cell">The cell the player is in</param>
	/// <param name="mob_cell">The cell the mob is in</param>
	/// <param name="predecessor">
    /// An array of ints which correspond to indices of cells in the world grid. predecessor[i] represents the immediate 
    /// predecessor of the cell at index i in the world grid found during the BFS
    /// </param>
    bool BFS(Entity player_cell, Entity mob_cell, std::vector<int>& predecessor);

    /// <summary>
	/// Checks if the player and a mob are in the same cell
	/// </summary>
	/// <param name="player">The player entity</param>
	/// <param name="mob">The mob entity</param>
    /// <returns>Returns true if the player and mob are in the same cell, false otherwise</returns>
    bool same_cell(Entity player, Entity mob);

    /// <summary>
	/// Checks if the mob has reached the next cell in its path. A mob has reached the next cell in its path if it
	/// is close to the center of that cell
	/// </summary>
	/// <param name="mob">The mob entity to check</param>
    /// <returns>Returns true if the mob has reached its next cell, false otherwise</returns>
    bool reached_next_cell(Entity mob);

    /// <summary>
	/// Checks if the player has moved from the cell the mob believes the player is in
	/// </summary>
	/// <param name="player">The player entity</param>
	/// <param name="mob">The mob entity</param>
    /// <returns>Returns true if the player has moved, false otherwise</returns>
	bool has_player_moved(Entity player, Entity mob);

    /// <summary>
	/// Updates a mob’s velocity to move to the next cell in its path
	/// </summary>
	/// <param name="mob">The mob entity to whose velocity should be updated</param>
	/// <param name="elapsed_ms">The time (in milliseconds) since the last frame</param>
    void update_velocity_to_next_cell(Entity mob, float elapsed_ms);
};