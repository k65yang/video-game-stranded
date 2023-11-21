// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include "physics_system.hpp"

// Game configuration
const float IFRAMES = 1500;
const int FOOD_PICKUP_AMOUNT = 20;
float PLAYER_TOTAL_DISTANCE = 0;
const float FOOD_DECREASE_THRESHOLD  = 5.0f; // Adjust this value as needed
const float FOOD_DECREASE_RATE = 10.f;	// Decreases by 10 units per second (when moving)
float CURSOR_ANGLE = 0;
int PLAYER_DIRECTION = 4;  // Default to facing up
float ELAPSED_TIME = 0;


// Create the fish world
WorldSystem::WorldSystem()
	: points(0)
	, next_turtle_spawn(0.f)
	, next_fish_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	}

WorldSystem::~WorldSystem() {
	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// TODO: make a more elegant solution via manipulating the projection matrix instead of this hack
	// so UI elements are aspect ratio and resolution independent in theory
	// Build window size using screen dimensions
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int y = mode->height;
	int s = y / aspect_ratio.y;	// scale factor such that s * aspect_ratio = {mode->width, mode->height}. 
	int x = aspect_ratio.x * s;
	// Remember: aspect_ratio.y * s = y, aspect_ratio.x * s = x

	GLFWmonitor* monitor_ptr = glfwGetPrimaryMonitor();

	if (windowed_mode) {
		monitor_ptr = nullptr;
		x = target_resolution.x;
		y = target_resolution.y;
	}

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(x, y, "Stranded", monitor_ptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto cursor_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_button_redirect);

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, TerrainSystem* terrain_arg, WeaponsSystem* weapons_system_arg, PhysicsSystem* physics_system_arg, MobSystem* mob_system_arg, AudioSystem* audio_system_arg) {
	this->renderer = renderer_arg;
	this->terrain = terrain_arg;
	this->weapons_system = weapons_system_arg;
	this->mob_system = mob_system_arg;
	this->physics_system = physics_system_arg;
	this->audio_system = audio_system_arg;

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	Player& player = registry.players.get(player_salmon);
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << " Food: " << player.food << "  HP: " << player.health;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motion_container = registry.motions;

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	float min_timer_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < min_timer_ms) {
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.screen_darken_factor = 0;
			restart_game();
			return true;
		}
	}

	// TODO: CLEAN UP THESE LOOPS they are surely not optimized
	// Tick down tooltip timer
	for (Entity entity : registry.tips.entities) {
		ToolTip& tip = registry.tips.get(entity);
		tip.timer -= elapsed_ms_since_last_update;
		
		// TODO: Make the tooltip fade after it has been a certain period of time (when we have opacity)

		if (tip.timer < 0) {
			registry.renderRequests.remove(entity);
			registry.tips.remove(entity);
		}
	}

	if (current_tooltip < tooltips.size() && registry.tips.size() == 0 && tooltips_on) {
		help_bar = createHelp(renderer, { 0.f, -7.f }, tooltips[current_tooltip]);
		current_tooltip++;
	}

	// Tick down iframes timer and health decrease timer
	for (Entity entity : registry.players.entities) {
		Player& player = registry.players.get(entity);
		player.iframes_timer -= elapsed_ms_since_last_update;

		if (player.iframes_timer < 0) {
			player.iframes_timer = 0;
		}
		
		if (player.health_decrease_time < 0) {
			player.health_decrease_time = 0;
			Motion& camera_motion = registry.motions.get(main_camera);
			camera_motion.angle = 0.f;
			camera_motion.scale = vec2(1.0, 1.0);
		} 
		
		if (player.food_decrease_time < 0) {
			player.food_decrease_time = 0;
		}

		if (player.health_decrease_time > 0) {
			player.health_decrease_time -= elapsed_ms_since_last_update;

			Motion& health = registry.motions.get(health_bar);
			vec2 new_health_scale = vec2(((float)player.health / (float)PLAYER_MAX_HEALTH) * HEALTH_BAR_SCALE[0], HEALTH_BAR_SCALE[1]);
			health.scale = interpolate(health.scale, new_health_scale, 1 - (player.health_decrease_time / IFRAMES));

			// Screen shake, for feedback to the player that they have been hit.
			int direction = rand() % 8;
			Motion& camera_motion = registry.motions.get(main_camera);
			switch (direction) {
				case 0:
					camera_motion.angle += 0.01;
					break;
				case 1:
					camera_motion.angle -= 0.01;
					break;
				case 2:
					camera_motion.scale += vec2(0.004, 0.004);
					break;
				case 3:
					camera_motion.scale -= vec2(0.004, 0.004);
					break;
			}
		}
		
		if (player.food_decrease_time > 0) {
			player.food_decrease_time -= elapsed_ms_since_last_update;

			Motion& food = registry.motions.get(food_bar);
			vec2 new_food_scale = vec2(((float)player.food / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);
			food.scale = interpolate(food.scale, new_food_scale, 1 - (player.food_decrease_time / IFRAMES));
		}
	}

	// Apply food decreasing the more you travel. 
	if (player.food > 0) {
		if (PLAYER_TOTAL_DISTANCE >= FOOD_DECREASE_THRESHOLD ) {
			// Decrease player's food by 1
			player.food -= 1;
			// Shrink the food bar
			player.food_decrease_time = IFRAMES;

			// Reset the total movement distance
			PLAYER_TOTAL_DISTANCE = 0;
		}
	}
	// else the food is below 0, player dies
	else if (!registry.deathTimers.has(player_salmon)) {
		registry.deathTimers.emplace(player_salmon);
	}

	// reduce window brightness if any of the present players is dying
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	Motion& m = registry.motions.get(player_salmon);
	Motion& f = registry.motions.get(fow);
	f.position = m.position;

	ELAPSED_TIME += elapsed_ms_since_last_update;

	// update spritesheet with aiming direction 
	updatePlayerDirection();

	// Player Movement code, build the velocity resulting from player movement
	//for movement, animation, and distance calculation
	handlePlayerMovement(elapsed_ms_since_last_update);
		
		


	// Camera movement mode
	Camera& c = registry.cameras.get(main_camera);
	Motion& camera_motion = registry.motions.get(main_camera);
	if (c.mode_follow) {
		camera_motion.position = m.position;	// why are the positions inverted???
	}
	else {
		handle_movement(camera_motion, CAMERA_LEFT);
	}
	// UI Movement
	Motion& health = registry.motions.get(health_bar);
	Motion& food = registry.motions.get(food_bar);
	Motion& help = registry.motions.get(help_bar);
	Motion& q1 = registry.motions.get(quest_items[0].first);
	Motion& q2 = registry.motions.get(quest_items[1].first);

	health.position = { -8.f + camera_motion.position.x, 7.f + camera_motion.position.y };
	food.position = { 8.f + camera_motion.position.x, 7.f + camera_motion.position.y };
	help.position = { camera_motion.position.x, -7.f + camera_motion.position.y };
	q1.position = { 10.f + camera_motion.position.x, -2.f + camera_motion.position.y };
	q2.position = { 10.f + camera_motion.position.x, 2.f + camera_motion.position.y };

	if (user_has_first_weapon) {
		Motion& weapon_ui = registry.motions.get(weapon_indicator);
		weapon_ui.position = { -10.f + camera_motion.position.x, -6.f + camera_motion.position.y };
	}

	// Mob updates
	for (Entity entity : registry.mobs.entities) {
		// slow updates
		if (registry.mobSlowEffects.has(entity)) {
			Motion& motion = registry.motions.get(entity);
			MobSlowEffect& mobSlowEffect = registry.mobSlowEffects.get(entity);

			// Apply slow effect if not yet applied
			if (!mobSlowEffect.applied) {
				mobSlowEffect.initial_velocity = motion.velocity;
				motion.velocity = motion.velocity * mobSlowEffect.slow_ratio;
				mobSlowEffect.applied = true;
			}
			
			// Increment duration
			mobSlowEffect.elapsed_slow_time_ms += elapsed_ms_since_last_update;
			// printf("total slow time: %f\n", mobSlowEffect.elapsed_slow_time_ms);
			
			// Return mob to normal speed if slow duration is over. 
			// Remove the MobSlowEffect component only.
			if (mobSlowEffect.duration_ms < mobSlowEffect.elapsed_slow_time_ms) {
				motion.velocity = mobSlowEffect.initial_velocity;
				registry.mobSlowEffects.remove(entity);
			}
		}
	}

	return true;
}
void WorldSystem::updatePlayerDirection() {
	if (CURSOR_ANGLE >= -M_PI / 4 && CURSOR_ANGLE < M_PI / 4) {
		PLAYER_DIRECTION = 2;  // Right
		}
	else if (CURSOR_ANGLE >= M_PI / 4 && CURSOR_ANGLE < 3 * M_PI / 4) {
		PLAYER_DIRECTION = 4;  // Down
		}
	else if (CURSOR_ANGLE >= -3 * M_PI / 4 && CURSOR_ANGLE < -M_PI / 4) {
		PLAYER_DIRECTION = 0;  // Up
		}
	else {
		PLAYER_DIRECTION = 3;  // Left
		}

	// Update player's direction
	registry.players.components[0].framey = PLAYER_DIRECTION;
	
	}

