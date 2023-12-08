# Team13-Stranded
One of the games of all time.

For our submission for milestone 1, we were able to successfully complete all features that we had outlined, and in a few cases we did a little bit extra than what was intitially there, such as having some placeholder sprites - like a spaceship. Additionally, we had a lot of our features coded in such a way that they can be extensible, such as items having stubbed code for the 4 different types of items (quest, character upgrade, food, weapons), and a generic creation function for items. In our creative features, two basic components were completed: a fog of war mask to obscure vision, and basic pathfinding for mobs, recalculated every "cell" (internal represenation). Lastly, both mobs and items are spawned randomly within the map's boundaries.

## WHERE ARE OUR FEATURES LOCATED:

#### Milestone 1

<b>Key-frame/state interpolation (health and food bars):</b> world_system.cpp, "interpolate", and in "step", where we tick down the timers and call the interpolate function. <br>

<b>keyboard/mouse controls:</b> world_system.cpp, "handle_movement", "key_to_index", "update_key_presses"<br>
<b>Random/coded action:</b> world_system.cpp, "get_random_spawn_location", "is_spawn_location_used", "spawn_items", "spawn_mobs" <br>
<b>Well-defined game-space boundaries:</b> world_system.cpp, "createBoxBoundary"<br>
<b>Simple collision detection & resolution:</b> physics_system.cpp; world_system.cpp, "handle_collision"<br>

<b>Pathfinding with BFS:</b> pathfinding_system.cpp <br>
<b>Camera controls:</b> world_system.cpp, "handle_movement", "update_camera_follow" <br>


<br/>
<br/>

#### Milestone 2

<b>User input logic response on left click:</b> "WorldSystem::on_mouse_click", "WeaponsSystem::fireWeapon" <br>

<b>Sprite sheet animation:</b> sprite_sheet.vs.glsl, sprite_sheet.fs.glsl, "WorldSystem::step", "drawTexturedMesh" in render_system.cpp <br>

<b>New integrated assets:</b> data/textures, data/meshes <br>

<b>Basic user tutorial:</b>  "WorldSystem::step", "createHelp" in world_init.cpp <br>

<b>Mesh based collision:</b>"createMeshCollider" in world_init.cpp , "SATcollides" in physic system.cpp<br>

<b>Pathfinding with A*:</b> "A_star" in pathfinding_system.cpp <br>

<br/>
<br/>

#### Milestone 3

<b>Precise collision w/ bounding volume hierarchy</b>"PhysicsSystem::buildBVH()"<br>

<b>Audio feedback</b> audio_system.hpp, audio_system.cpp <br>

<b>Reloadability</b> save.cpp, save.hpp <br>

<b>Consistent game resolution</b> "aspect_ratio" , "targetResolution" in common.hpp, "RenderSystem::createProjectionMatrix()", "WorldSystem::create_window()"


### Game Balancing

The following table lists some changes that we made in response to cross play comments in effort to better balance the game.

| Comment | Game Change |
| --------------- | --------------- |
| "Only other thing I would maybe change is making the game slightly more difficult. I think there is food everywhere and the monsters don't do too much damage"    | - Increased mob spawn rate </br> -Slightly increased damage from all mobs.  |
| "more clarity with the quest would be nice, it was pretty hard to find the quest items"    | - Added a screen in the beginning of the game to explain what the player needs to do </br>- Updated quest item sprites so they are more easily seen </br> - Updated tutorial system to tell player what they need to do once they get a quest item |
| "I think if there were other reasons to return to the ship that could be cool too"    | - Added the condition where the player must return to the ship to hand in their quest items </br> - To beat the game, the player must return to the ship (once they have all quest items) and they can see their ship blast off    |
| "I think it's a bit too easy to run around enemies, since they can't catch up to you"    | - Increased enemy speed </br> - Increase aggro range of enemies (they start tracking you from further away so there is a bigger mob!) </br> - Updated a mob type to be terrain agnostic (will not be slowed by terrain effects and can go through walls!)    |
| "Food drains quite fast so you have to keep an eye on it"    | - Greatly increased food spawns    |
| "It would be more fun to be able to understand the map and fighting capabilities a little better"    | - Updated the player UI to be more explicit on the weapons the player has </br> - Added additional tutorial popups for controls    |

The following table lists some of the balance issues the developers ran into during testing and how it was fixed.

| Issue | Game Change |
| --------------- | --------------- |
| Ever since we made the map 196x196, it is very easy to get lost and run out of food and die    | - Added blue arrow pointing back to the spaceship, so player always knows the way "home"    |
| The shotgun is too OP; there is no incentive to use any other gun    | - Reduced shotgun ammo and damage </br> - Increase machine gun damage   |
| The player would spawn with no weapons and must pick them up before using it. This causes them to be defenseless from attacking mobs while looking for a weapon.    | - Revamped the entire weapons system so that the player actually starts with all weapons. </br> - There was additional balance issues where the player is too strong early game, so we only gave the player ammo for one weapon (the other weapons have no ammo)   |
| It may be very overwhelming for the player to worry about both health and food.    | - Added two powerups: a speed powerup (doubles speed without consuming more food) and health powerup (out of combat healing) so players can worry less about either their health or food.    |
| Row 3, Col 1    | Row 3, Col 2    |

<b></b> <br>
## How to start
Build instructions are exactly the same as A1/A2. The .exe is even named salmon.exe! Make sure to regenerate the CMake cache if you are having build errors.
