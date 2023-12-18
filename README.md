# Team13-Stranded
One of the games of all time.

For our submission for milestone 1, we were able to successfully complete all features that we had outlined, and in a few cases we did a little bit extra than what was intitially there, such as having some placeholder sprites - like a spaceship. Additionally, we had a lot of our features coded in such a way that they can be extensible, such as items having stubbed code for the 4 different types of items (quest, character upgrade, food, weapons), and a generic creation function for items. In our creative features, two basic components were completed: a fog of war mask to obscure vision, and basic pathfinding for mobs, recalculated every "cell" (internal represenation). Lastly, both mobs and items are spawned randomly within the map's boundaries.

## WHERE ARE OUR FEATURES LOCATED:

Milestone 1

<b>Key-frame/state interpolation (health and food bars):</b> world_system.cpp, "interpolate", and in "step", where we tick down the timers and call the interpolate function. <br>

<b>keyboard/mouse controls:</b> world_system.cpp, "handle_movement", "key_to_index", "update_key_presses"<br>
<b>Random/coded action:</b> world_system.cpp, "get_random_spawn_location", "is_spawn_location_used", "spawn_items", "spawn_mobs" <br>
<b>Well-defined game-space boundaries:</b> world_system.cpp, "createBoxBoundary"<br>
<b>Simple collision detection & resolution:</b> physics_system.cpp; world_system.cpp, "handle_collision"<br>

<b>Pathfinding with BFS:</b> pathfinding_system.cpp <br>
<b>Camera controls:</b> world_system.cpp, "handle_movement", "update_camera_follow" <br>


<br/>
<br/>

Milestone 2

<b>User input logic response on left click:</b> "WorldSystem::on_mouse_click", "WeaponsSystem::fireWeapon" <br>

<b>Sprite sheet animation:</b> sprite_sheet.vs.glsl, sprite_sheet.fs.glsl, "WorldSystem::step", "drawTexturedMesh" in render_system.cpp <br>

<b>New integrated assets:</b> data/textures, data/meshes <br>

<b>Basic user tutorial:</b>  "WorldSystem::step", "createHelp" in world_init.cpp <br>

<b>Mesh based collision:</b>"createMeshCollider" in world_init.cpp , "SATcollides" in physic system.cpp<br>

<b>Pathfinding with A*:</b> "A_star" in pathfinding_system.cpp <br>

<br/>
<br/>

Milestone 3

<b>Precise collision w/ bounding volume hierarchy</b>"PhysicsSystem::buildBVH()"<br>

<b>Audio feedback</b> audio_system.hpp, audio_system.cpp <br>

<b>Reloadability</b> save.cpp, save.hpp <br>

<b>Consistent game resolution</b> "aspect_ratio" , "targetResolution" in common.hpp, "RenderSystem::createProjectionMatrix()", "WorldSystem::create_window()"

<b></b> <br>
## How to start
Build instructions are exactly the same as A1/A2. The .exe is even named salmon.exe! Make sure to regenerate the CMake cache if you are having build errors.

<b></b> <br>
### Image Credits

_Only assets taken from the web are credited here. If the image is not mentioned, then we made it ourselves._

Player upgrade - Healing Icon: https://www.flaticon.com/free-icon/regeneration_4490839?term=healing&page=1&position=8&origin=search&related_id=4490839

Player upgrade - Speed Icon: https://www.flaticon.com/free-icon/speed_1409012?related_id=1409083&origin=search

Player upgrade - Invisible Icon: https://www.flaticon.com/free-icon/speed_1409012?related_id=1409083&origin=search

Shuriken Weapon Icon: ???

Crossbow Weapon Icon: https://www.flaticon.com/free-icon/crossbow_486976

Shotgun Weapon Icon: https://simon-develop.itch.io/shotgun 

Machine Gun Weapon Icon: https://pixabay.com/vectors/kalashnikov-rifle-russian-weapon-155147/

Start Button: https://www.flaticon.com/free-icon/start-button_5261929?related_id=5261874&origin=search 

Muzzle flash: https://opengameart.org/content/16-toon-muzzle-flash