void WorldSystem::handlePlayerMovement(float elapsed_ms_since_last_update) {
	// Check if any movement keys are pressed and if player is not dead
	bool anyMovementKeysPressed = keyDown[LEFT] || keyDown[RIGHT] || keyDown[UP] || keyDown[DOWN];

	// We'll consider moveVelocity existing in player space
	// Allow movement if player is not dead 
	// Movement code, build the velocity resulting from player movement
	if (!registry.deathTimers.has(player_salmon)) {
		Motion& m = registry.motions.get(player_salmon);
		m.velocity = { 0, 0 };

		handle_movement(m, LEFT);

		if (length(m.velocity) > 0) {
			m.velocity *= terrain->get_terrain_speed_ratio(terrain->get_cell(m.position));
			}

		if (anyMovementKeysPressed) {
			PLAYER_TOTAL_DISTANCE += FOOD_DECREASE_RATE * elapsed_ms_since_last_update / 1000.f;

			if (ELAPSED_TIME > 100) {
				// Update walking animation
				registry.players.components[0].framex = (registry.players.components[0].framex + 1) % 4;
				ELAPSED_TIME = 0.0f; // Reset the timer
				}
			}
		else {
			// No movement keys pressed, set back to the first frame
			registry.players.components[0].framex = 0;
			}
		}
	else {
		// Player is dead, do not allow movement
		Motion& m = registry.motions.get(player_salmon);
		m.velocity = { 0, 0 };
		registry.players.components[0].framey = 1;
		}
	}

