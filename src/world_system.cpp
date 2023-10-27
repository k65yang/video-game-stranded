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



float elapsed_time = 0;
// Create the fish world
WorldSystem::WorldSystem()
	: points(0)
	, next_turtle_spawn(0.f)
	, next_fish_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	Mix_CloseAudio();

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

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Stranded", nullptr, nullptr);
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

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("salmon_eat.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("salmon_dead.wav").c_str(),
			audio_path("salmon_eat.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, TerrainSystem* terrain_arg, WeaponsSystem* weapons_system_arg) {
	this->renderer = renderer_arg;
	this->terrain = terrain_arg;
	this->weapons_system = weapons_system_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

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

	// rendering spritesheet with movement 
	elapsed_time += elapsed_ms_since_last_update;
	if (keyDown[LEFT]|| keyDown[LEFT] && keyDown[DOWN]|| keyDown[LEFT] && keyDown[UP]) {
		registry.players.components[0].framey = 3;
		if (elapsed_time > 100) {
			registry.players.components[0].framex = (registry.players.components[0].framex + 1) % 4;
			elapsed_time = 0.0f; // Reset the timer
		}
	}
	else if (keyDown[RIGHT] || keyDown[RIGHT] && keyDown[DOWN] || keyDown[RIGHT] && keyDown[UP]) {
		registry.players.components[0].framey = 1;
		if (elapsed_time > 100) {
			registry.players.components[0].framex = (registry.players.components[0].framex + 1) % 4;
			elapsed_time = 0.0f; // Reset the timer
			}
		}

	else if (keyDown[DOWN]) {
		registry.players.components[0].framey = 2;

		if (elapsed_time > 100) {
			registry.players.components[0].framex = (registry.players.components[0].framex + 1) % 4;
			elapsed_time = 0.0f; // Reset the timer
			}
		}

	else if (keyDown[UP]) {
		registry.players.components[0].framey = 0;

		if (elapsed_time > 100) {
			registry.players.components[0].framex = (registry.players.components[0].framex + 1) % 4;
			elapsed_time = 0.0f; // Reset the timer
			}
		}
	//No keys pressed 
	else {
		registry.players.components[0].framey = 2;
		registry.players.components[0].framex = 0;

		}


	// Movement code, build the velocity resulting from player moment
	// We'll consider moveVelocity existing in player space
	// Allow movement if player is not dead 
	if (!registry.deathTimers.has(player_salmon)) {
		m.velocity = { 0, 0 };
		handle_movement(m, LEFT);

		// If any keys are pressed resulting movement then add to total travel distance. 
		if (keyDown[LEFT] || keyDown[RIGHT] || keyDown[UP] || keyDown[DOWN]) {
			PLAYER_TOTAL_DISTANCE += FOOD_DECREASE_RATE * elapsed_ms_since_last_update / 1000.f;
		}
	}
	else {
		// Player is dead, do not allow movement
		m.velocity = { 0, 0 };
	}

	// Camera movement mode
	Camera& c = registry.cameras.get(main_camera);
	Motion& camera_motion = registry.motions.get(main_camera);
	if (c.mode_follow) {
		camera_motion.position = m.position;	// why are the positions inverted???
	}
	else {
		handle_movement(camera_motion, CAMERA_LEFT);
	}
	// bars movement 
	Motion& health = registry.motions.get(health_bar);
	Motion& food = registry.motions.get(food_bar);

	health.position = { -8.f + camera_motion.position.x, 7.f + camera_motion.position.y };
	food.position = { 8.f + camera_motion.position.x, 7.f + camera_motion.position.y };

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

				// We need to force path finding to restart. 
				// If not, the mob might continue in the current direction forever.
				Mob& mob = registry.mobs.get(entity);
				mob.is_tracking_player = false;
			}
		}
	}

	return true;
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

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Re-initialize the terrain
	terrain->init(world_size_x, world_size_y, renderer);

	// Create a Spaceship 
	createSpaceship(renderer, { 0,0 });

	// Create a new salmon
	player_salmon = createPlayer(renderer, { 0, 0 });
	registry.colors.insert(player_salmon, { 1, 0.8f, 0.8f });

	// Equip the player weapon. Player starts with no weapon for now.
	// TODO: do we want to give the player a starting weapon?
	player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_NONE);

	// Create the main camera
	main_camera = createCamera({ 0,0 });

	// Create fow
	fow = createFOW(renderer, { 0,0 });

	// Create health bars 
	health_bar = createHealthBar(renderer, { -8.f, 7.f });

	// Create food bars 
	food_bar = createFoodBar(renderer, { 8.f, 7.f });

	// Add wall of stone around the map
	for (unsigned int i = 0; i < registry.terrainCells.entities.size(); i++) {
		Entity e = registry.terrainCells.entities[i];
		TerrainCell& cell = registry.terrainCells.components[i];

		if (cell.flag & TERRAIN_FLAGS::COLLIDABLE)
			createCollider(e);
	}

	//FOR DEMO, CAN REMOVE LATER
	createTerrainCollider(renderer, terrain, { 3.f, -3.f });  
	createTerrainCollider(renderer, terrain, { 3.f, 3.f });   
	createTerrainCollider(renderer, terrain, { -3.f, 3.f });  
	createTerrainCollider(renderer, terrain, { -3.f, -3.f });
	
	// clear all used spawn locations
	used_spawn_locations.clear();

	// FOR DEMO - to show different types of items being created.	
	spawn_items();
	spawn_mobs();

	// for movement velocity
	for (int i = 0; i < KEYS; i++)
	  keyDown[i] = false;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other_entity;

		// Collisions involving the player
		if (registry.players.has(entity)) {
			Player& player = registry.players.get(entity);

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
			

			if (registry.terrainCells.has(entity_other) || registry.boundaries.has(entity_other)) {

				Motion& motion = registry.motions.get(player_salmon); 
				vec2 correctionVec = registry.collisions.components[i].MTV * registry.collisions.components[i].overlap;
				motion.position = motion.position + correctionVec;
			}

			// Checking Player - Items
			if (registry.items.has(entity_other)) {

				Item& item = registry.items.get(entity_other);

				// Handle the item based on its function
				switch (item.data) {
				case ITEM_TYPE::QUEST:
					// Display a quest item as having been fetched, and update this on the screen?
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
					// TODO: some sort of UI update
					break;
				case ITEM_TYPE::WEAPON_CROSSBOW:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_CROSSBOW);
					break;
				case ITEM_TYPE::WEAPON_SHOTGUN:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHOTGUN);
					break;
				case ITEM_TYPE::WEAPON_MACHINEGUN:
					player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_MACHINEGUN);
					break;
				case ITEM_TYPE::UPGRADE:
					// Just add to inventory
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

	// Movement with velocity handled in step function  
	update_key_presses(key, action);

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

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, true);
	}

	// Debugging
	/*if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}*/

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
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHURIKEN);
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_CROSSBOW);
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_SHOTGUN);
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		player_equipped_weapon = weapons_system->createWeapon(ITEM_TYPE::WEAPON_MACHINEGUN);
	}

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
		float screen_centre_x = window_width_px/2;
		float screen_centre_y = window_height_px/2;

		Motion& motion = registry.motions.get(player_salmon);
		//motion.angle = atan2(mouse_position.y - screen_centre_y, mouse_position.x - screen_centre_x);
		// printf("View direction: %f \n", motion.angle);
	}
}


