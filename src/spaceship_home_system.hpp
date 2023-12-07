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
        /// @param health_storage Initial amount of health the spaceship home stores
        /// @param food_storage Initial amount of food the spaceship home stores
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        void resetSpaceshipHomeSystem(int health_storage, int food_storage, int ammo_storage);

        /// @brief Executes various actions when player enters the spaceship
        /// @param player_health_bar The entity for the player's health bar
        /// @param player_food_bar The entity for the player's food bar
        void enterSpaceship(Entity player_health_bar, Entity player_food_bar);

        /// @brief Executes various actions when player exits the spaceship
        void exitSpaceship();

        /// @brief Checks if player is in the spaceship home
        /// @return Returns true if the player is in the spaceship home, false otherwise
        bool isHome();

        /// @brief Checks if the mouse is hovering over a storage item
        /// @param mouse_pos The position of the mouse in clip coordinates
        /// @param type The storage item to check
        /// @return Returns true if the mouse is hovering over the storage item, false otherwise
        bool isMouseOverStorageItem(vec2 mouse_pos, ITEM_TYPE type);

        /// @brief Regenerates a stat of the player (ex. health, food, ammo) using a resource stored in the spaceship 
        /// @param type The type of the resource to use to regenerate the stat
        void regenerateStat(RESOURCE_TYPE type);

        /// @brief Updates the text count for a resource in storage
        /// @param type The type of the resource to update the text count for
        /// @return The updated text count
        Entity updateStorageCountText(RESOURCE_TYPE type);

    private:
        const vec2 SPACESHIP_HOME_POSITION = { 0.f, 0.f };
        const vec2 FOOD_ITEM_POSITION = { -6.2f, -0.5f };
        const vec2 FOOD_STORAGE_COUNT_POSITION = { -4.7f, -0.3f };
        const vec2 AMMO_ITEM_POSITION = { 1.8f, -0.5f };
        const vec2 AMMO_STORAGE_COUNT_POSITION = { 4.0f, -0.4f };
        const vec2 HEALTH_ITEM_POSITION = { 2.2f, -3.5f };
        const vec2 HEALTH_STORAGE_COUNT_POSITION = { 3.6f, -3.2f };
        const vec3 STORAGE_FULL_TEXT_COLOR = { 0.f, 0.f, 0.f };
        const vec3 STORAGE_EMPTY_TEXT_COLOR = { 1.f, 0.f, 0.f };
        const float STORAGE_COUNT_TEXT_SCALE = 0.5;

        RenderSystem* renderer;
        WeaponsSystem* weaponsSystem;
        QuestSystem* quest_system;
        Entity spaceship_home;
        Entity health_item;
        Entity health_storage_count;
        Entity food_item;
        Entity food_storage_count;
        Entity ammo_item;
        Entity ammo_storage_count;

        /// @brief Creates the spaceship home
        /// @param position Position of the spaceship home
        /// @param health_storage Initial amount of health the spaceship home stores 
        /// @param food_storage Initial amount of food the spaceship home stores 
        /// @param ammo_storage Initial amount of ammo the spaceship home stores
        /// @return The created entity
        Entity createSpaceshipHome(vec2 position, int health_storage, int food_storage, int ammo_storage);

        /// @brief Creates a spaceship home item
        /// @param position Position of the item
        /// @param texture The texture for the item to create
        /// @return The created entity
        Entity createSpaceshipHomeItem(vec2 position, TEXTURE_ASSET_ID texture);

        /// @brief Updates a stat of the player (ex. health, food, ammo) using resources stored in the spaceship
        /// @param stat The current value of the stat
        /// @param storage The current value of the resoure in the spaceship
        /// @param max_stat_value The max value the player can have for the stat
        void updateStat(int& stat, int& storage, int max_stat_value);

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

        /// @brief Creates a string that reflects the remaining amount of a resource in a storage
        /// @param storage The remaining amount of the resource in the storage
        /// @return A string that reflects the remaining amount of a resource in a storage
        std::string createStorageCountTextString(int storage);
};
