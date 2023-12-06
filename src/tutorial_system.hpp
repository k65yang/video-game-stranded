#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

// A tutorial system class that handles everything tutorial home related
class TutorialSystem
{
    public:
        /// @brief TutorialSystem constructor
        TutorialSystem()
        {
        }

        /// @brief Function to update tutorial system in world time
        /// @param elapsed_ms How many ms elapsed since last update
        void step(float elapsed_ms);

        /// @brief Initializes the render system that the tutorial system uses
        /// @param renderer_arg Pointer to the render system
        void init(RenderSystem* renderer_arg);

        /// @brief Creates tutorial text
        /// @param type The type of tutorial to create the text for
        Entity createTutorialText(TUTORIAL_TYPE type);

    private:
        const vec2 TUTORIAL_TEXT_POSITION = { 0.f, -7.f };
        const std::string QUEST_ITEM_TUTORIAL_TEXT = "You found a spaceship part! Bring it back to your spaceship to reattach it to the ship";
        const std::string SPACESHIP_HOME_TUTORIAL_TEXT = "Welcome home! Your ammo, food, and health automatically regenerate here, but note that there is a limited amount of these resources in storage";

        RenderSystem* renderer;
};
