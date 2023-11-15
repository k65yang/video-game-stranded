#pragma once
#include <queue>
#include <vector>
#include <deque>
#include <functional>
#include <utility>
#include <unordered_map>

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
	/// Performs A* over the world grid cells, starting from the cell the mob is in, to find the shortest path to the 
	/// cell the player is in. Since euclidean distance is used as the heuristic, the heuristic is admissible and A* 
	/// will always find the shortest path. 
	/// </summary>
	/// <param name="player_cell">The cell the player is in</param>
	/// <param name="mob_cell">The cell the mob is in</param>
	/// <param name="predecessor">
    /// A map of key, value pairs where the key corresponds to an index of a cell in the world grid and the value 
	/// represents the index of the immediate predecessor of the cell found during A*
    /// </param>
    bool A_star(Entity player_cell, Entity mob_cell, std::unordered_map<int, int>& predecessor);

	/// <summary>
	/// Conducts a BFS over the world grid cells, starting from the cell the mob is in, to find the shortest path 
    /// to the cell the player is in
    /// Reference: https://www.geeksforgeeks.org/shortest-path-unweighted-graph/
	/// </summary>
	/// <param name="player_cell">The cell the player is in</param>
	/// <param name="mob_cell">The cell the mob is in</param>
	/// <param name="predecessor">
    /// A map of key, value pairs where the key corresponds to an index of a cell in the world grid and the value 
	/// represents the index of the immediate predecessor of the cell found during BFS
    /// </param>
	bool BFS(Entity player_cell, Entity mob_cell, std::unordered_map<int, int>& predecessor);

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
	/// Checks if the player has moved from the cell the mob believes the player is in.
	/// A player has moved if the cell they are in is different from the cell the mob believes the player is in
	/// </summary>
	/// <param name="player">The player entity</param>
	/// <param name="mob">The mob entity</param>
    /// <returns>Returns true if the player has moved, false otherwise</returns>
	bool has_player_moved(Entity player, Entity mob);

	/// <summary>
	/// Stops a mob from tracking the player
	/// </summary>
	/// <param name="mob">The mob entity</param>
    void stop_tracking_player(Entity mob);

	/// <summary>
	/// Checks if the player is in aggro range of the mob
	/// </summary>
	/// <param name="player">The player entity</param>
	/// <param name="mob">The mob entity</param>
    /// <returns>Returns true if the player is in aggro range of the player, false otherwise</returns>
    bool is_player_in_mob_aggro_range(Entity player, Entity mob);

	/// <summary>
	/// Checks if a mob has entered a new cell
	/// </summary>
	/// <param name="mob">The mob entity</param>
    /// <returns>Returns true if the mob has entered a new cell, false otherwise</returns>
    bool entered_new_cell(Entity mob);

	/// <summary>
	/// Removes the terrain speed effect of the previous cell the mob was in and applies the terrain speed effect of the 
	/// new cell the mob is in to the mob’s velocity
	/// </summary>
	/// <param name="mob">The mob entity</param>
	/// <param name="prev_cell">The previous cell the mob was in</param>
	/// <param name="new_cell">The new cell the mob is in</param>
    /// <returns>Returns the terrain speed ratio of the cell</returns>
    void apply_new_terrain_speed_effect(Entity mob, Entity prev_cell, Entity new_cell);

    /// <summary>
	/// Updates a mob’s velocity to move to the next cell in its path
	/// </summary>
	/// <param name="mob">The mob entity to whose velocity should be updated</param>
	/// <param name="elapsed_ms">The time (in milliseconds) since the last frame</param>
    void update_velocity_to_next_cell(Entity mob, float elapsed_ms);
};