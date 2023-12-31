// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <algorithm>


// Game configuration
const float IFRAMES = 1500;
const int FOOD_PICKUP_AMOUNT = 20;
float PLAYER_TOTAL_DISTANCE = 0;
const float FOOD_DECREASE_THRESHOLD  = 5.0f; // Adjust this value as needed
const float FOOD_DECREASE_RATE = 7.f;	// Decreases by 10 units per second (when moving)
float CURSOR_ANGLE = 0;
int PLAYER_DIRECTION = 2;  // Default to facing up
float ELAPSED_TIME = 0;
bool PLAYER_DEATH_FROM_FOOD = false; 


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
GLFWwindow* WorldSystem::create_window(ivec2& window_size) {
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

	window_size = { x, y };

	return window;
}



void WorldSystem::init(
	RenderSystem* renderer_arg, 
	TerrainSystem* terrain_arg, 
	WeaponsSystem* weapons_system_arg, 
	PhysicsSystem* physics_system_arg, 
	MobSystem* mob_system_arg, 
	AudioSystem* audio_system_arg,
	SpaceshipHomeSystem* spaceship_home_system_arg,
	QuestSystem* quest_system_arg,
	TutorialSystem* tutorial_system_arg,
	ParticleSystem* particle_system_arg,
	PowerupSystem* powerup_system_arg) {

	this->renderer = renderer_arg;
	this->terrain = terrain_arg;
	this->weapons_system = weapons_system_arg;
	this->mob_system = mob_system_arg;
	this->physics_system = physics_system_arg;
	this->audio_system = audio_system_arg;
	this->spaceship_home_system = spaceship_home_system_arg;
	this->quest_system = quest_system_arg;
	this->tutorial_system = tutorial_system_arg;
	this->particle_system = particle_system_arg;
	this->powerup_system = powerup_system_arg;

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

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());
	
	// Removing out of screen entities
	auto& motion_container = registry.motions;
	Motion& camera_motion = registry.motions.get(main_camera);

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	auto& player_component = registry.players.get(player_salmon);

	float min_timer_ms = 3000.f;
	// Handles when player dies or won the game 
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& timer = registry.deathTimers.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < min_timer_ms) {
			min_timer_ms = timer.timer_ms;
		}

		// restart the game once the death timer expired
		if (timer.timer_ms < 0) {
			if (spaceship_home_system->ALL_ITEMS_SUBMITTED){
				// pop up for victory 
				pop_up_text = createEndingTextPopUp(renderer, { 0,0 }, TEXTURE_ASSET_ID::VICTORY_TEXT); 
			}
			else {
				if (PLAYER_DEATH_FROM_FOOD) {
					pop_up_text = createEndingTextPopUp(renderer, { camera_motion.position.x,camera_motion.position.y }, TEXTURE_ASSET_ID::DEATH_TEXT_F);
				}
				else {
					pop_up_text = createEndingTextPopUp(renderer, { camera_motion.position.x,camera_motion.position.y}, TEXTURE_ASSET_ID::DEATH_TEXT_H);

				}
			}
			screen.screen_darken_factor = 0;
			return true;
		}
	}

	// Tick down iframes timer and health decrease timer
	
	
	player_component.iframes_timer -= elapsed_ms_since_last_update;

	if (player_component.iframes_timer < 0) {
		player_component.iframes_timer = 0;
		physics_system->isPlayerInvincible = false;
		//std::cout << "player is normalll..." << std::endl; //DELETE LATER

	}
		
	if (player_component.health_decrease_time < 0) {
		player_component.health_decrease_time = 0;
		camera_motion.angle = 0.f;
		camera_motion.scale = vec2(1.0, 1.0);
	} 
		
	if (player_component.food_decrease_time < 0) {
		player_component.food_decrease_time = 0;
	}

	if (player_component.health_decrease_time > 0) {
		player_component.health_decrease_time -= elapsed_ms_since_last_update;

			
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
		
	if (player_component.food_decrease_time > 0) {
		player_component.food_decrease_time -= elapsed_ms_since_last_update;

		Motion& food = registry.motions.get(food_bar);
		vec2 new_food_scale = vec2(((float)player_component.food / (float)PLAYER_MAX_FOOD) * FOOD_BAR_SCALE[0], FOOD_BAR_SCALE[1]);
		food.scale = interpolate(food.scale, new_food_scale, 1 - (player_component.food_decrease_time / IFRAMES));
	}
	


	ELAPSED_TIME += elapsed_ms_since_last_update;

	remaing_time_for_next_hunger_sound -= elapsed_ms_since_last_update;
	

	if (player_component.food < PLAYER_MAX_FOOD / 4 && remaing_time_for_next_hunger_sound <= 0) {
		audio_system->play_one_shot(AudioSystem::PLAYER_LOW_HUNGER);
		remaing_time_for_next_hunger_sound = low_hunger_sound_interval;
	}

	
	
	Motion& m = registry.motions.get(player_salmon);

	// Apply food decreasing the more you travel. 
	if (player_component.food > 0) {
		if (PLAYER_TOTAL_DISTANCE >= FOOD_DECREASE_THRESHOLD && !debugging.in_debug_mode) {
			// Decrease player's food by 1
			player_component.food -= 1;
			// Shrink the food bar
			player_component.food_decrease_time = IFRAMES;

			// Reset the total movement distance
			PLAYER_TOTAL_DISTANCE = 0;

		}
	}
	// else the food is below 0, player dies
	else if (!registry.deathTimers.has(player_salmon)) {
		registry.deathTimers.emplace(player_salmon);
		PLAYER_DEATH_FROM_FOOD = true;
	}


	// HEALTH BAR UI UPDATES
	Motion& health = registry.motions.get(health_bar);
	vec2 new_health_scale = vec2(((float)player_component.health / (float)PLAYER_MAX_HEALTH) * HEALTH_BAR_SCALE[0], HEALTH_BAR_SCALE[1]);
	health.scale = interpolate(health.scale, new_health_scale, 1 - (player_component.health_decrease_time / IFRAMES));
		
	

	// reduce window brightness if any of the present players is dying
	screen.screen_darken_factor = 1 - min_timer_ms / 3000;

	vec2 player_p = m.position;
	//Motion& f = registry.motions.get(fow);
	//f.position = m.position;


	// update spritesheet with aiming direction 
	updatePlayerDirection();

	// Player Movement code, build the velocity resulting from player movement
	handlePlayerMovement(elapsed_ms_since_last_update);

	// Camera movement mode
	Camera& c = registry.cameras.get(main_camera);
	if (c.mode_follow) {
		/*
		if (debugging.in_debug_mode)
			camera_motion.velocity = { 0,0 };
		else
		*/
		camera_motion.position = m.position;
	}
	else {
		handle_movement(camera_motion, CAMERA_LEFT);
	}
	// UI Movement
	for (Entity e : registry.screenUI.entities) {
		if (registry.motions.has(e)) {
			vec2& ui_inital_position = registry.screenUI.get(e);
			Motion& ui_motion = registry.motions.get(e);
			ui_motion.position = ui_inital_position + camera_motion.position;
		}
	}

	// Update pointing arrows
	for (int i = 0; i < registry.pointingArrows.size(); i++) {
		Entity e = registry.pointingArrows.entities[i];
		PointingArrow& arrow = registry.pointingArrows.components[i];

		Motion& arrow_m = registry.motions.get(e);
		vec2 local_space = arrow.radius_offset;
		Motion& target_m = registry.motions.get(arrow.target);
		vec2 target_p = target_m.position;
		vec2 delta_p = target_p - player_p;

		// Check if the target is nearby
		if (length(delta_p) > length(arrow.radius_offset)) {
			// We need to make a custom transform matrix
			float angle = atan2f(delta_p.y, delta_p.x);	// Angle that points towards the target

			Transform transform;
			transform.translate(player_p);	// Translate by player position
			transform.rotate(angle);		// Rotate the position of the arrow depending on where the ship is
			transform.scale({ 1, 1 });		// Constant scale to retain radius_offset distancing

			vec3 offset = vec3(arrow.radius_offset, 1.0f);

			arrow_m.position = vec2(transform.mat * offset);
			arrow_m.angle = angle + (M_PI / 2); // offset added because arrow points up initially 
			// instead of 90 degrees to the right
		}
		else {
			arrow_m.position = target_p;	// Hover over target
			arrow_m.angle = M_PI;			// Point down
		}
	}

	

	

	// Mob updates
	for (Entity entity : registry.mobs.entities) {
		// set the health bars to be below the mob
		Mob& mob = registry.mobs.get(entity);
		Motion& motion = registry.motions.get(entity);
		Motion& health = registry.motions.get(mob.health_bar);
		vec2& mob_position = motion.position;
		vec2 health_position = { mob_position.x, mob_position.y - 0.8 };
		health.position = health_position;

		// slow updates
		if (registry.mobSlowEffects.has(entity)) {
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

	// Player updates
	
	Motion& motion = registry.motions.get(player_salmon);

	// Knockback updates
	if (registry.playerKnockbackEffects.has(player_salmon)) {
		PlayerKnockbackEffect& playerKnockbackEffect = registry.playerKnockbackEffects.get(player_salmon);
			
		// Increment duration
		playerKnockbackEffect.elapsed_knockback_time_ms += elapsed_ms_since_last_update;
			
		// Set player velocity to zero and remove the PlayerKnockbackEffect component if knockback effect is over
		if (playerKnockbackEffect.duration_ms < playerKnockbackEffect.elapsed_knockback_time_ms) {
			motion.velocity = {0.f, 0.f};
			registry.playerKnockbackEffects.remove(player_salmon);

			printf("KNOCKBACK REMOVED\n");
		}
	}

	// Inaccuracy updates
	if (registry.playerInaccuracyEffects.has(player_salmon)) {
		PlayerInaccuracyEffect& playerInaccuracyEffect = registry.playerInaccuracyEffects.get(player_salmon);
			
		// Increment duration
		playerInaccuracyEffect.elapsed_inaccuracy_time_ms += elapsed_ms_since_last_update;
			
		// Remove the PlayerInaccuracyEffect component if knockback effect is over
		if (playerInaccuracyEffect.duration_ms < playerInaccuracyEffect.elapsed_inaccuracy_time_ms) {
			registry.playerInaccuracyEffects.remove(player_salmon);

			printf("INACCURACY REMOVED\n");
		}
	}

		

		

	
	
	if (registry.spaceships.has(spaceship_depart)) {
		update_spaceship_frame(elapsed_ms_since_last_update);
	}
	
	// Lets the editor drag
	if (debugging.in_debug_mode && editor_place_tile)
		map_editor_routine();

	// update for muzzle flash animation
	// change to next frame if not at last frame
	if (registry.animations.get(muzzleFlash).framex != 4) {
		registry.animations.get(muzzleFlash).framex += 1;
	}
	else {
	// set to transparent
		registry.colors.get(muzzleFlash).a = 0.0f;
	}
		

	return true;
}

void WorldSystem::updatePlayerDirection() {
	if (CURSOR_ANGLE >= -M_PI / 4 && CURSOR_ANGLE < M_PI / 4) 
		PLAYER_DIRECTION = 0;  // Weapom, Right
	else if (CURSOR_ANGLE >= M_PI / 4 && CURSOR_ANGLE < 3 * M_PI / 4) 
		PLAYER_DIRECTION = 1;  // Weapom, Down
	else if (CURSOR_ANGLE >= -3 * M_PI / 4 && CURSOR_ANGLE < -M_PI / 4) 
		PLAYER_DIRECTION = 2;  // Weapom, UP
	else 
		PLAYER_DIRECTION = 4;  // Weapom, left


	// Update player's direction
	registry.animations.get(player_salmon).framey = PLAYER_DIRECTION;
	
	}

void WorldSystem::handlePlayerMovement(float elapsed_ms_since_last_update) {
	// Check if any movement keys are pressed and if player is not dead
	bool anyMovementKeysPressed = keyDown[LEFT] || keyDown[RIGHT] || keyDown[UP] || keyDown[DOWN];
	auto& player_animation = registry.animations.get(player_salmon);

	// We'll consider moveVelocity existing in player space
	// Allow movement if player is not dead 
	// Movement code, build the velocity resulting from player movement
	if (!registry.deathTimers.has(player_salmon) && !registry.playerKnockbackEffects.has(player_salmon)) {
		Motion& m = registry.motions.get(player_salmon);
		m.velocity = { 0, 0 };

		handle_movement(m, LEFT);

		if (length(m.velocity) > 0) {

			float speedRatio = terrain->get_terrain_speed_ratio(terrain->get_cell(m.position));

			if (speedRatio == 0.25f || speedRatio == 0.40f) {
				particle_system->createWaterSplash(player_salmon,1, speedRatio);
			}

			m.velocity *= speedRatio;
		}

		if (anyMovementKeysPressed) {
			PLAYER_TOTAL_DISTANCE += FOOD_DECREASE_RATE * elapsed_ms_since_last_update / 1000.f;

			if (ELAPSED_TIME > 100) {
				// Update walking animation
				player_animation.framex = (player_animation.framex + 1) % 4;
				ELAPSED_TIME = 0.0f; // Reset the timer
				}
		} else {
			// No movement keys pressed, set back to the first frame
			player_animation.framex = 0;
			}
		}
	else if (registry.deathTimers.has(player_salmon)) {
		// Player is dead, do not allow movement
		Motion& m = registry.motions.get(player_salmon);
		Player& player = registry.players.get(player_salmon);
		m.velocity = { 0, 0 };

		player_animation.framey = 3;

	
		if (player.health <=  0) {
			Motion& health = registry.motions.get(health_bar);
			health.scale = { 0,0 };
		}

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

		auto& player = registry.players.get(player_salmon);

		motion.velocity = moveVelocity * player.current_speed * invert;
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the items submmited 
	spaceship_home_system->ALL_ITEMS_SUBMITTED = false;
	
	bool PLAYER_DEATH_FROM_FOOD = false;
	renderer->enableFow = 1;
	renderer->fow_radius = 4.5f;

	while (registry.deathTimers.entities.size() > 0)
		registry.remove_all_components_of(registry.deathTimers.entities.back());

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
			physics_system->createDefaultCollider(e);
	}

	// THIS MUST BE CALL AFTER TERRAIN COLLIDER CREATION AND BEFORE ALL OTHER ENTITY CREATION
	// build the static BVH with all terrain colliders.
	physics_system->initStaticBVH(registry.colliders.size());

	// Create Spaceship
	spaceship = createSpaceship(renderer, { 0,-2.5 });

	// Create a new salmon
	player_salmon = createPlayer(renderer, physics_system, { 0, 0 });
	registry.colors.insert(player_salmon, { 1, 0.8f, 0.8f , 1.f});

	// Create the main camera
	main_camera = createCamera({ 0,0 });

	// Create arrow pointing back to the ship
	ship_arrow = createPointingArrow(renderer, player_salmon, spaceship);

	// Reset the spaceship home system
	spaceship_home_system->resetSpaceshipHomeSystem(SPACESHIP_MAX_HEALTH_STORAGE, SPACESHIP_MAX_FOOD_STORAGE, SPACESHIP_MAX_AMMO_STORAGE);

	// DISABLE FOW MASK
	//fow = createFOW(renderer, { 0,0 });

	// Create player health bar
	health_bar = createBar(renderer, HEALTH_BAR_FRAME_POS, PLAYER_MAX_HEALTH, BAR_TYPE::HEALTH_BAR);



	health_frame = createFrame(renderer, HEALTH_BAR_FRAME_POS, FRAME_TYPE::HEALTH_FRAME);

	// Create player food bar
	food_bar = createBar(renderer, FOOD_BAR_FRAME_POS, PLAYER_MAX_FOOD, BAR_TYPE::FOOD_BAR);
	food_frame = createFrame(renderer, FOOD_BAR_FRAME_POS, FRAME_TYPE::FOOD_FRAME);

	// Reset the weapon indicator
	user_has_first_weapon = false;

	weapons_system->createNonselectedWeaponIndicators();



	// Reset quest system
	std::vector<QUEST_ITEM_STATUS> statuses(4, QUEST_ITEM_STATUS::NOT_FOUND);
	quest_system->resetQuestSystem(statuses);

	// Reset tutorial system
	tutorial_system->resetTutorialSystem();
	tutorial_system->openHelpDialog();

	// clear all used spawn locations
	used_spawn_locations.clear();

	// FOR DEMO - to show different types of items being created.	
 
	// TODO: uncomment these after messing w/ map editor
	spawn_items();
	mob_system->spawn_mobs();
	//createItem(renderer, physics_system, {5.f, 3.f}, ITEM_TYPE::POWERUP_SPEED);
	//createItem(renderer, physics_system, {5.f, 5.f}, ITEM_TYPE::POWERUP_HEALTH);

	// for movement velocity
	for (int i = 0; i < KEYS; i++)
	  keyDown[i] = false;

	muzzleFlash = createMuzzleFlash(renderer, vec2(0.f));

	// Reset powerups system. Must call this after creating player since player entity is stored as a cached in powerup system
	powerup_system->resetPowerupSystem(player_salmon);

	// Debugging for memory/component leaks
	registry.list_all_components();
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	vec2 hasCorrectedDirection = {0,0};
	int correctionCount = 0;

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
					// discard all player vs mob collision
					for (int i = 0; i < collisionsRegistry.size(); i++) {
						if (registry.players.has(collisionsRegistry.entities[i]) && registry.mobs.has(collisionsRegistry.components[i].other_entity))
							collisionsRegistry.remove(collisionsRegistry.entities[i]);
						//std::cout << "removed one colliisions." << std::endl;

					}

					// set player to ignore mob collision
					physics_system->isPlayerInvincible = true;
					//std::cout << "player is invincible......" << std::endl;

					// skip damage
					break;
				}

				Mob& mob = registry.mobs.get(entity_other);
				player.health = max(0, player.health - mob.damage);
				mob_system->apply_mob_attack_effects(entity, entity_other);

				// Shrink the health bar
				player.health_decrease_time = IFRAMES; // player's health decays over this period of time, using interpolation
				// use the same amount as iframes so health is never "out of date"

				// Give the player some frames of invincibility so that they cannot die instantly when running into a mob
				player.iframes_timer = IFRAMES;

				if (player.health <= 0) {
					audio_system->play_one_shot(AudioSystem::PLAYER_DEATH);
					if (!registry.deathTimers.has(entity)) {
						// TODO: game over screen
						registry.deathTimers.emplace(entity);
					}
				}
				else {
					if (player.health <= PLAYER_MAX_HEALTH * 0.25f)
						audio_system->play_one_shot(AudioSystem::PLAYER_LOW_HEALTH);
					else
						audio_system->play_one_shot(AudioSystem::PLAYER_HIT);
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
					case ITEM_TYPE::QUEST_TWO:
					case ITEM_TYPE::QUEST_THREE:
					case ITEM_TYPE::QUEST_FOUR:
						quest_system->processQuestItem(item.data, QUEST_ITEM_STATUS::FOUND);
						audio_system->play_one_shot(AudioSystem::QUEST_PICKUP);
						
						if (!player.has_collected_quest_item) {
							tutorial_system->createTutorialDialog(TUTORIAL_TYPE::QUEST_ITEM_TUTORIAL);
							player.has_collected_quest_item = true;
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
						weapons_system->increaseAmmo(ITEM_TYPE::WEAPON_SHURIKEN, 5);
						audio_system->play_one_shot(AudioSystem::RELOAD_SHURIKEN);
						break;
					case ITEM_TYPE::WEAPON_CROSSBOW:
						weapons_system->increaseAmmo(ITEM_TYPE::WEAPON_CROSSBOW, 5);
						audio_system->play_one_shot(AudioSystem::RELOAD_CROSSBOW);
						break;
					case ITEM_TYPE::WEAPON_SHOTGUN:
						weapons_system->increaseAmmo(ITEM_TYPE::WEAPON_SHOTGUN, 3);
						audio_system->play_one_shot(AudioSystem::RELOAD_SHOTGUN);
						break;
					case ITEM_TYPE::WEAPON_MACHINEGUN:
						weapons_system->increaseAmmo(ITEM_TYPE::WEAPON_MACHINEGUN, 10);
						audio_system->play_one_shot(AudioSystem::RELOAD_MG);
						break;
					case ITEM_TYPE::WEAPON_UPGRADE:
						weapons_system->upgradeWeapon();
						break;

					// POWER UP
					case ITEM_TYPE::POWERUP_SPEED:
						powerup_system->applyPowerup(POWERUP_TYPE::SPEED);
						break;

					case ITEM_TYPE::POWERUP_HEALTH:
						powerup_system->applyPowerup(POWERUP_TYPE::HEALTH_REGEN);
						break;

					case ITEM_TYPE::POWERUP_INVISIBLE:
						powerup_system->applyPowerup(POWERUP_TYPE::INVISIBLE);
						break;

					case ITEM_TYPE::POWERUP_INFINITE_BULLET:
						powerup_system->applyPowerup(POWERUP_TYPE::INFINITE_BULLET);
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

				// blood splash particle effects
				particle_system->createParticleSplash(entity, entity_other, 10, collisionsRegistry.components[i].MTV);

				Mob& mob = registry.mobs.get(entity_other);

				// Mob takes damage. Kill if no hp left.
				mob.health -= projectile.damage;
				// printf("mob health: %i", mob.health);
				if (mob.health <= 0) {
					registry.remove_all_components_of(mob.health_bar);
					registry.remove_all_components_of(entity_other);
					audio_system->play_one_shot(AudioSystem::MOB_DEATH);
				}
				else {
					audio_system->play_one_shot(AudioSystem::MOB_HIT);
					Motion& health = registry.motions.get(mob.health_bar);
					health.scale = vec2(((float)mob.health / (float)mob_system->mob_health_map.at(mob.type)) * 2.5, 0.3);
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

void WorldSystem::update_spaceship_frame(float elapsed_ms_since_last_update) {
	auto& a = registry.animations.get(spaceship_depart);

	if (ELAPSED_TIME > 300 || a.framex == 5) {
		// Update walking animation
		if (a.framex != 5) {

			a.framex = (a.framex + 1) % 6;
			ELAPSED_TIME = 0.0f; // Reset the timer
		}
		// spaceship departs 
		if (a.framex == 5) {
			Motion& spaceship_motion = registry.motions.get(spaceship_depart);
			spaceship_motion.velocity += vec2{ 0,-0.5 };
			debugging.in_debug_mode = !debugging.in_debug_mode;
			//renderer->enableFow = 0;
			if (renderer->fow_radius <= 17)
				renderer->fow_radius += 0.1;
			ELAPSED_TIME = 0.0f;
		}
	}
}

void WorldSystem::update_spaceship_depart() {
	if (spaceship_home_system->ALL_ITEMS_SUBMITTED) {
		// Remove all ship parts if all items are collected
		while (registry.spaceships.entities.size() > 0)
			registry.remove_all_components_of(registry.spaceships.entities.back());
		// Remove arrow 
		registry.pointingArrows.remove(ship_arrow); 
		registry.renderRequests.remove(ship_arrow);
		// create depart spaceship
		spaceship_depart = createSpaceshipDepart(renderer); 

		// Clear powerups
		registry.powerups.clear();
		// remove player		
		registry.renderRequests.remove(player_salmon);
		// disable player movements 
		registry.deathTimers.emplace(player_salmon); 
	}
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	Motion& player_motion = registry.motions.get(player_salmon);
	Player& player = registry.players.get(player_salmon);

	// Movement with velocity handled in step function  
	update_key_presses(key, action);

	// Saving and reloading
	if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		// Load the game state 
		std::ifstream f("save.json");
		json data = json::parse(f);

		std::cout << "Loading ...\n";

		load_game(data);
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_K) {
		// Save the game state (player location, weapon, health, food, mobs & location)
		Player& player = registry.players.get(player_salmon);
		Motion& player_motion = registry.motions.get(player_salmon);
		SpaceshipHome& spaceship_home_info = registry.spaceshipHomes.components[0];
		Inventory& inventory = registry.inventories.get(player_salmon);
		ITEM_TYPE active_weapon = weapons_system->getActiveWeapon();

		std::vector<Weapon> weapons;
		for (auto& weapon_entity : registry.weapons.entities) {
			weapons.push_back(registry.weapons.get(weapon_entity));
		}

		std::vector<std::pair<Mob&, Motion&>> mobs;
		for (auto& mob : registry.mobs.entities) {
			mobs.push_back({ registry.mobs.get(mob), registry.motions.get(mob) });
		}

		std::vector<std::pair<Item&, Motion&>> items;
		for (auto& item : registry.items.entities) {
			items.push_back({ registry.items.get(item), registry.motions.get(item) });
		}

		std::vector<QUEST_ITEM_STATUS> quest_item_statuses{
			inventory.quest_items[ITEM_TYPE::QUEST_ONE],
			inventory.quest_items[ITEM_TYPE::QUEST_TWO],
			inventory.quest_items[ITEM_TYPE::QUEST_THREE],
			inventory.quest_items[ITEM_TYPE::QUEST_FOUR],
		};

		std::vector<Powerup> powerups;
		for (auto& powerup_component : registry.powerups.components) {
			powerups.push_back(powerup_component);
		}
		

		SaveGame(player, player_motion, active_weapon, weapons, mobs, items, quest_item_statuses, spaceship_home_info, powerups, powerup_system->old_speed, player.current_speed);

		tutorial_system->createTutorialText(TUTORIAL_TYPE::GAME_SAVED);
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		restart_game();
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		if (tutorial_system->isHelpDialogOpen()) {
			tutorial_system->closeHelpDialog();
		}
		else if (player.is_home) {
			spaceship_home_system->exitSpaceship();
			audio_system->play_one_shot(AudioSystem::SHIP_LEAVE);
			update_spaceship_depart();
		}
		else {
			glfwSetWindowShouldClose(window, true);
		}
	}
	if (registry.spaceships.has(spaceship)) {

		if (action == GLFW_PRESS && key == GLFW_KEY_E) {
			// Enter ship if player is near
			if (tutorial_system->isPlayerNearSpaceship(player_motion.position, registry.motions.get(spaceship).position) && !player.is_home) {
				spaceship_home_system->enterSpaceship();
				audio_system->play_one_shot(AudioSystem::SHIP_ENTER);

				if (!player.has_entered_spaceship) {
					tutorial_system->createTutorialDialog(TUTORIAL_TYPE::SPACESHIP_HOME_TUTORIAL);
					player.has_entered_spaceship = true;
				}
			}
		}
	}




	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
		debugging.hide_ui = !debugging.hide_ui;

	// Level editor controls
	if (debugging.in_debug_mode && action == GLFW_PRESS) {
		process_editor_controls(action, key);
	}

	// Press B to toggle debug mode
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		debugging.in_debug_mode = !debugging.in_debug_mode;
		if (renderer->enableFow == 0) {
			renderer->enableFow = 1;
		}
		else {
			renderer->enableFow = 0;
		}
		
	}

	// Press B to toggle debug mode
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		std::cout << "player at x: " << registry.motions.get(player_salmon).position.x << ", y: " << registry.motions.get(player_salmon).position.y << std::endl;

	}

	// Control the current speed with `<` `>` as well as fow radius
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		player.current_speed -= 0.1f;
		printf("Current speed = %f\n", player.current_speed);

		renderer->fow_radius -= 0.3f;
		if (renderer->fow_radius < 0)
			renderer->fow_radius = 0;
		std::cout << "current fog radius " << renderer->fow_radius << std::endl;

	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		player.current_speed += 0.1f;
		printf("Current speed = %f\n", player.current_speed);

		renderer->fow_radius += 0.3f;
		std::cout << "current fog radius " << renderer->fow_radius << std::endl;

	}
	player.current_speed = fmax(0.f, player.current_speed);

	// Hotkeys to equip weapons
	bool equipped_weapon_key_pressed = false;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		weapons_system->setActiveWeapon(ITEM_TYPE::WEAPON_SHURIKEN);
		equipped_weapon_key_pressed = true;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		weapons_system->setActiveWeapon(ITEM_TYPE::WEAPON_CROSSBOW);
		equipped_weapon_key_pressed = true;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		weapons_system->setActiveWeapon(ITEM_TYPE::WEAPON_SHOTGUN);
		equipped_weapon_key_pressed = true;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		weapons_system->setActiveWeapon(ITEM_TYPE::WEAPON_MACHINEGUN);
		equipped_weapon_key_pressed = true;	}

	// Checking if we need to display help pop up
	if (equipped_weapon_key_pressed && !user_has_first_weapon) {
		// Has just equipped up the first weapon
		user_has_first_weapon = true;
	}
	equipped_weapon_key_pressed = false;

	// TESTING: hotkey to upgrade weapon
	if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		weapons_system->upgradeWeapon();
	}
}
void WorldSystem::process_editor_controls(int action, int key)
{
	if (key == GLFW_KEY_KP_1) {	// numpad 1
		// Toggle collidable flag
		editor_flag ^= TERRAIN_FLAGS::COLLIDABLE;
		if (editor_flag & TERRAIN_FLAGS::COLLIDABLE)
			std::cout << "New terrain are collidable" << std::endl;
		else
			std::cout << "New terrain are non-collidable" << std::endl;
	}
	if (key == GLFW_KEY_KP_2) {	// numpad 2
		// Toggles pathfindable flag
		editor_flag ^= TERRAIN_FLAGS::DISABLE_PATHFIND;
		if (editor_flag & TERRAIN_FLAGS::DISABLE_PATHFIND)
			std::cout << "New terrain is now accessible for regular mobs" << std::endl;
		else
			std::cout << "New terrain will not be accessible for regular mobs" << std::endl;
	}
	if (key == GLFW_KEY_KP_3) {	// numpad 3
		// Toggles pathfindable flag
		editor_flag ^= TERRAIN_FLAGS::ALLOW_SPAWNS;
		if (editor_flag & TERRAIN_FLAGS::ALLOW_SPAWNS)
			std::cout << "New terrain allows random item and mob spawning" << std::endl;
		else
			std::cout << "New terrain will not allow random spawning" << std::endl;
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
	if (key == GLFW_KEY_PAGE_UP) { // PageUp key
		// This expands the map to world_size_x, world_size_y.
		// Make sure you disable item and mob spawning because physics and pathfinding
		// will break!!
		assert(registry.mobs.entities.empty());
		assert(registry.items.entities.empty());
		

		terrain->expand_map(world_size_x, world_size_y);
		renderer->empty_terrain_buffer();

		std::unordered_map<unsigned, RenderSystem::ORIENTATIONS> orientations;
		terrain->generate_orientation_map(orientations);
		renderer->initializeTerrainBuffers(orientations);

		for (unsigned int i = 0; i < registry.terrainCells.entities.size(); i++) {
		Entity e = registry.terrainCells.entities[i];
		TerrainCell& cell = registry.terrainCells.components[i];

		if (cell.flag & TERRAIN_FLAGS::COLLIDABLE)
			physics_system->createDefaultCollider(e);
		}

		physics_system->initStaticBVH(registry.colliders.size());
	}
}
/// <summary>
/// Rotates player to follow mouse movement
/// </summary>
/// <param name="mouse_position"> Location of mouse on game screen</param>
void WorldSystem::on_mouse_move(vec2 mouse_position) {
	vec2 mouse_pos_clip = screen_to_clip_coords(mouse_position);

	// Disable rotation when the player dies
	if (!registry.deathTimers.has(player_salmon)) {
		// The player is always in the middle of the screen so we need to compute the 
		// rotation angle w.r.t. the centre of the screen
		ivec2 window_size = renderer->window_resolution;
		vec2 cursor = mouse_position * renderer->screen_to_window_correction;

		float screen_centre_x = window_size.x/2;
		float screen_centre_y = window_size.y/2;


		Motion& motion = registry.motions.get(player_salmon);
		CURSOR_ANGLE = atan2(cursor.y - screen_centre_y, cursor.x - screen_centre_x);
	}

	// Change mouse cursor type if hovering over help button or storage item
	if (tutorial_system->isMouseOverHelpButton(mouse_pos_clip) || 
		spaceship_home_system->isHome() && !tutorial_system->isHelpDialogOpen() && spaceship_home_system->isMouseOverAnyStorageItem(mouse_pos_clip)
	) {
		GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		glfwSetCursor(window, cursor);
	} else {
		glfwSetCursor(window, NULL);
	}
}


/// <summary>
/// Function to handle mouse click (weapon fire)
/// </summary>
void WorldSystem::on_mouse_click(int button, int action, int mods) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	vec2 mouse_pos_clip = screen_to_clip_coords({ xpos, ypos });

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// Open/close help dialog when help button is clicked
		if (!tutorial_system->isHelpDialogOpen() && tutorial_system->isMouseOverHelpButton(mouse_pos_clip)) {
			tutorial_system->openHelpDialog();
			return;
		} else if (tutorial_system->isHelpDialogOpen() && tutorial_system->isMouseOverHelpButton(mouse_pos_clip)) {
			tutorial_system->closeHelpDialog();
			return;
		}

		// Regenerate stat if spaceship storage item is clicked
		if (spaceship_home_system->isHome() && !tutorial_system->isHelpDialogOpen()) {
			if (spaceship_home_system->isMouseOverStorageItem(mouse_pos_clip, TEXTURE_ASSET_ID::SPACESHIP_HOME_AMMO)) {
				spaceship_home_system->regenerateStat(RESOURCE_TYPE::AMMO, food_bar, health_bar);
			} else if (spaceship_home_system->isMouseOverStorageItem(mouse_pos_clip, TEXTURE_ASSET_ID::SPACESHIP_HOME_FOOD)) {
				spaceship_home_system->regenerateStat(RESOURCE_TYPE::FOOD, food_bar, health_bar);
			} else if (spaceship_home_system->isMouseOverStorageItem(mouse_pos_clip, TEXTURE_ASSET_ID::SPACESHIP_HOME_HEALTH)) {
				spaceship_home_system->regenerateStat(RESOURCE_TYPE::HEALTH, food_bar, health_bar);
			}
		}

		if (!registry.deathTimers.has(player_salmon) && !spaceship_home_system->isHome() && !tutorial_system->isHelpDialogOpen()) {
			// if theres ammo in current weapon 
			Motion& player_motion = registry.motions.get(player_salmon);

			float projectileSpawnOffset = 1.4f;
			// Play appropriate shooting noises if we've just shot
			// Added offset to spawn projectile a bit outward. 
			ITEM_TYPE fired_weapon = weapons_system->fireWeapon(player_motion.position.x + projectileSpawnOffset * cos(CURSOR_ANGLE), player_motion.position.y + projectileSpawnOffset * sin(CURSOR_ANGLE), CURSOR_ANGLE);
			
			// If we successfully shot...
			if (fired_weapon != ITEM_TYPE::WEAPON_NONE) {

				emitMuzzleFlash(player_motion.position, PLAYER_DIRECTION);

				switch (fired_weapon) {
				case ITEM_TYPE::WEAPON_SHOTGUN:
					audio_system->play_one_shot(AudioSystem::SHOT); break;
				case ITEM_TYPE::WEAPON_MACHINEGUN:
					audio_system->play_one_shot(AudioSystem::SHOT_MG); break;
				case ITEM_TYPE::WEAPON_CROSSBOW:
					audio_system->play_one_shot(AudioSystem::SHOT_CROSSBOW); break;
				case ITEM_TYPE::WEAPON_SHURIKEN:
					audio_system->play_one_shot(AudioSystem::SHOT_SHURIKEN); break;
				}
			
			}
			// We didn't shoot successfully since we ran out of ammo
			else if (weapons_system->getActiveWeaponAmmoCount() <= 0 && player_equipped_weapon && !powerup_system->disable_bullet_consumption) {
				fired_weapon = weapons_system->getActiveWeapon();

				if (fired_weapon == ITEM_TYPE::WEAPON_NONE)
					return;

				switch (fired_weapon) {
					case ITEM_TYPE::WEAPON_SHOTGUN:
						audio_system->play_one_shot(AudioSystem::EMPTY_SHOTGUN); break;
					case ITEM_TYPE::WEAPON_MACHINEGUN:
						audio_system->play_one_shot(AudioSystem::EMPTY_MG); break;
					case ITEM_TYPE::WEAPON_CROSSBOW:
						audio_system->play_one_shot(AudioSystem::EMPTY_CROSSBOW); break;
				}
			}
		}
	}
	if (debugging.in_debug_mode) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)
				editor_place_tile = true;
			else if (action == GLFW_RELEASE)
				editor_place_tile = false;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			ivec2 pos = terrain->quantize_vec2(mouse_pos_clip);
			std::cout << "Clicked tile: (" << pos.x << ", " << pos.y << ')' << std::endl;
		}
	}
}