void WorldSystem::handle_movement(Motion& motion, InputKeyIndex indexStart, bool invertDirection, bool useAbsoluteVelocity)
{
	float invert = invertDirection ? -1 : 1;	// shorthand that inverts the results if invertDirection.
	vec2 moveVelocity = { 0, 0 };
	if (keyDown[indexStart])
		moveVelocity.x += -1;    // If LEFT is pressed then obviously add a left component
	if (keyDown[indexStart + 1]) 
		moveVelocity.x += 1;    // If RIGHT is pressed then obviously add a right component
	if (keyDown[indexStart + 2])
		moveVelocity.y += -1;    // If UP is pressed then obviously add an up component
	if (keyDown[indexStart + 3])
		moveVelocity.y += 1;    // If DOWN is pressed then obviously add a down component

	if (length(moveVelocity) > 0) {
		// prevent motion from going slightly faster if moving diagonally
		moveVelocity = normalize(moveVelocity);

		if (!useAbsoluteVelocity) {
			// Make a quick transformation matrix to rotate the movement according to
			// entity orientation
			float c = cosf(motion.angle);
			float s = sinf(motion.angle);
			mat2 rotate = { { c, s },{ -s, c } }; // Not affine because velocity is ALWAYS a vector

			// Rotate the vector into "world" space. The model/entity matrix turns local/entity coordinates into
			// "world" coordinates.
			moveVelocity = (rotate * moveVelocity);
		}

		motion.velocity = moveVelocity * current_speed * invert;
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 5.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// These weapons don't have a motions so let's kill them all!
	while (registry.weapons.entities.size() > 0)
		registry.remove_all_components_of(registry.weapons.entities.back());

	// Reset the weapons system
	weapons_system->resetWeaponsSystem();

	// Reset the terrain system
	terrain->resetTerrainSystem();

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Re-initialize the terrain
	terrain->init(loaded_map_name, renderer);

	// PRESSURE TESTING FOR BVH, can remove later
	//terrain->init(512, 512, renderer);

	// Add wall of stone around the map
	for (unsigned int i = 0; i < registry.terrainCells.entities.size(); i++) {
		Entity e = registry.terrainCells.entities[i];
		TerrainCell& cell = registry.terrainCells.components[i];

		if (cell.flag & TERRAIN_FLAGS::COLLIDABLE)
			createDefaultCollider(e);
	}

	// THIS MUST BE CALL AFTER TERRAIN COLLIDER CREATION AND BEFORE ALL OTHER ENTITY CREATION
	// build the static BVH with all terrain colliders.
	physics_system->initStaticBVH(registry.colliders.size());

	// Create a Spaceship 

	spaceship = createSpaceship(renderer, { 0,-2.5 });

	// Create Home Screen 
	home = createHome(renderer);

	// Create a new salmon
	player_salmon = createPlayer(renderer, { 0, 0 });
	registry.colors.insert(player_salmon, { 1, 0.8f, 0.8f });

	// Create the main camera
	main_camera = createCamera({ 0,0 });

	// Create fow
	fow = createFOW(renderer, { 0,0 });

	// Create health bars 
	health_bar = createHealthBar(renderer, { -8.f, 7.f });

	// Create food bars 
	food_bar = createFoodBar(renderer, { 8.f, 7.f });

	// Reset the weapon indicator
	user_has_first_weapon = false;

	// A function that handles the help/tutorial (some tool tips at the top of the screen)
	help_bar = createHelp(renderer, { 0.f, -7.f }, tooltips[0]);
	current_tooltip = 1;

	quest_items.clear();
	quest_items.push_back({ createQuestItem(renderer, {10.f, -2.f}, TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND), false });
	quest_items.push_back({ createQuestItem(renderer, {10.f, 2.f}, TEXTURE_ASSET_ID::QUEST_2_NOT_FOUND), false });

	// clear all used spawn locations
	used_spawn_locations.clear();

	// FOR DEMO - to show different types of items being created.	
	spawn_items();
	mob_system->spawn_mobs();

	// for movement velocity
	for (int i = 0; i < KEYS; i++)
	  keyDown[i] = false;

	
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	vec2 hasCorrectedDirection = {0,0};

	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;
		
		// Collisions involving the player
		if (registry.players.has(entity)) {
			Player& player = registry.players.get(entity);
			// Checking Player - Spaceship (For regen)
			// only regnerate after spaceship exit 
			/*
			if (entity_other == spaceship) {
				player.health = PLAYER_MAX_HEALTH;
				player.food = PLAYER_MAX_FOOD;

				Motion& health = registry.motions.get(health_bar);
				Motion& food = registry.motions.get(food_bar);
				health.scale = HEALTH_BAR_SCALE;
				food.scale = FOOD_BAR_SCALE;

				Spaceship& s = registry.spaceship.get(home);
				s.in_home = TRUE;

				Motion& camera_motion = registry.motions.get(main_camera);
				//// update Spaceship home movement 
				Motion& s_motion = registry.motions.get(home);
				s_motion.position = { camera_motion.position.x,camera_motion.position.y };


				printf("nearhome\n");


			}
			*/

			// Checking Player - Mobs
			if (registry.mobs.has(entity_other)) {


				if (player.iframes_timer > 0 || registry.deathTimers.has(entity)) {
					// Don't damage and discard all other collisions for a bit
					collisionsRegistry.clear();
					return;
				}

				Mob& mob = registry.mobs.get(entity_other);
				player.health -= mob.damage;

				// Shrink the health bar
				player.health_decrease_time = IFRAMES; // player's health decays over this period of time, using interpolation
				// use the same amount as iframes so health is never "out of date"

				// Give the player some frames of invincibility so that they cannot die instantly when running into a mob
				player.iframes_timer = IFRAMES;

				if (player.health <= 0) {
					if (!registry.deathTimers.has(entity)) {
						// TODO: game over screen
						registry.deathTimers.emplace(entity);
					}
				}
			}

			// Checking Player - Terrain
			if (registry.terrainCells.has(entity_other) && registry.collisions.components[i].MTV != hasCorrectedDirection)
			
			{
				Motion& motion = registry.motions.get(player_salmon);
				/*
				std::cout << "MTV x" << registry.collisions.components[i].MTV.x << "  MTV Y: " << registry.collisions.components[i].MTV.y << std::endl;
				std::cout << "MTV x" << "overlap " << registry.collisions.components[i].overlap << std::endl;
				*/
				vec2 correctionVec = registry.collisions.components[i].MTV * registry.collisions.components[i].overlap;
				motion.position = motion.position + correctionVec;
				hasCorrectedDirection = registry.collisions.components[i].MTV;
			}

			// Checking Player - Items
			if (registry.items.has(entity_other)) {

				Item& item = registry.items.get(entity_other);

				// Handle the item based on its function
				switch (item.data) {
				case ITEM_TYPE::QUEST_ONE:
					registry.remove_all_components_of(quest_items[0].first);
					quest_items[0].first = createQuestItem(renderer, {10.f, -2.f}, TEXTURE_ASSET_ID::QUEST_1_FOUND);
					quest_items[0].second = true;

					for (auto& item : quest_items) {
						if (!item.second) break;
						// end game
					}
					break;
				case ITEM_TYPE::QUEST_TWO:
					registry.remove_all_components_of(quest_items[1].first);
					quest_items[1].first = createQuestItem(renderer, {10.f, 2.f}, TEXTURE_ASSET_ID::QUEST_2_FOUND);
					quest_items[1].second = true;

					for (auto& item : quest_items) {
						if (!item.second) break;
						// end game
					}
					break;
				case ITEM_TYPE::FOOD:
					// Add to food bar
					player.food += FOOD_PICKUP_AMOUNT;
					if (player.food > PLAYER_MAX_FOOD) {
						player.food = PLAYER_MAX_FOOD;
					}
					break;
				case ITEM_TYPE::WEAPON_NONE:
					// Do nothing
					break;
				case ITEM_TYPE::WEAPON_SHURIKEN:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHURIKEN);
					
					// Remove the current weapon_indicator and add a new one for the equipped weapon
					if (user_has_first_weapon) {
						registry.remove_all_components_of(weapon_indicator);
					} else {
						// Has just picked up the first weapon
						user_has_first_weapon = true;
						help_bar = createHelp(renderer, { 0.f, -7.f }, TEXTURE_ASSET_ID::HELP_WEAPON);
					}
					weapon_indicator = createWeaponIndicator(renderer, { -10.f, -6.f }, TEXTURE_ASSET_ID::ICON_SHURIKEN);
					break;
				case ITEM_TYPE::WEAPON_CROSSBOW:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_CROSSBOW);

					// Remove the current weapon_indicator and add a new one for the equipped weapon
					if (user_has_first_weapon) {
						registry.remove_all_components_of(weapon_indicator);
					} else {
						// Has just picked up the first weapon
						user_has_first_weapon = true;
						help_bar = createHelp(renderer, { 0.f, -7.f }, TEXTURE_ASSET_ID::HELP_WEAPON);
					}
					weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_CROSSBOW);
					break;
				case ITEM_TYPE::WEAPON_SHOTGUN:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHOTGUN);

					// Remove the current weapon_indicator and add a new one for the equipped weapon
					if (user_has_first_weapon) {
						registry.remove_all_components_of(weapon_indicator);
					} else {
						// Has just picked up the first weapon
						user_has_first_weapon = true;
						help_bar = createHelp(renderer, { 0.f, -7.f }, TEXTURE_ASSET_ID::HELP_WEAPON);
					}
					weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_SHOTGUN);
					break;
				case ITEM_TYPE::WEAPON_MACHINEGUN:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_MACHINEGUN);

					// Remove the current weapon_indicator and add a new one for the equipped weapon
					if (user_has_first_weapon) {
						registry.remove_all_components_of(weapon_indicator);
					} else {
						// Has just picked up the first weapon
						user_has_first_weapon = true;
						help_bar = createHelp(renderer, { 0.f, -7.f }, TEXTURE_ASSET_ID::HELP_WEAPON);
					}
					weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_MACHINE_GUN);
					break;
				case ITEM_TYPE::WEAPON_UPGRADE:
					weapons_system->upgradeCurrentWeapon();
					break;
				}

				// remove item from map
				registry.remove_all_components_of(entity_other);
			}
		}

		// Collisions involving projectiles. 
		// For now, the projectile will be removed upon any collisions with mobs/terrain
		// In the future, an idea could be "pass-through weapons", weapons that can collateral?
		if (registry.projectiles.has(entity)) {
			Projectile& projectile = registry.projectiles.get(entity);

			// Checking Projectile - Mobs
			if (registry.mobs.has(entity_other)) {
				Mob& mob = registry.mobs.get(entity_other);

				// Mob takes damage. Kill if no hp left.
				mob.health -= projectile.damage;
				// printf("mob health: %i", mob.health);
				if (mob.health <= 0) {
					registry.remove_all_components_of(entity_other);
					audio_system->PlayOneShot(AudioSystem::MOB_DEATH);
				}
				else {
					audio_system->PlayOneShot(AudioSystem::MOB_HIT);
				}
				// Add weapon effects to the mob
				weapons_system->applyWeaponEffects(entity, entity_other);

				// Remove projectile
				registry.remove_all_components_of(entity);
			} 
			// Checking Projectile - non passable terrain cell
			else if ((registry.terrainCells.has(entity_other) && (registry.terrainCells.get(entity_other).flag & TERRAIN_FLAGS::COLLIDABLE)))
			{
				registry.remove_all_components_of(entity);
			}
			
			// Checking Projectile - Boundary 
			else if (registry.boundaries.has(entity_other))
			{
				registry.remove_all_components_of(entity);
			}
		}
		// Todo: Collision involving Player - Spaceship should not allow player move ?
			
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}
// PARAM IS TIME IN OUR EXAMPLE, BETWEEN 0 AND 1
vec2 WorldSystem::interpolate(vec2 p1, vec2 p2, float param) {
	float new_x = p1[0] + ((p2[0] - p1[0]) * param);
	float new_y = p1[1] + ((p2[1] - p1[1]) * param);
	return vec2(new_x, new_y);
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// check if player is home and pause the game in main
bool WorldSystem::is_home() const {
	return registry.spaceship.get(home).in_home; 
	}


int WorldSystem::key_to_index(int key) {
	switch (key) {
			case GLFW_KEY_W:
				return InputKeyIndex::UP;
			case GLFW_KEY_S:
				return InputKeyIndex::DOWN;
			case GLFW_KEY_A:
				return InputKeyIndex::LEFT;
			case GLFW_KEY_D:
				return InputKeyIndex::RIGHT;

			// camera controls
			case GLFW_KEY_UP:
				return InputKeyIndex::CAMERA_UP;
			case GLFW_KEY_DOWN:
				return InputKeyIndex::CAMERA_DOWN;
			case GLFW_KEY_LEFT:
				return InputKeyIndex::CAMERA_LEFT;
			case GLFW_KEY_RIGHT:
				return InputKeyIndex::CAMERA_RIGHT;

		}
	return -1;    // key is not tracked so we don't really care
	}

void WorldSystem::update_key_presses(int key, int action) {
	int i = key_to_index(key);
	if (i >= 0) {
		if (action == GLFW_PRESS) {
			keyDown[i] = true;

			// If we press down any of the arrow keys, set camera to freemode.
			if (i >= CAMERA_LEFT && i <= CAMERA_DOWN) {
				Camera& c = registry.cameras.get(main_camera);
				c.mode_follow = false;
			}
		}

		if (action == GLFW_RELEASE) {
			keyDown[i] = false;

			if (i >= CAMERA_LEFT && i <= CAMERA_DOWN) {
				update_camera_follow();
			}
		}
	}
}

void WorldSystem::update_camera_follow() {
	for (int i = CAMERA_LEFT; i <= CAMERA_DOWN; i++) {
		if (keyDown[i])
			return;	// if any of the camera control buttons are still pressed, do not follow the player just yet
	}
	Camera& c = registry.cameras.get(main_camera);
	c.mode_follow = true;
}


// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	Motion& player_motion = registry.motions.get(player_salmon);
	Spaceship& s = registry.spaceship.get(home);
	// Movement with velocity handled in step function  
	update_key_presses(key, action);

	/*
	if (action == GLFW_PRESS && key == GLFW_KEY_G) {
		Motion& player = registry.motions.get(player_salmon);
		Entity tile = terrain->get_cell(player.position);
		TerrainCell& cell = registry.terrainCells.get(tile);
		cell.terrain_type = TERRAIN_TYPE::ROCK;
		terrain->update_tile(tile, cell);
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_V) {
		Motion& player = registry.motions.get(player_salmon);
		Entity tile = terrain->get_cell(player.position);

		std::vector<Entity> entities;
		terrain->get_accessible_neighbours(tile, entities);
		for (Entity e : entities) {
			TerrainCell& cell = registry.terrainCells.get(e);
			cell.terrain_type = TERRAIN_TYPE::ROCK;
			cell.flag |= TERRAIN_FLAGS::COLLIDABLE;
			terrain->update_tile(e, cell);
		}
	}
	*/

	// Saving and reloading
	if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		// Load the game state 
		std::ifstream f("save.json");
		json data = json::parse(f);

		std::cout << "Loading ...";

		load_game(data);
	}
	
	if (action == GLFW_PRESS && key == GLFW_KEY_K) {
		// Save the game state (player location, weapon, health, food, mobs & location)
		Player& player = registry.players.get(player_salmon);
		Motion& player_motion = registry.motions.get(player_salmon);

		std::vector<std::pair<Mob&, Motion&>> mobs;
		for (auto& mob : registry.mobs.entities) {
			mobs.push_back({ registry.mobs.get(mob), registry.motions.get(mob) });
		}

		std::vector<std::pair<Item&, Motion&>> items;
		for (auto& item : registry.items.entities) {
			items.push_back({ registry.items.get(item), registry.motions.get(item) });
		}

		std::vector<bool> quests;
		for (auto& item : quest_items) {
			quests.push_back(item.second);
		}

		ITEM_TYPE type = ITEM_TYPE::WEAPON_NONE;
		if (user_has_first_weapon) {
			Weapon& weapon = registry.weapons.get(player_equipped_weapon);
			type = weapon.weapon_type;
		}

		SaveGame(player, player_motion, mobs, items, quests, type);

		tooltips_on = false;
		help_bar = createHelp(renderer, { 0.f, -7.f }, TEXTURE_ASSET_ID::SAVING);
		current_tooltip = tooltips.size();
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		if (s.in_home) {
			// Exit home screen and go back to world 
			// Todo: Regenerate after exit
			s.in_home = false;
			player_motion.position = { 0,0 };

			}
		else {
			// Close the window if not in home screen
			glfwSetWindowShouldClose(window, true);
			}
	}

	// Enter ship if player is near
	if (length(registry.motions.get(player_salmon).position - registry.motions.get(spaceship).position) < 1.0f && s.in_home == false) {
		printf("Near entrance, press E to enter\n");
		if (action == GLFW_PRESS && key == GLFW_KEY_E ) {
			Player& player = registry.players.get(player_salmon);

			Spaceship& s = registry.spaceship.get(home);
			player.health = PLAYER_MAX_HEALTH;
			player.food = PLAYER_MAX_FOOD;

			Motion& health = registry.motions.get(health_bar);
			Motion& food = registry.motions.get(food_bar);
			health.scale = HEALTH_BAR_SCALE;
			food.scale = FOOD_BAR_SCALE;


			// no caemra shake 
			Motion& camera_motion = registry.motions.get(main_camera);
			camera_motion.angle = 0;
			camera_motion.scale = vec2(1, 1);

			//camera_motion.scale = vec2(0, 0); 
			//// update Spaceship home movement 
			Motion& s_motion = registry.motions.get(home);
			s_motion.position = { camera_motion.position.x,camera_motion.position.y };

			printf("You are home\n");
			s.in_home = true;
		}
	}


	// Debugging
	/*if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}*/

	// Level editor controls
	if (debugging.in_debug_mode && action == GLFW_PRESS) {
		if (key == GLFW_KEY_KP_1) {	// numpad 1
			// Toggle collidable flag
			editor_flag ^= TERRAIN_FLAGS::COLLIDABLE;
			if (editor_flag & TERRAIN_FLAGS::COLLIDABLE)
				std::cout << "New terrain are now collidable" << std::endl;
			else
				std::cout << "New terrain are now non-collidable" << std::endl;
		}
		if (key == GLFW_KEY_KP_2) {	// numpad 2
			// Toggles pathfindable flag
			editor_flag ^= TERRAIN_FLAGS::DISABLE_PATHFIND;
			if (editor_flag & TERRAIN_FLAGS::DISABLE_PATHFIND)
				std::cout << "New terrain are now inaccessible to mobs" << std::endl;
			else
				std::cout << "New terrain are now accessible for mobs" << std::endl;
		}
		if (key == GLFW_KEY_KP_3) {	// numpad 3
			// Toggles pathfindable flag
			editor_flag ^= TERRAIN_FLAGS::ALLOW_SPAWNS;
			if (editor_flag & TERRAIN_FLAGS::ALLOW_SPAWNS)
				std::cout << "New terrain can now be spawned by items and mobs (if no collision)" << std::endl;
			else
				std::cout << "New terrain are now removed from the spawning pool" << std::endl;
		}
		if (key == GLFW_KEY_KP_SUBTRACT) {	// numpad -
			// Goes down a TERRAIN_TYPE
			if (editor_terrain == 0) {
				editor_terrain = static_cast<TERRAIN_TYPE>(TERRAIN_COUNT - 1);
			}
			else {
				editor_terrain = static_cast<TERRAIN_TYPE>(editor_terrain - 1);
			}
			std::cout << "Tile: " << std::to_string(editor_terrain) << std::endl;
		}
		if (key == GLFW_KEY_KP_ADD) {	// numpad '+'
			// Goes up a TERRAIN_TYPE

			editor_terrain = static_cast<TERRAIN_TYPE>(editor_terrain + 1);
			if (editor_terrain == TERRAIN_COUNT) {
				editor_terrain = static_cast<TERRAIN_TYPE>(0);
			}
			std::cout << "Tile: " << std::to_string(editor_terrain) << std::endl;
		}
		if (key == GLFW_KEY_KP_DECIMAL)	// numpad '.'
			// Saves map data
			terrain->save_grid(loaded_map_name);	
	}

	// Press B to toggle debug mode
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		debugging.in_debug_mode = !debugging.in_debug_mode;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);

	// TESTING: hotkeys to equip weapons
	//if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
	//	player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHURIKEN);

	//	// Remove the current weapon_indicator and add a new one for the equipped weapon
	//	registry.remove_all_components_of(weapon_indicator);
	//	weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_SHURIKEN);

	//}
	//if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
	//	player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_CROSSBOW);

	//	// Remove the current weapon_indicator and add a new one for the equipped weapon
	//	registry.remove_all_components_of(weapon_indicator);
	//	weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_CROSSBOW);

	//}
	//if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
	//	player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHOTGUN);

	//	// Remove the current weapon_indicator and add a new one for the equipped weapon
	//	registry.remove_all_components_of(weapon_indicator);
	//	weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_SHOTGUN);

	//}
	//if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
	//	player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_MACHINEGUN);

	//	// Remove the current weapon_indicator and add a new one for the equipped weapon
	//	registry.remove_all_components_of(weapon_indicator);
	//	weapon_indicator = createWeaponIndicator(renderer, {-10.f, -6.f}, TEXTURE_ASSET_ID::ICON_MACHINE_GUN);

	//}

	// TESING: hotkey to upgrade weapon
	if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		weapons_system->upgradeCurrentWeapon();
	}
}
/// <summary>
/// Rotates player to follow mouse movement
/// </summary>
/// <param name="mouse_position"> Location of mouse on game screen</param>
void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// Disable rotation when the player dies
	if (!registry.deathTimers.has(player_salmon)) {
		// The player is always in the middle of the screen so we need to compute the 
		// rotation angle w.r.t. the centre of the screen
		ivec2 window_size = renderer->window_resolution;

		float screen_centre_x = window_size.x/2;
		float screen_centre_y = window_size.y/2;

		Motion& motion = registry.motions.get(player_salmon);
		CURSOR_ANGLE = atan2(mouse_position.y - screen_centre_y, mouse_position.x - screen_centre_x);
		//printf("View direction: %f \n", cursor_angle);
	}
}