/// <summary>
/// Function to handle mouse click (weapon fire)
/// </summary>
void WorldSystem::on_mouse_click(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		Motion& player_motion = registry.motions.get(player_salmon);
		// printf("player x: %f, player y: %f \n", player_motion.position.x, player_motion.position.y);
		weapons_system->fireWeapon(player_motion.position.x, player_motion.position.y, player_motion.angle);
	}
}

void WorldSystem::spawn_items() {
	const int NUM_ITEM_TYPES = 3;

	for (int i = 0; i < ITEM_LIMIT; i++) {
		// Get random spawn location
		vec2 spawn_location = get_random_spawn_location();

		// Randomly choose item type
		int item_type = rng() % NUM_ITEM_TYPES;

		switch (item_type) {
			case 0:
				createItem(renderer, spawn_location, ITEM_TYPE::QUEST);
				break;
			case 1:
				createItem(renderer, spawn_location, ITEM_TYPE::FOOD);
				break;
			case 2:
				createItem(renderer, spawn_location, ITEM_TYPE::UPGRADE);
				break;
		}
	}

	// TESTING: Force one spawn of each weapon.
	// createItem(renderer, get_random_spawn_location(), ITEM_TYPE::WEAPON_SHURIKEN);
	// createItem(renderer, get_random_spawn_location(), ITEM_TYPE::WEAPON_CROSSBOW);
	// createItem(renderer, get_random_spawn_location(), ITEM_TYPE::WEAPON_SHOTGUN);
	// createItem(renderer, get_random_spawn_location(), ITEM_TYPE::WEAPON_MACHINEGUN);
};

void WorldSystem::spawn_mobs() {
	for (int i = 0; i < MOB_LIMIT; i++) {
		// Get random spawn location
		vec2 spawn_location = get_random_spawn_location();
		
		createBasicMob(renderer, spawn_location);
	}
};

vec2 WorldSystem::get_random_spawn_location() {
	vec2 position;

	// Get unused spawn location within [-terrain->size_x/2 + 1, terrain->size_x/2 - 1] for x and 
	// [-terrain->size_y/2 + 1, terrain->size_y/2 - 1] for y
	while (true) {
		position.x = abs((int) rng()) % (terrain->size_x - 2) + (-((terrain->size_x) / 2)) + 1;
		position.y = abs((int) rng()) % (terrain->size_y - 2) + (-((terrain->size_y) / 2)) + 1;

		// Skip locations that are covered by spaceship
		if (position.x <= 1 && position.x >= -1 && position.y <= 2 && position.y >= -2) {
			continue;
		}

		// Skip locations that are not accessible
		if (terrain->isImpassable(position))
			continue;

		if (!is_spawn_location_used(position)) {
			break;
		}
	} 

	// Add spawn location to used spawn locations
	used_spawn_locations.push_back(position);

	return position;
};

bool WorldSystem::is_spawn_location_used(vec2 position) {
	for (vec2 p : used_spawn_locations) {
		if (p.x == position.x && p.y == position.y) {
			return true;
		}
	}

	return false;
};