void WorldSystem::map_editor_routine() {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);	// For some reason it only supports doubles!	
	vec2 mouse_pos = screen_to_clip_coords({ xpos, ypos });

	Entity tile = terrain->get_cell(mouse_pos);
	TerrainCell& cell = registry.terrainCells.get(tile);
	bool to_collidable = (editor_flag & COLLIDABLE);
	bool from_collidable = (cell.flag & COLLIDABLE);
	uint32_t data = ((uint32_t)editor_terrain << 16) | editor_flag;

	// DEBUG: print out the tile coordinates
	// printf("x: %f, y: %f\n", mouse_pos.x, mouse_pos.y);

	if (cell != data) {
		cell.from_uint32(data);

		// Update collisions
		if (to_collidable != from_collidable) {
			if (to_collidable) {
				physics_system->createDefaultCollider(tile);
			}
			else {
				registry.colliders.remove(tile);
			}
			physics_system->initStaticBVH(registry.colliders.size());
		}

		terrain->update_tile(tile, cell, true);	// true because we need to update adjacent cells too
	}
}

vec2 WorldSystem::screen_to_clip_coords(vec2 point) {
	point *= renderer->screen_to_window_correction;
	mat3 view_ = renderer->createModelMatrix(main_camera);

	// You can cache this to save performance.
	mat3 proj_ = inverse(renderer->createScaledProjectionMatrix());

	ivec2 screen = renderer->window_resolution;

	// Recall that valid clip coordinates are between [-1, 1]. 
	// First, we need to turn screen (pixel) coordinates into clip coordinates:
	vec3 clip_coords = {
		(point.x / screen.x) * 2 - 1,		// Get the fraction of the x pos in the screen, multiply 2 to map range to [0, 2], 
												// then offset so the range is now [-1, 1].
		-(point.y / screen.y) * 2 + 1,		// Same thing, but recall that the y direction is opposite in glfw.
		1.0 };									// Denote that this is a point.
	clip_coords = view_ * proj_ * clip_coords;

	return {clip_coords.x, clip_coords.y};
}