/// <summary>
/// Function to handle mouse click (weapon fire)
/// </summary>
void WorldSystem::on_mouse_click(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (!registry.deathTimers.has(player_salmon)) {
			Motion& player_motion = registry.motions.get(player_salmon);

			if (registry.weapons.has(player_equipped_weapon)) {
				Weapon& w = registry.weapons.get(player_equipped_weapon);

				// Play appropriate shooting noises if we've just shot
				if (w.can_fire) {
					switch (w.weapon_type) {
					case ITEM_TYPE::WEAPON_SHOTGUN:
						audio_system->PlayOneShot(AudioSystem::SHOT); break;
					case ITEM_TYPE::WEAPON_MACHINEGUN:
						audio_system->PlayOneShot(AudioSystem::SHOT_MG); break;
					case ITEM_TYPE::WEAPON_CROSSBOW:
						audio_system->PlayOneShot(AudioSystem::SHOT_CROSSBOW); break;
					}
				}
			}

			weapons_system->fireWeapon(player_motion.position.x, player_motion.position.y, CURSOR_ANGLE);
		}
	}

	if (debugging.in_debug_mode && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		mat3 view_ = renderer->createModelMatrix(main_camera);

		// You can cache this to save performance.
		mat3 proj_ = inverse(renderer->createProjectionMatrix());

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);	// For some reason it only supports doubles!
		ivec2 window_size = renderer->window_resolution;

		// Recall that valid clip coordinates are between [-1, 1]. 
		// First, we need to turn screen (pixel) coordinates into clip coordinates:
		vec3 mouse_pos = {
			(xpos / window_size.x) * 2 - 1,			// Get the fraction of the x pos in the screen, multiply 2 to map range to [0, 2], 
													// then offset so the range is now [-1, 1].
			-(ypos / window_size.y) * 2 + 1,		// Same thing, but recall that the y direction is opposite in glfw.
			1.0 };									// Denote that this is a point.
		mouse_pos = view_ * proj_ * mouse_pos;

		Entity tile = terrain->get_cell(mouse_pos);
		TerrainCell& cell = registry.terrainCells.get(tile);
		bool to_collidable = (editor_flag & COLLIDABLE);
		bool from_collidable = (cell.flag & COLLIDABLE);

		cell.terrain_type = editor_terrain;
		cell.flag = editor_flag;

		// Update collisions
		if (to_collidable != from_collidable) {
			if (to_collidable) {
				createDefaultCollider(tile);
			}
			else {
				registry.colliders.remove(tile);
			}
			physics_system->initStaticBVH(registry.colliders.size());
		}

		terrain->update_tile(tile, cell, true);	// true because we need to update adjacent cells too
	}
}

