#pragma once

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

// A system that displays the start screen(s)
class StartScreenSystem
{
    public:
    StartScreenSystem()
    {
    }

    ~StartScreenSystem();

    void init(GLFWwindow* window_arg, RenderSystem* renderer_arg, TerrainSystem* terrain_arg);

    void step(float elapsed_ms);

    bool is_finished();

    private:
    // The window
    GLFWwindow* window;
    int window_w;
    int window_h;

    // The rendering system
    RenderSystem* renderer;

    // The terrain system
    TerrainSystem* terrain;

    // Tracking which screen to show
    std::vector <TEXTURE_ASSET_ID> screen_textures = {
        TEXTURE_ASSET_ID::START_SCREEN_ONE,
        TEXTURE_ASSET_ID::START_SCREEN_TWO,
    };
    int screen_idx;
    int prev_screen_idx;

    // Tracking used entities for the current screen
    std::vector<Entity> used_entities;
    std::vector<Entity> moving_entities;

    // Track locations of buttons in each screen
    std::map<std::string, std::vector<vec2>> screen_one_buttons = {
        {"start_button", {}}
    };

    // Track hover swaps
    std::map<std::string, std::pair<Entity, Entity>> screen_one_hover_swaps = {
        {"start_button", {}}
    };
    bool was_hovering;

    // Track camera motion and position
    std::vector<std::pair<vec2, float>> camera_movement = {
        {{ 0.01f ,  0.f },   5000.f}, // moves left for 5s, etc...
        {{ 0.f   ,  0.01f }, 5000.f},
        {{-0.01f ,  0.f },   10000.f},
        {{ 0.f   , -0.01f }, 5000.f},
        {{ 0.01f ,  0.f },   5000.f},
    };
    int movement_idx;
    vec2 camera_position;

    // Did we show all the necessary starting screens?
    bool finished_start_screens;

    // Callback functions for mouse actions
    void on_mouse_move(vec2 mouse_position);
    void on_mouse_click(int button, int action, int mods);

    // Setup the background image of the start screen
    // Does not set up any additional images on the screen (these will be done in helpers)
    void setupScreen(int screen_num);

    // Setup additional screen objects for each individual screen
    void setupScreenOneObjects();
    void setupScreenTwoObjects();

    // Helper to check if cursor is hovering over the button
    bool is_hovering(vec2 position, vec2 top_left, vec2 bottom_right);

    // Helper to clear used entities for the current screen
    void clear_used_entities();
};