void WorldSystem::spawn_items() {
	// lookup table for weapon upgrades per zone
	std::unordered_map<ZONE_NUMBER,int> zone_weapon_upgrades = {
		{ZONE_0, 0},
		{ZONE_1, 0},    
		{ZONE_2, 2},	
		{ZONE_3, 8},
		{ZONE_4, 12},
		{ZONE_5, 12},
	};

	// lookup table for food per zone
	std::unordered_map<ZONE_NUMBER,int> zone_food = {
		{ZONE_0, 3},
		{ZONE_1, 10},    
		{ZONE_2, 20},	
		{ZONE_3, 40},
		{ZONE_4, 40},
		{ZONE_5, 40},
	};

	// lookup table for ammo
	std::unordered_map<ZONE_NUMBER,int> zone_ammo = {
		{ZONE_0, 0},
		{ZONE_1, 2},    
		{ZONE_2, 3},	
		{ZONE_3, 7},
		{ZONE_4, 8},
		{ZONE_5, 16},
	};

	// lookup table for powerup
	std::unordered_map<ZONE_NUMBER, int> zone_powerup = {
		{ZONE_0, 0},
		{ZONE_1, 0},
		{ZONE_2, 2},
		{ZONE_3, 5},
		{ZONE_4, 7},
		{ZONE_5, 10},
	};


	for (int zone_num = ZONE_0; zone_num != ZONE_COUNT; zone_num++) {
		ZONE_NUMBER zone = static_cast<ZONE_NUMBER>(zone_num);

		// spawn the weapon upgrades 
		for (int i = 0; i < zone_weapon_upgrades[zone]; i++) {
			createItem(renderer, physics_system, terrain->get_random_terrain_location(zone), ITEM_TYPE::WEAPON_UPGRADE);
		}

		// spawn food
		for (int i = 0; i < zone_food[zone]; i++) {
			createItem(renderer, physics_system, terrain->get_random_terrain_location(zone), ITEM_TYPE::FOOD);
		}

		// spawn ammo
		std::vector<ITEM_TYPE> weapons = {ITEM_TYPE::WEAPON_SHURIKEN, ITEM_TYPE::WEAPON_CROSSBOW, ITEM_TYPE::WEAPON_SHOTGUN, ITEM_TYPE::WEAPON_MACHINEGUN};
		for (auto& weapon : weapons) {
			for (int i = 0; i < zone_ammo[zone]; i++) {
				createItem(renderer, physics_system, terrain->get_random_terrain_location(zone), weapon);
			}
		}

		// spawn powerup
		std::vector<ITEM_TYPE> powerups = { ITEM_TYPE::POWERUP_SPEED, ITEM_TYPE::POWERUP_HEALTH, ITEM_TYPE::POWERUP_INVISIBLE, ITEM_TYPE::POWERUP_INFINITE_BULLET};
		for (auto& powerup : powerups) {
			for (int i = 0; i < zone_powerup[zone]; i++) {
				createItem(renderer, physics_system, terrain->get_random_terrain_location(zone), powerup);
			}
		}

	}

 
	//Hardcoded powerup spawns
	
	// TOP LEFT REGION
	powerup_spawn_helper(vec2{ -5.00f, -92.0f });
	powerup_spawn_helper(vec2{ -30.0f, -50.0f }); // water
	powerup_spawn_helper(vec2{ -34.0f, -88.0f });
	powerup_spawn_helper(vec2{ -39.0f, -55.0f });
	powerup_spawn_helper(vec2{ -49.0f, -74.0f });
	powerup_spawn_helper(vec2{ -52.0f, -34.0f });
	powerup_spawn_helper(vec2{ -56.0f, -81.0f });
	powerup_spawn_helper(vec2{ -57.0f, -58.0f });
	powerup_spawn_helper(vec2{ -60.0f, -45.0f });
	powerup_spawn_helper(vec2{ -60.0f, -69.0f });
	powerup_spawn_helper(vec2{ -64.0f, -94.0f });
	powerup_spawn_helper(vec2{ -65.0f, -94.0f });
	powerup_spawn_helper(vec2{ -80.0f, -93.0f });
	powerup_spawn_helper(vec2{ -82.0f, -43.0f });
	powerup_spawn_helper(vec2{ -84.0f, -62.0f });
	powerup_spawn_helper(vec2{ -91.0f, -69.0f });
	powerup_spawn_helper(vec2{ -91.0f, -69.0f });
	powerup_spawn_helper(vec2{ -93.0f, -78.0f });
	powerup_spawn_helper(vec2{ -96.0f, -24.0f }); // water


	// BOTTOM LEFT REGION


	powerup_spawn_helper(vec2{ -15.0f, 35.0f }); // center ground
	powerup_spawn_helper(vec2{ -37.0f, 72.0f });
	powerup_spawn_helper(vec2{ -72.0f, 33.0f });
	powerup_spawn_helper(vec2{ -83.0f, 39.0f }); // bottom left cave
	powerup_spawn_helper(vec2{ -90.0f, 61.0f });

	// BOTTOM RIGHT REGION
	powerup_spawn_helper(vec2{ 3.00f, 20.0f });
	powerup_spawn_helper(vec2{ 76.0f, 6.0f });
	powerup_spawn_helper(vec2{ 23.0f, 13.0f });
	powerup_spawn_helper(vec2{ 69.0f, 42.0f });
	powerup_spawn_helper(vec2{ 83.0f, 30.0f });
	powerup_spawn_helper(vec2{ 96.0f, 2.00f });


	// TOP RIGHT REGION
	powerup_spawn_helper(vec2{ 8.00f, -81.0f });
	powerup_spawn_helper(vec2{ 33.0f, -38.0f });
	powerup_spawn_helper(vec2{ 43.0f, -92.0f });
	powerup_spawn_helper(vec2{ 54.0f, -92.0f });
	powerup_spawn_helper(vec2{ 75.0f, -12.0f });
	powerup_spawn_helper(vec2{ 91.0f, -90.0f });
	powerup_spawn_helper(vec2{ 96.0f, -18.0f });




	// Hardcoded quest items
	createItem(renderer, physics_system, { -78.f, -84.f }, ITEM_TYPE::QUEST_ONE);
	createItem(renderer, physics_system, { 55.f, -52.f }, ITEM_TYPE::QUEST_TWO);
	createItem(renderer, physics_system, { 78.f, 48.f }, ITEM_TYPE::QUEST_THREE);
	createItem(renderer, physics_system, { -73.f, 62.f }, ITEM_TYPE::QUEST_FOUR);


	
}

