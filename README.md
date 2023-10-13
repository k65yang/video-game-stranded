# Team13-Stranded
One of the games of all time.


For our submission for milestone 1, we were able to successfully complete all features that we had outlined, and in a few cases we did a little bit extra than what was intitially there, such as having some placeholder sprites - like a spaceship. Additionally, we had a lot of our features coded in such a way that they can be extensible, such as items having stubbed code for the 4 different types of items (quest, character upgrade, food, weapons), and a generic creation function for items. In our creative features, two basic components were completed: a fog of war mask to obscure vision, and basic pathfinding for mobs, recalculated every "cell" (internal represenation). Lastly, both mobs and items are spawned randomly within the map's boundaries.


WHERE ARE OUR FEATURES LOCATED:
interpolation/key-frame animation (health and food bars): world_system.cpp, "interpolate", and in "step", where we tick down the timers and call the interpolate function.
pathfinding with BFS: pathfinding_system.cpp
camera controls: world_system.cpp, "handle_movement", "update_camera_follow"
random/coded action: world_system.cpp, "get_random_spawn_location", "is_spawn_location_used", "spawn_items", "spawn_mobs"
game-space boundaries: ?? TODO
