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

        /// @brief Resets the tutorial system
        void resetTutorialSystem();
        
        /// @brief Opens the help dialog
        void openHelpDialog();

        /// @brief Closes the help dialog
        void closeHelpDialog();

        /// @brief Checks if the help dialog is open
        /// @return Returns true if the help dialog is open, false otherwise
        bool isHelpDialogOpen();
        
        /// @brief Checks if the mouse is hovering the help button
        /// @param mouse_pos The position of the mouse in clip coordinates
        /// @return Returns true if the mouse is hovering over the help button, false otherwise
        bool isMouseOverHelpButton(vec2 mouse_pos);

        /// @brief Checks if the player is near the spaceship
        /// @param player_pos The position of the player
        /// @param spaceship_pos The position of the spaceship
        /// @return Returns true if the player is near the spaceship, false otherwise
        bool isPlayerNearSpaceship(vec2 player_pos, vec2 spaceship_pos);

        /// @brief Creates a tutorial text that disappears after some time
        /// @param type The type of tutorial to create the text for
        Entity createTutorialText(TUTORIAL_TYPE type);

        /// @brief Creates a tutorial dialog that disappears after some time
        /// @param type The type of tutorial to create the dialog for
        Entity createTutorialDialog(TUTORIAL_TYPE type);

    private:
        const vec2 GAME_SAVED_TEXT_POSITION = { -1.5f, -7.f };
        const vec2 GAME_LOADED_TEXT_POSITION = { -1.5f, -7.f };
        const std::string GAME_SAVED_TEXT = "Game saved";
        const std::string GAME_LOADED_TEXT = "Game loaded";
        const std::string ENTER_SPACESHIP_TEXT = "Press E to enter";
        const float TUTORIAL_TEXT_SCALE = 0.5f;

        const vec2 HELP_BUTTON_POSITION = { 11.f, -7.f }; 
        const vec2 HELP_DIALOG_POSITION = { 0.f, 0.f }; 
        const vec2 TUTORIAL_DIALOG_POSITION = { 0.f, -4.f };
        const vec2 HELP_BUTTON_SCALE = { target_resolution.x / tile_size_px * 0.05f, target_resolution.y / tile_size_px * 0.07f}; 
        const vec2 HELP_DIALOG_SCALE = { target_resolution.x / tile_size_px * 0.5f, target_resolution.y / tile_size_px * 0.5f }; 
        const vec2 TUTORIAL_DIALOG_SCALE = { target_resolution.x / tile_size_px * 0.3f, target_resolution.y / tile_size_px * 0.45f };

        RenderSystem* renderer;
        Entity help_dialog;
        Entity help_button;
        Entity enter_spaceship_text;
        bool is_help_dialog_open;
        bool is_enter_spaceship_text_shown;

        /// @brief Creates the help button
        /// @return The help button
        Entity createHelpButton();

        /// @brief Creates the help dialog
        /// @return The help dialog
        Entity createHelpDialog();

        /// @brief Shows the enter spaceship text
        void showEnterSpaceshipText();

        /// @brief Hides the enter spaceship text
        void hideEnterSpaceshipText();

        /// @brief Checks if the enter spaceship text is shown
        /// @return Returns true if the enter spaceship text is shown, false otherwise
        bool isEnterSpaceshipTextShown();

        /// @brief Removes currently displayed tutorial texts/dialogs
        void removeDisplayedTutorials();
};
