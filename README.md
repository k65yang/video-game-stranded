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

#### Milestone 4

<b>Comprehensive tutorial</b>"PhysicsSystem::buildBVH()"<br>

<b>Optimize user interaction and REPORT it</b> See below. <br>

<b>Game Balance</b> See below. <br>

<b>External Integration</b> "freetype" in /ext , "renderText" in render_system.cpp; "initializeTextRenderer()" in render_system_init.cpp

<b>Simple Rendering Effects</b> fog.vs.glsl, fog.fs.glsl in /shaders; drawToScreen() in render_system.cpp

<b>Particle System</b> particle_system.cpp/hpp, RenderSystem::drawParticles() in render_system.cpp

<b>Story elements</b> start_screen_system.cpp/hpp, "WorldSystem::update_spaceship_depart" in world_system.cpp

<b>(Numerous?) In-House Integrated Assets</b> All weapon textures including ammo and icons, "/data/textures/mob_spritesheet.png", "/data/textures/player_spritesheet.png", All sound effects in "/data/audio" are mixed and combined in-house using external sound sample libraries.

### Game Balancing and User Interaction Report

The following table lists some changes that we made in response to cross play comments in effort to better balance the game.

| Comment | Game Change | Type |
| --------------- | --------------- | ------ |
| "Only other thing I would maybe change is making the game slightly more difficult. I think there is food everywhere and the monsters don't do too much damage"    | - Increased mob spawn rate </br> -Slightly increased damage from all mobs.  | Game Balance |
| "more clarity with the quest would be nice, it was pretty hard to find the quest items"    | - Added a screen in the beginning of the game to explain what the player needs to do </br>- Updated quest item sprites so they are more easily seen </br> - Updated tutorial system to tell player what they need to do once they get a quest item | User Interaction |
| "No visual indicator when enemies take damage/Damaging enemies is a bit unclear." | - Added new particles when you hit an enemy</br> - Added mob health bars | User Interaction |
| "A bit hard to tell if you shot your bullet or if it's currently reloading." | - Added gunshot noises for successful shots</br> - Added dry firing (shooting without ammo) sound effects | User Interaction |
| "I think if there were other reasons to return to the ship that could be cool too"    | - Added the condition where the player must return to the ship to hand in their quest items </br> - To beat the game, the player must return to the ship (once they have all quest items) and they can see their ship blast off    | Game Balance / User Interaction |
| "I think it's a bit too easy to run around enemies, since they can't catch up to you"    | - Increased enemy speed </br> - Increase aggro range of enemies (they start tracking you from further away so there is a bigger mob!) </br> - Updated a mob type to be terrain agnostic (will not be slowed by terrain effects and can go through walls!)    | Game Balance |
| "Food drains quite fast so you have to keep an eye on it"    | - Greatly increased food spawns    | Game Balance |
| "It would be more fun to be able to understand the map and fighting capabilities a little better"    | - Updated the player UI to be more explicit on the weapons the player has </br> - Added additional tutorial popups for controls    | User Interaction |
| "Tutorial is a bit too fast. I found \[the game] starts abruptly and it was difficult to read everything." | - Game is now paused until you close the tutorial window | User Interaction|


The following table lists some of the balance and user experience issues the developers ran into during testing and how it was fixed.

| Issue | Game Change | Type | 
| --------------- | --------------- | -------- |
| Ever since we made the map 196x196, it is very easy to get lost and run out of food and die    | - Added blue arrow pointing back to the spaceship, so player always knows the way "home"    | User Interaction |
| The shotgun is too OP; there is no incentive to use any other gun    | - Reduced shotgun ammo and damage </br> - Increase machine gun damage   | Game Balance |
| The player would spawn with no weapons and must pick them up before using it. This causes them to be defenseless from attacking mobs while looking for a weapon.    | - Revamped the entire weapons system so that the player actually starts with all weapons. </br> - There was additional balance issues where the player is too strong early game, so we only gave the player ammo for one weapon (the other weapons have no ammo)   | Game Balance |
| It may be very overwhelming for the player to worry about both health and food.    | - Added two powerups: a speed powerup (doubles speed without consuming more food) and health powerup (out of combat healing) so players can worry less about either their health or food. </br> - Also added audio cues when you take damage, when your health is low, and when you're dead   | Game Balance |
| It's sometimes hard to keep an eye on your ammo bar when you're in a fight. Someones you don't really know when you've run out of ammo | - Added ammo pickup sounds</br>- Added dry firing sound when you're out of ammo. It makes running out of ammo during a fight both terrifying and funny. | User Interaction |


<b></b> <br>
## How to start
Build instructions are exactly the same as A1/A2. The .exe is even named salmon.exe! Make sure to regenerate the CMake cache if you are having build errors.
