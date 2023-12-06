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
        const vec2 QUEST_ITEM_TUTORIAL_TEXT_POSITION = { -10.f, -7.f };
        const vec2 SPACESHIP_HOME_TUTORIAL_TEXT_POSITION = { -10.f, -7.f };
        const vec2 GAME_SAVED_TEXT_POSITION = { -1.5f, -7.f };
        const vec2 GAME_LOADED_TEXT_POSITION = { -1.5f, -7.f };
        const std::string QUEST_ITEM_TUTORIAL_TEXT = "You found a spaceship part! Bring it back to your spaceship to reattach it to the ship!";
        const std::string SPACESHIP_HOME_TUTORIAL_TEXT = "Welcome home! Stock up on ammo, food, and health here before you continue exploring!";
        const std::string GAME_SAVED_TEXT = "Game saved";
        const std::string GAME_LOADED_TEXT = "Game loaded";
        const float TUTORIAL_TEXT_SCALE = 0.5f;

        RenderSystem* renderer;
};