void WorldSystem::spawn_items() {
	const int NUM_ITEM_TYPES = 2;

	for (int i = 0; i < ITEM_LIMIT; i++) {
		// Get random spawn location
		vec2 spawn_location = terrain->get_random_terrain_location();

		// Randomly choose item type
		int item_type = rng() % NUM_ITEM_TYPES;

		switch (item_type) {
			case 0:
				createItem(renderer, spawn_location, ITEM_TYPE::WEAPON_UPGRADE);
				break;
			case 1:
				createItem(renderer, spawn_location, ITEM_TYPE::FOOD);
				break;
		}
	}

	// TESTING: Force one spawn of each weapon.
	 createItem(renderer, terrain->get_random_terrain_location(), ITEM_TYPE::WEAPON_SHURIKEN);
	 createItem(renderer, terrain->get_random_terrain_location(), ITEM_TYPE::WEAPON_CROSSBOW);
	 createItem(renderer, terrain->get_random_terrain_location(), ITEM_TYPE::WEAPON_SHOTGUN);
	 createItem(renderer, terrain->get_random_terrain_location(), ITEM_TYPE::WEAPON_MACHINEGUN);

	 // TESTING: Force spawn quest items once
	 createItem(renderer, terrain->get_random_terrain_location(), ITEM_TYPE::QUEST_ONE);
	 createItem(renderer, terrain->get_random_terrain_location(), ITEM_TYPE::QUEST_TWO);
}

