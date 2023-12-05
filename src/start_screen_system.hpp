#pragma once

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

// A system that displays the start screen(s)
class StartScreenSystem
{
    public:
    StartScreenSystem()
    {
    }

    void init(GLFWwindow* window_arg, RenderSystem* renderer_arg);

    void step();

    bool is_finished();

    private:
    // The window
    GLFWwindow* window;

    // The rendering system
    RenderSystem* renderer;

    // Tracking which screen to show
    std::vector <TEXTURE_ASSET_ID> screen_textures = {
        TEXTURE_ASSET_ID::START_SCREEN_ONE,
    };
    int screen_idx;
    int prev_screen_idx;

    // Did we show all the necessary starting screens?
    bool finished_start_screens;

    // Callback functions for mouse actions
    void on_mouse_move(vec2 mouse_position);
    void on_mouse_click(int button, int action, int mods);

    // Setup the start screen
    void setupScreen(int screen_num);
};