// Adapted from restart_game, BASICALLY a lot of optional arguments to change small things :D
void WorldSystem::load_game(json j) {
	vec2 player_location = { j["player_motion"]["position_x"], j["player_motion"]["position_y"] };

	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("START LOADING\n");

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// These weapons don't have a motions so let's kill them all!
	while (registry.weapons.entities.size() > 0)
		registry.remove_all_components_of(registry.weapons.entities.back());


	renderer->enableFow = 1;
	renderer->fow_radius = 4.5f;


	// Reset the weapons system
	weapons_system->resetWeaponsSystem();

	// Reset the terrain system
	terrain->resetTerrainSystem();

	

	// Re-initialize the terrain
	terrain->init(loaded_map_name, renderer);

	// PRESSURE TESTING FOR BVH, can remove later
	//terrain->init(512, 512, renderer);
	
	// Add wall of stone around the map
	for (unsigned int i = 0; i < registry.terrainCells.entities.size(); i++) {
		Entity e = registry.terrainCells.entities[i];
		TerrainCell& cell = registry.terrainCells.components[i];

		if (cell.flag & TERRAIN_FLAGS::COLLIDABLE)
			physics_system->createDefaultCollider(e);
		}

	// THIS MUST BE CALL AFTER TERRAIN COLLIDER CREATION AND BEFORE ALL OTHER ENTITY CREATION
	// build the static BVH with all terrain colliders.
	physics_system->initStaticBVH(registry.colliders.size());

	// Create a Spaceship 
	spaceship = createSpaceship(renderer, { 0, -2.5 });




	// Create a new salmon
	player_salmon = createPlayer(renderer, physics_system, player_location);
	Player& player = registry.players.get(player_salmon);
	int p_health = j["player"]["health"];
	int p_food = j["player"]["food"];
	bool p_is_home = j["player"]["is_home"];
	bool p_has_collected_quest_item = j["player"]["has_collected_quest_item"];
	bool p_has_entered_spaceship = j["player"]["has_entered_spaceship"];
	float p_current_speed = j["player"]["current_speed"];
	player.health = p_health;
	player.food = p_food;
	player.is_home = p_is_home;
	player.has_collected_quest_item = p_has_collected_quest_item;
	player.has_entered_spaceship = p_has_entered_spaceship;
	player.current_speed = p_current_speed;
	registry.colors.insert(player_salmon, { 1, 0.8f, 0.8f, 1.0f});

	// Create the main camera
	main_camera = createCamera(player_location);
	Motion& camera_motion = registry.motions.get(main_camera);

	// Create arrow pointing back to the ship
	ship_arrow = createPointingArrow(renderer, player_salmon, spaceship);

	// Create player health bar
	health_bar = createBar(renderer, HEALTH_BAR_FRAME_POS, PLAYER_MAX_HEALTH, BAR_TYPE::HEALTH_BAR);
	health_frame = createFrame(renderer, HEALTH_BAR_FRAME_POS, FRAME_TYPE::HEALTH_FRAME);

	// Create player food bar
	food_bar = createBar(renderer, FOOD_BAR_FRAME_POS, PLAYER_MAX_FOOD, BAR_TYPE::FOOD_BAR);
	food_frame = createFrame(renderer, FOOD_BAR_FRAME_POS, FRAME_TYPE::FOOD_FRAME);

	// Reset spaceship home system
	int sh_health_storage = j["spaceshipHome"]["health_storage"];
	int sh_food_storage = j["spaceshipHome"]["food_storage"];
	int sh_ammo_storage = j["spaceshipHome"]["ammo_storage"];
	spaceship_home_system->resetSpaceshipHomeSystem(sh_health_storage, sh_food_storage, sh_ammo_storage);
	if (p_is_home) spaceship_home_system->enterSpaceship();

	// Load all weapons data
	for (auto weapon : j["weapons"]) {
		weapons_system->setWeaponAttributes(
			static_cast<ITEM_TYPE>(weapon["weapon_type"]),
			static_cast<bool>(weapon["can_fire"]),
			static_cast<int>(weapon["ammo_count"]),
			static_cast<int>(weapon["level"])
		);
	}

	// Set the active weapon
	ITEM_TYPE weapon_type = (ITEM_TYPE) j["active_weapon"];
	weapons_system->setActiveWeapon(weapon_type);
	if (weapon_type == ITEM_TYPE::WEAPON_NONE) {
		user_has_first_weapon = false;
	}


	// Reset powerup system. 

	// restore speed before power up
	float o_speed = j["old_speed"];
	powerup_system->old_speed = o_speed;

	// Reset all the durations. 
	powerup_system->resetPowerupSystem(player_salmon);

	// restore player current speed
	player.current_speed = j["current_speed"];


	// Load all powerup durations
	for (auto& powerup : j["powerups"]) {
		powerup_system->setPowerup(static_cast<float> (powerup["duration_ms"]), static_cast<POWERUP_TYPE>(powerup["powerup_type"]));
	}
	

	// Quest items
	quest_system->resetQuestSystem(j["quest_item_statuses"]);

	// Reset tutorial system
	tutorial_system->resetTutorialSystem();

	// clear all used spawn locations
	used_spawn_locations.clear();

	load_spawned_items_mobs(j);

	// for movement velocity
	for (int i = 0; i < KEYS; i++)
		keyDown[i] = false;

	muzzleFlash = createMuzzleFlash(renderer, vec2(0.f));

	weapons_system->createNonselectedWeaponIndicators();

	tutorial_system->createTutorialText(TUTORIAL_TYPE::GAME_LOADED);

	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("FINISHED LOADING \n");

}