// Adapted from restart_game, BASICALLY alot of optional arguments to change small things :D
void WorldSystem::load_game(json j) {
	vec2 player_location = { j["player_motion"]["position_x"], j["player_motion"]["position_y"] };

	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 5.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// These weapons don't have a motions so let's kill them all!
	while (registry.weapons.entities.size() > 0)
		registry.remove_all_components_of(registry.weapons.entities.back());

	// Reset the weapons system
	weapons_system->resetWeaponsSystem();

	ITEM_TYPE weapon_type = (ITEM_TYPE)j["weapon"];
	if (weapon_type == ITEM_TYPE::WEAPON_NONE) {
		user_has_first_weapon = false;
		registry.remove_all_components_of(weapon_indicator);
	}
	else {
		// GIVE player their weapon if they have one, and set weapon indicator accordingly
		player_equipped_weapon = weapons_system->createWeapon(weapon_type);
		user_has_first_weapon = true;

		switch (weapon_type) {
		case ITEM_TYPE::WEAPON_CROSSBOW:
			weapon_indicator = createWeaponIndicator(renderer, { -10.f, -6.f }, TEXTURE_ASSET_ID::ICON_CROSSBOW);
			break;
		case ITEM_TYPE::WEAPON_MACHINEGUN:
			weapon_indicator = createWeaponIndicator(renderer, { -10.f, -6.f }, TEXTURE_ASSET_ID::ICON_MACHINE_GUN);
			break;
		case ITEM_TYPE::WEAPON_SHOTGUN:
			weapon_indicator = createWeaponIndicator(renderer, { -10.f, -6.f }, TEXTURE_ASSET_ID::ICON_SHOTGUN);
			break;
		case ITEM_TYPE::WEAPON_SHURIKEN:
			weapon_indicator = createWeaponIndicator(renderer, { -10.f, -6.f }, TEXTURE_ASSET_ID::WEAPON_SHURIKEN);
			break;
		}

	}

	// Reset the terrain system
	terrain->resetTerrainSystem();

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Re-initialize the terrain
	terrain->init(loaded_map_name, renderer);

	// PRESSURE TESTING FOR BVH, can remove later
	//terrain->init(512, 512, renderer);

	// Add wall of stone around the map
	for (unsigned int i = 0; i < registry.terrainCells.entities.size(); i++) {
		Entity e = registry.terrainCells.entities[i];
		TerrainCell& cell = registry.terrainCells.components[i];

		if (cell.flag & TERRAIN_FLAGS::COLLIDABLE)
			createDefaultCollider(e);
	}

	// THIS MUST BE CALL AFTER TERRAIN COLLIDER CREATION AND BEFORE ALL OTHER ENTITY CREATION
	// build the static BVH with all terrain colliders.
	physics_system->initStaticBVH(registry.colliders.size());

	// Create a Spaceship 
	spaceship = createSpaceship(renderer, { 0,0 });

	// Create a new salmon
	player_salmon = createPlayer(renderer, player_location);
	Player& player = registry.players.get(player_salmon);
	player.health = j["player"]["health"];
	player.food = j["player"]["food"];
	registry.colors.insert(player_salmon, { 1, 0.8f, 0.8f });

	// Create the main camera
	main_camera = createCamera(player_location);

	// Create fow
	fow = createFOW(renderer, player_location);

	// Create health bars 
	health_bar = createHealthBar(renderer, { -8.f, 7.f }, player.health);

	// Create food bars 
	food_bar = createFoodBar(renderer, { 8.f, 7.f }, player.food);

	tooltips_on = false;
	help_bar = createHelp(renderer, { 0.f, -7.f }, TEXTURE_ASSET_ID::LOADED);
	current_tooltip = tooltips.size();

	quest_items.clear();

	if (!j["quests"][0]) {
		quest_items.push_back({ createQuestItem(renderer, {10.f, -2.f}, TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND), false });
	}
	else {
		quest_items.push_back({ createQuestItem(renderer, {10.f, -2.f}, TEXTURE_ASSET_ID::QUEST_1_FOUND), true });
	}

	if (!j["quests"][1]) {
		quest_items.push_back({ createQuestItem(renderer, {10.f, 2.f}, TEXTURE_ASSET_ID::QUEST_2_NOT_FOUND), false });
	}
	else {
		quest_items.push_back({ createQuestItem(renderer, {10.f, 2.f}, TEXTURE_ASSET_ID::QUEST_2_FOUND), true });
	}

	// clear all used spawn locations
	used_spawn_locations.clear();

	load_spawned_items_mobs(j);

	// for movement velocity
	for (int i = 0; i < KEYS; i++)
		keyDown[i] = false;
}

void WorldSystem::load_spawned_items_mobs(json& j) {
	for (auto& item : j["items"]) {
		createItem(renderer, { item[1]["position_x"], item[1]["position_y"] }, item[0]["data"]);
	}

	for (auto& mob : j["mobs"]) {
		mob_system->create_mob({ mob[1]["position_x"], mob[1]["position_y"] }, mob[0]["type"], mob[0]["health"]);
	}
}