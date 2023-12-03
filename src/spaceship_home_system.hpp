#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"
#include "weapons_system.hpp"
#include "quest_system.hpp"

// A spaceship system class that handles everything spaceship home related
class SpaceshipHomeSystem
{
    public:
        /// @brief SpaceshipHomeSystem constructor
        SpaceshipHomeSystem()
        {
        }

        /// @brief Function to update spaceship home system in world time
        /// @param elapsed_ms How many ms elapsed since last update
        void step(float elapsed_ms);

        /// @brief Initializes the render system that the spaceship home system uses
        /// @param renderer_arg Pointer to the render system
        /// @param weapon_system_arg Pointer to the weapons system
        void init(RenderSystem* renderer_arg, WeaponsSystem* weapon_system_arg, QuestSystem* quest_system_arg);

        /// @brief Initializes spaceship home elements 
        /// @param food_storage Initial amount of food the spaceship home stores
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        void resetSpaceshipHomeSystem(int food_storage, int ammo_storage);

        /// @brief Executes various actions when player enters spaceship
        /// @param player_health_bar The entity for the player's health bar
        /// @param player_food_bar The entity for the player's food bar
        void enterSpaceship(Entity player_health_bar, Entity player_food_bar);

        /// @brief Checks if player is in the spaceship home
        /// @return Returns true if the player is in the spaceship home, false otherwise
        bool isHome();

    private:
        const vec2 FOOD_ITEM_POSITION = { -5.6f, 0.f };
        const vec2 FOOD_STORAGE_BAR_POSITION = { -3.5f, 0.f };
        const vec2 FOOD_STORAGE_BAR_FRAME_POSITION = { -3.49f, 0.f };
        const vec2 AMMO_ITEM_POSITION = { 1.f, 0.5f };
        const vec2 AMMO_STORAGE_BAR_POSITION = { 4.5f, 0.5f };
        const vec2 AMMO_STORAGE_BAR_FRAME_POSITION = { 4.51f, 0.5f };

        RenderSystem* renderer;
        WeaponsSystem* weaponsSystem;
        QuestSystem* quest_system;
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
};