void WorldSystem::load_spawned_items_mobs(json& j) {
	for (auto& item : j["items"]) {
		createItem(renderer, physics_system, { item[1]["position_x"], item[1]["position_y"] }, item[0]["data"]);
	}

	for (auto& mob : j["mobs"]) {
		mob_system->create_mob({ mob[1]["position_x"], mob[1]["position_y"] }, mob[0]["type"], mob[0]["health"]);
	}
}

void WorldSystem::emitMuzzleFlash(vec2 position, int PLAYER_DIRECTION) {
	auto& m = registry.motions.get(muzzleFlash);
	vec2 offset;

	float horizonatalOffset = 1.75f;
	float verticalOffset = 1.1f;

	// caluclate offset position and angle 
	switch (PLAYER_DIRECTION)
	{
		case 1: //Down
			offset = vec2{ 0.f, verticalOffset };
			m.angle = (90.f * M_PI / 180.f);
			break;
		case 2: //UP
			offset = vec2{ 0.f, -verticalOffset };
			m.angle = (270.f * M_PI / 180.f);

			break;
		case 4: //LEFT
			offset = vec2{ -horizonatalOffset, -0.05f };
			m.angle = (180.f * M_PI / 180.f);
			break;
		case 0: //RIGHT
			offset = vec2{ horizonatalOffset, -0.05f };
			m.angle = (0.f * M_PI / 180.f);
			break;
		default:
			offset = vec2{ 0.f, verticalOffset };
			m.angle = (90.f * M_PI / 180.f);
			break;

	}

	// move muzzle flash to target position
	m.position = position + offset;

	// set to start from first frame
	registry.animations.get(muzzleFlash).framex = 0;

	// set alpha
	 registry.colors.get(muzzleFlash).a = 1.0f;



}

void WorldSystem::powerup_spawn_helper(vec2 position){ 

	// set up the random number generator
	std::random_device rng;
	std::mt19937 generator(rng());  // Seed the generator
	std::uniform_int_distribution<> distribution_powerup_type(0, 3);

	switch (distribution_powerup_type(rng)) {
	case 0:
		createItem(renderer, physics_system, position, ITEM_TYPE::POWERUP_SPEED);
		break;
	case 1:
		createItem(renderer, physics_system, position, ITEM_TYPE::POWERUP_HEALTH);
		break;
	case 2:
		createItem(renderer, physics_system, position, ITEM_TYPE::POWERUP_INVISIBLE);
		break;
	case 3:
		createItem(renderer, physics_system, position, ITEM_TYPE::POWERUP_INFINITE_BULLET);
		break;
	}

}
