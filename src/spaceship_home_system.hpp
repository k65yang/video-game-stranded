#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

// A spaceship system class that handles everything spaceship home related
class SpaceshipHomeSystem
{
    public:
        const int SPACESHIP_MAX_FOOD_STORAGE = 500;
        const int SPACESHIP_MAX_AMMO_STORAGE = 100;

        /// @brief SpaceshipHomeSystem constructor
        SpaceshipHomeSystem()
        {
        }

        /// @brief Function to update spaceship home system in world time
        /// @param elapsed_ms How many ms elapsed since last update
        void step(float elapsed_ms);

        /// @brief Initializes the render system that the spaceship home system uses
        /// @param renderer_arg Pointer to the render system
        void init(RenderSystem* renderer_arg);

        /// @brief Initializes spaceship home elements 
        /// @param camera The entity for the camera
        /// @param is_home Whether the player is in the spaceship home or not 
        /// @param food_storage Initial amount of food the spaceship home stores
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        void resetSpaceshipHomeSystem(Entity camera, bool is_home, int food_storage, int ammo_storage);

        /// @brief Executes various actions when player enters spaceship
        /// @param player_health_bar The entity for the player's health bar
        /// @param player_food_bar The entity for the player's food bar
        /// @param player_ammo_bar The entity for the player's ammo bar
        /// @param player_weapon The entity for the player's weapon
        void enterSpaceship(Entity player_health_bar, Entity player_food_bar, Entity player_ammo_bar, Entity player_weapon);

        /// @brief Checks if player is in the spaceship home
        /// @return Returns true if the player is in the spaceship home, false otherwise
        bool isHome();

    private:
        const vec2 FOOD_STORAGE_BAR_SCALE = { 0.5f, 2.f };
        const vec2 AMMO_STORAGE_BAR_SCALE = { 0.5f, 2.f };   

        RenderSystem* renderer;
        Entity spaceship_home;
        Entity food_item;
        Entity food_storage_bar;
        Entity food_storage_bar_frame;
        Entity ammo_item;
        Entity ammo_storage_bar;
        Entity ammo_storage_bar_frame;

        /// @brief Creates the spaceship home
        /// @param position Position of the spaceship home
        /// @param is_inside Whether the player is in the spaceship home or not
        /// @param food_storage Initial amount of food the spaceship home stores 
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        /// @return The created entity
        Entity createSpaceshipHome(vec2 position, bool is_inside, int food_storage, int ammo_storage);

        /// @brief Creates a spaceship home item
        /// @param position Position of the item
        /// @param texture The texture for the item to create
        /// @return The created entity
        Entity createSpaceshipHomeItem(vec2 position, TEXTURE_ASSET_ID texture);

        /// @brief Updates the positions of the various UI elements of the spaceship home
        void updateSpaceshipHomeUI();

        /// @brief Regenerates a stat of the player (ex. health, food, ammo) using resources stored in the spaceship
        /// @param stat The current value of the stat
        /// @param storage The current value of the resoure in the spaceship
        /// @param max_stat_value The max value the player can have for the stat
        void regenerateStat(int& stat, int& storage, int max_stat_value);

        /// @brief Updates the scale of a UI bar
        /// @param new_val The new value for the bar
        /// @param bar The motion component of the bar
        /// @param max_bar_value The max value the bar can be
        /// @param scale_factor The scale factor for the bar
        /// @param is_stat If true, the bar that is being updated is a player stat bar. Otherwise, the bar that is being updated is a spaceship resource storage bar
        void updateBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor, bool is_stat=true);
};
