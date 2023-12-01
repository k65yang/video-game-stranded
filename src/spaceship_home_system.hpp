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
        /// @param food_storage Initial amount of food the spaceship home stores
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        void resetSpaceshipHomeSystem(int food_storage, int ammo_storage);

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
        const vec2 FOOD_ITEM_OFFSET = { -5.6f, 0.f };
        const vec2 FOOD_STORAGE_BAR_OFFSET = { -3.5f, 0.f };
        const vec2 FOOD_STORAGE_BAR_FRAME_OFFSET = { -3.49f, 0.f };
        const vec2 AMMO_ITEM_OFFSET = { 1.f, 0.5f };
        const vec2 AMMO_STORAGE_BAR_OFFSET = { 4.5f, 0.5f };
        const vec2 AMMO_STORAGE_BAR_FRAME_OFFSET = { 4.51f, 0.5f };

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
        /// @param food_storage Initial amount of food the spaceship home stores 
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        /// @return The created entity
        Entity createSpaceshipHome(vec2 position, int food_storage, int ammo_storage);

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

        /// @brief Updates the scale of a player stat bar
        /// @param new_val The new value for the bar
        /// @param bar The motion component of the bar
        /// @param max_bar_value The max value the bar can be
        /// @param scale_factor The scale factor for the bar
        void updateStatBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor);

        /// @brief Updates the scale of a spaceship sstorage bar
        /// @param new_val The new value for the bar
        /// @param bar The motion component of the bar
        /// @param max_bar_value The max value the bar can be
        /// @param scale_factor The scale factor for the bar
        void updateStorageBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor);

        /// @brief Gets a new (x, y) position offset from some base position
        /// @param pos The base position
        /// @param offset A vec2 storing the horizontal and vertical offset 
        /// @return The new position 
        vec2 getNewPosition(vec2 pos, vec2 offset);
};
