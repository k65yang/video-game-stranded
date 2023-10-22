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

void WorldSystem::init(RenderSystem* renderer_arg, TerrainSystem* terrain_arg) {
	this->renderer = renderer_arg;
	this->terrain = terrain_arg;
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

		if (player.health_decrease_time > 0) {
			player.health_decrease_time -= elapsed_ms_since_last_update;

			if (player.health_decrease_time < 0) {
				player.health_decrease_time = 0;
			}
			else {
				Motion& health = registry.motions.get(health_bar);
				vec2 new_health_scale = vec2(((float)player.health / (float)PLAYER_MAX_HEALTH) * HEALTH_BAR_SCALE[0], HEALTH_BAR_SCALE[1]);
				health.scale = interpolate(health.scale, new_health_scale, 1 - (player.health_decrease_time / IFRAMES));
			}
		}
	}

	// Tick down iframes timer and food decrease timer 
	for (Entity entity : registry.players.entities) {
		Player& player = registry.players.get(entity);
		player.iframes_timer -= elapsed_ms_since_last_update;

		if (player.iframes_timer < 0) {
			player.iframes_timer = 0;
			}

		if (player.food_decrease_time > 0) {
			player.food_decrease_time -= elapsed_ms_since_last_update;

			if (player.food_decrease_time < 0) {
				player.food_decrease_time = 0;
				}
			else {
				Motion& food = registry.motions.get(food_bar);
				vec2 new_food_scale = vec2(((float)player.food / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);
				food.scale = interpolate(food.scale, new_food_scale, 1 - (player.food_decrease_time / IFRAMES));
				}
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
		// TODO: game over screen
		registry.deathTimers.emplace(player_salmon);
		}

	// reduce window brightness if any of the present players is dying
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	Motion& m = registry.motions.get(player_salmon);
	Motion& f = registry.motions.get(fow);
	f.position = m.position;



	// Movement code, build the velocity resulting from player moment
	// We'll consider moveVelocity existing in player space
	// Allow movment if player is not dead 

	
	if (!registry.deathTimers.has(player_salmon)) {
		m.velocity = { 0, 0 };
		handle_movement(m, LEFT);

		// If any keys are pressed resulting movement then add to total travel distance. 
		if (keyDown[LEFT] || keyDown[RIGHT] || keyDown[UP] || keyDown[DOWN]) {
			PLAYER_TOTAL_DISTANCE += 0.1;
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

	// Update weapons cooldowns
	for (Entity entity: registry.weapons.entities) {
		Weapon& weapon = registry.weapons.get(entity);
		if (weapon.can_fire)
			continue;

		weapon.elapsed_last_shot_time_ms += elapsed_ms_since_last_update;
		if (weapon.fire_rate < weapon.elapsed_last_shot_time_ms)
			weapon.can_fire = true;
	}

	return true;
}

void WorldSystem::handle_movement(Motion& motion, InputKeyIndex index_start, bool invertDirection)
{
	float invert = invertDirection ? -1 : 1;	// shorthand that inverts the results if invertDirection.
	vec2 moveVelocity = { 0, 0 };
	if (keyDown[index_start])
		moveVelocity.x += -1;    // If LEFT is pressed then obviously add a left component
	if (keyDown[index_start + 1])
		moveVelocity.x += 1;    // If RIGHT is pressed then obviously add a right component
	if (keyDown[index_start + 2])
		moveVelocity.y += -1;    // If UP is pressed then obviously add an up component
	if (keyDown[index_start + 3])
		moveVelocity.y += 1;    // If DOWN is pressed then obviously add a down component

	if (length(moveVelocity) > 0) {
		// prevent motion from going slightly faster if moving diagonally
		moveVelocity = normalize(moveVelocity);

		motion.velocity = moveVelocity * current_speed * invert;
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

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
	player_equipped_weapon = createAndEquipWeapon(ITEM_TYPE::WEAPON_NONE);

	// Create the main camera
	main_camera = createCamera({0,0});

	// Create fow
	fow = createFOW(renderer, { 0,0 });

	// Create health bars 
	health_bar = createHealthBar(renderer, {-8.f, 7.f });

	// Create food bars 
	food_bar = createFoodBar(renderer, { 8.f, 7.f });

	createBoxBoundary(renderer, { 15, 15 }, { 0,0 });


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
			if (registry.terrainColliders.has(entity_other)) {

				Motion& motion = registry.motions.get(player_salmon);

				// resetting key press and respected velocity
				
				if (keyDown[UP] == true) {
					motion.velocity.y = 0;
				}

				if (keyDown[DOWN] == true) {
					motion.velocity.y = 0;
				}

				if (keyDown[LEFT] == true) {
					motion.velocity.x = 0;
				}

				if (keyDown[RIGHT] == true) {
					motion.velocity.x = 0;
				}

				// correct player position. Will need to improve for later milestone
				motion.position.x = positionCorrection(motion.position.x);
				motion.position.y = positionCorrection(motion.position.y);

				// remove handled collision, not needed apparently
				// collisionsRegistry.remove(entity_other);
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
				case ITEM_TYPE::WEAPON_GENERIC:
					player_equipped_weapon = createAndEquipWeapon(ITEM_TYPE::WEAPON_GENERIC);
					// TODO: some sort of UI update
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

				// Mob takes damage.
				mob.health -= projectile.damage;

				// If mob has no hp, it is killed.
				if (mob.health <= 0) {
					registry.remove_all_components_of(entity_other);
				}
				registry.remove_all_components_of(entity);
			} 
			// Checking Projectile - Terrain Colliders
			else if (registry.terrainColliders.has(entity_other)) {
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
		RenderRequest& req = registry.renderRequests.get(tile);
		req.used_texture = TEXTURE_ASSET_ID::TERRAIN_STONE;
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_V) {
		Motion& player = registry.motions.get(player_salmon);
		Entity tile = terrain->get_cell(player.position);

		std::vector<Entity> entities;
		terrain->get_accessible_neighbours(tile, entities);
		for (Entity e : entities) {
			RenderRequest& req = registry.renderRequests.get(e);
			TerrainCell& cell = registry.terrainCells.get(tile);
			cell.terrain_type = TERRAIN_TYPE::ROCK;
			req.used_texture = TEXTURE_ASSET_ID::TERRAIN_STONE;
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
		motion.angle = atan2(mouse_position.y - screen_centre_y, mouse_position.x - screen_centre_x);
		// printf("View direction: %f \n", motion.angle);
	}
}

/// <summary>
/// Function to handle mouse click (weapon fire)
/// </summary>
void WorldSystem::on_mouse_click(int button, int action, int mods) {
	Weapon& weapon = registry.weapons.get(player_equipped_weapon);
	if (weapon.weapon_type != ITEM_TYPE::WEAPON_NONE && weapon.can_fire) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			Motion& player_motion = registry.motions.get(player_salmon);

			// Create the projectile
			// TODO: offset projectile location a little so it doesn't get created on top of player
			Entity entity = createProjectile(renderer, {player_motion.position.x, player_motion.position.y});

			// Set projectile direction
			Motion& projectile_motion = registry.motions.get(entity);
			projectile_motion.angle = player_motion.angle;
			projectile_motion.velocity = weapon.projectile_velocity * vec2(cos(projectile_motion.angle), sin(projectile_motion.angle));

			// Control weapon fire rate
			weapon.can_fire = false;
			weapon.elapsed_last_shot_time_ms = 0.f;
		}
	}

}

	/// <summary>
	/// helper function for terrain collision response, calculates the corrected player position
	/// </summary>
	/// <param name="position"> pass in either player.position.x or y</param>
float WorldSystem::positionCorrection(float position) {
	// only correct the position when the difference is between 0.05 to 0.
	// this is to account for case where left and down are both pressed when player is colliding wall to the left
	// in this case we would only want to correct the x position

	float whole = 0;

	if (position >= 0) {
		whole = floor(position);
		}
	else {
		whole = ceil(position);
		}
	
	float fraction = mod(position, whole);
	float offset = 0.02;

	// determine round up or down, adding offset to prevent continuous triggering
	if (position >= 0 && fraction <= 0.05) {
		position = floor(position) - offset;
		}
	else if (position < 0 && fraction >= -0.05) {
		position = ceil(position) + offset;
		}
	else {
		//no correction
		// 
		}

	return position;
}

void WorldSystem::spawn_items() {
	const int NUM_ITEM_TYPES = 4;

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
				createItem(renderer, spawn_location, ITEM_TYPE::WEAPON_GENERIC);
				break;
			case 3:
				createItem(renderer, spawn_location, ITEM_TYPE::UPGRADE);
				break;
		}
	}

	// TESTING: spawning a generic item
	createItem(renderer, get_random_spawn_location(), ITEM_TYPE::WEAPON_GENERIC);
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

	// Get unused spawn location within [-terrain->size_x/2, terrain->size_x/2] for x and 
	// [-terrain->size_y/2, terrain->size_y/2] for y
	while (true) {
		position.x = abs((int) rng()) % terrain->size_x + (-(terrain->size_x / 2));
		position.y = abs((int) rng()) % terrain->size_y + (-(terrain->size_y / 2));

		// Skip locations that are covered by spaceship
		if (position.x <= 1 && position.x >= -1 && position.y <= 2 && position.y >= -2) {
			continue;
		}

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

Entity WorldSystem::createAndEquipWeapon(ITEM_TYPE weapon_type) {
	// Reserve an entity
	auto entity = Entity();

	// Initialize the weapon
	auto& weapon = registry.weapons.emplace(entity);
	weapon.weapon_type = weapon_type;
	weapon.can_fire = true;
	weapon.elapsed_last_shot_time_ms = 0.f;

	switch (weapon_type) {
		case ITEM_TYPE::WEAPON_NONE:
			weapon.fire_rate = 0;
			weapon.projectile_velocity = 0.f;
			weapon.projectile_damage = 0;
			break;

		case ITEM_TYPE::WEAPON_GENERIC:
			weapon.fire_rate = 1000.f;
			weapon.projectile_velocity = 10.f;
			weapon.projectile_damage = 100;
			break;
	
		default:
			// TODO: better way of handling this
			printf("Debug info: Item is not a weapon. Ignoring.");
			break;
		}
	

	return entity;
}