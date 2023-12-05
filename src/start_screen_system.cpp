#include "start_screen_system.hpp"

void StartScreenSystem::init(GLFWwindow* window_arg, RenderSystem* renderer_arg) {
    // Set pointers
    this->window = window_arg;
    this->renderer = renderer_arg;

    // We need to display all the screens
    finished_start_screens = false;
    screen_idx = 0;
    prev_screen_idx = -1;

    // Set callbacks
    glfwSetWindowUserPointer(window, this);
    auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((StartScreenSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto cursor_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((StartScreenSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, cursor_button_redirect);

    printf("start screen initalized\n");
}

void StartScreenSystem::step() {
    // Check if we have gone through all the start screens
    if (screen_idx == screen_textures.size()) {
        // Remove everything from the current screen
        registry.clear_all_components();

        finished_start_screens = true;
        return;
    }
        
    // Check if we need to load a new start screen
    if (prev_screen_idx != screen_idx) {
        // Remove everything from the current screen
        registry.clear_all_components();

        // Set up the correct screen
        setupScreen(screen_idx);

        prev_screen_idx = screen_idx;
    }
}

void StartScreenSystem::setupScreen(int screen_num) {
    auto entity = Entity();

    // Mesh
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

    // Get the size of the window
    int w, h;
	glfwGetFramebufferSize(window, &w, &h);
    printf("w: %i\n", w);
    printf("h: %i\n", h);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { 600.f, 400.f };
	motion.scale = vec2({ w, h });

	registry.renderRequests.insert(
		entity,
		{ screen_textures[screen_num],
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_1 });
}

void StartScreenSystem::on_mouse_move(vec2 mouse_position) {
    // nothing for now
}

void StartScreenSystem::on_mouse_click(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // next screen
        screen_idx++;
	}
}

bool StartScreenSystem::is_finished() {
    return finished_start_screens || bool(glfwWindowShouldClose(window));
}