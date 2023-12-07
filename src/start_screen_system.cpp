#include "start_screen_system.hpp"

StartScreenSystem::~StartScreenSystem() {
    // Remove everything from the registry; nothing should leak into the main game
    // registry.clear_all_components(); // Don't touch the registry
    clear_used_entities();

    // Reset the terrain system
	terrain->resetTerrainSystem();

    // Re-enable fow
    renderer->enableFow = 1;

    window = nullptr;
    renderer = nullptr;

    screen_textures.clear();
    screen_textures.shrink_to_fit();
}

void StartScreenSystem::init(GLFWwindow* window_arg, RenderSystem* renderer_arg, TerrainSystem* terrain_arg) {
    // Set pointers
    this->window = window_arg;
    this->renderer = renderer_arg;
    this->terrain = terrain_arg;

    // Initialize the terrain system with the map
    terrain->init(loaded_map_name, renderer);

    // We need to display all the screens
    finished_start_screens = false;
    screen_idx = 0;
    prev_screen_idx = -1;

    // Disable fow (enabled by default in the renderer)
    renderer->enableFow = 0;

    // Set a starting camera position and movement direction
    camera_position = {0.f, 0.f};   // camera at origin
    createCamera(camera_position);
    movement_idx = 0;               // camera will move right

    // The mouse is not hovering over anything
    // DO NOT PUT A BUTTON NEAR THE EDGES OF THE SCREEN
    was_hovering = false;

    // Get window size
	glfwGetFramebufferSize(window, &window_w, &window_h);

    // Set callbacks
    glfwSetWindowUserPointer(window, this);
    auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((StartScreenSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto cursor_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((StartScreenSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_button_redirect);
}

void StartScreenSystem::step(float elapsed_ms) {
    // Check if we have gone through all the start screens
    if (screen_idx == screen_textures.size()) {
        // Remove everything from the current screen
        clear_used_entities();

        // Done with start screens
        finished_start_screens = true;

        // Re-enable fow
        renderer->enableFow = 1;

        // Reset the terrain system
	    terrain->resetTerrainSystem();

        return;
    }
        
    // Check if we need to load a new start screen
    if (prev_screen_idx != screen_idx) {

        // Remove everything from the current screen
        clear_used_entities();

        // Set up the correct screen
        setupScreen(screen_idx);

        prev_screen_idx = screen_idx;
    }

    // Update camera position
    Motion& motion = registry.motions.get(registry.get_main_camera());
    motion.position += camera_movement[movement_idx].first;

    // Decrement the camera movement tracker and check if we need to change direction
    camera_movement[movement_idx].second -= elapsed_ms;
    if (camera_movement[movement_idx].second <= 0.f) {
        camera_movement[movement_idx].second = 2500.f;
        ++movement_idx %= camera_movement.size();
    }
}

void StartScreenSystem::setupScreen(int screen_num) {
    auto entity = Entity();

    // Mesh
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
    // The origin of the mesh is the centre. The origin of the screen is top right.
    // We need to shift the position to fill up the screen.
    // We also need to scale the mesh to match the screen size.
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { (float)window_w/2, (float)window_h/2 };
	motion.scale = vec2({ window_w, window_h });

	registry.renderRequests.insert(
		entity,
		{ screen_textures[screen_num],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });
    used_entities.push_back(entity);

    // Call helpers for additional objects on the specific screen
    switch (screen_num)
    {
    case 0:
        setupScreenOneObjects();
        break;
    
    default:
        break;
    }
}

void StartScreenSystem::setupScreenOneObjects() {
    // Compute location of the start button because we support multiple resolutions
    // The button width should be 1/3 of screen width and 1/3 of screen height
    // The location should be centred horizontally and a third of the way from the top of screen
    float button_width = window_w/3;
    float button_height = window_h/3; // ??? something is wrong here, height doesn't look like 1/3 of window
    float button_x = window_w/2;
    float button_y = window_h/3;

    // Set up the start button for rendering
    auto entity_one = Entity();
	Mesh& mesh_one = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity_one, &mesh_one);

    auto& motion_one = registry.motions.emplace(entity_one);
	motion_one.angle = 0.f;
	motion_one.velocity = { 0.f, 0.f };
	motion_one.position = { button_x, button_y };
	motion_one.scale = vec2({ button_width, button_height });

    registry.renderRequests.insert(
		entity_one,
		{ TEXTURE_ASSET_ID::START_BUTTON,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_2 });
    
    screen_one_hover_swaps["start_button"].first = entity_one;
    used_entities.push_back(entity_one);

    // Set up the "hovered" start button for rendering
    // Put in layer 3 (we do not render anything in layer 3)
    auto entity_two = Entity();
	Mesh& mesh_two = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity_two, &mesh_two);

    auto& motion_two = registry.motions.emplace(entity_two);
	motion_two.angle = 0.f;
	motion_two.velocity = { 0.f, 0.f };
	motion_two.position = { button_x, button_y };
	motion_two.scale = vec2({ button_width, button_height });

    registry.renderRequests.insert(
		entity_two,
		{ TEXTURE_ASSET_ID::START_BUTTON_HOVER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_3 });
    
    screen_one_hover_swaps["start_button"].second = entity_two;
    used_entities.push_back(entity_two);

    // Push location of top left and bottom right corners
    // Add 20px buffer into the shape
    screen_one_buttons["start_button"].push_back({button_x - button_width/2 + 20, button_y + button_height/2 - 20});
    screen_one_buttons["start_button"].push_back({button_x + button_width/2 - 20, button_y - button_height/2 + 20});
}

void StartScreenSystem::on_mouse_move(vec2 mouse_position) {
    if (screen_idx == 0) { // On first screen
        // There is a race condition where we check if we are hovering before
        // we actually know the location of the button
        if(screen_one_buttons["start_button"].size() < 1)
            return;
        
        vec2 button_top_left = screen_one_buttons["start_button"][0];
        vec2 button_bottom_right = screen_one_buttons["start_button"][1];
        bool is_currently_hovering = is_hovering(mouse_position, button_top_left, button_bottom_right);

        // Scuffed way to change states of a button
        if ((!was_hovering && is_currently_hovering) || (was_hovering && !is_currently_hovering)) {
            // Swap the layers of the start buttons
            Entity e_one = screen_one_hover_swaps["start_button"].first;
            Entity e_two = screen_one_hover_swaps["start_button"].second;

            RenderRequest& one_rr = registry.renderRequests.get(e_one);
            one_rr.layer_id = one_rr.layer_id == RENDER_LAYER_ID::LAYER_2 ? RENDER_LAYER_ID::LAYER_3 : RENDER_LAYER_ID::LAYER_2;
            RenderRequest& two_rr = registry.renderRequests.get(e_two);
            two_rr.layer_id = two_rr.layer_id == RENDER_LAYER_ID::LAYER_2 ? RENDER_LAYER_ID::LAYER_3 : RENDER_LAYER_ID::LAYER_2;

            was_hovering = is_currently_hovering;
        }
    }
}

bool StartScreenSystem::is_hovering(vec2 position, vec2 top_left, vec2 bottom_right) {
    return (
        position.x >= top_left.x && 
        position.x <= bottom_right.x &&
        position.y <= top_left.y && 
        position.y >= bottom_right.y);
}

void StartScreenSystem::on_mouse_click(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (screen_idx == 0 && was_hovering) {
            screen_idx++;
        }
	}
}

bool StartScreenSystem::is_finished() {
    return finished_start_screens || bool(glfwWindowShouldClose(window));
}

void StartScreenSystem::clear_used_entities() {
    for (Entity e : used_entities)
        registry.remove_all_components_of(e);
}
