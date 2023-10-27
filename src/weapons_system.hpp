#pragma once

#include <map>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

//  A weapons system class that handles everything weapons related
class WeaponsSystem
{
    public:
        /// @brief WeaponsSystem constructor
        WeaponsSystem()
        {
        }

        /// @brief  Initializes the render system that the weapon system needs to draw projectiles
        /// @param renderer_arg Pointer to the render system
        void init(RenderSystem* renderer_arg) {
            this->renderer = renderer_arg;
        }

        /// @brief Function to update weapon systems in world time.
        ///        Currently controls the fire rate of the equipped weapon only.
        /// @param elapsed_ms how much ms elapsed since last update
        void step(float elapsed_ms);
        
        /// @brief Creates a weapon and adds it to the weapons registry.
        /// NOTE: Does not check if the weapon exists already. 
        ///       (If we do not limit weapon spawns we can create/equip the same weapon multiple times)
        /// @param weapon_type The type of weapon to be created
        /// @return The created entity
        Entity createWeapon(ITEM_TYPE weapon_type);

        /// @brief Create the weapon projectile and sets its motion
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the player
        void fireWeapon(float player_x, float player_y, float angle);

        /// @brief Applies weapon weapon effects if any (slow, poison, knockback)
        /// @param mob Entity to apply the effects to
        void applyWeaponEffects(Entity projectile, Entity mob);

        /// @brief Increases the current weapon level by 1
        void upgradeCurrentWeapon(){
            weapon_level[active_weapon_type]++;
        };

        // TODO
        /// @brief When the game restarts (without exiting), we must clean the weapons system
        // void cleanWeaponsSystem();

    private:
        // Keeps track of the weapon
        ITEM_TYPE active_weapon_type;
        Entity active_weapon_entity;
        Weapon* weapon_component = nullptr; // starts off by pointing to nothing

        // Pointer to rendering system for projectiles
        RenderSystem* renderer;

        // Hardcoded weapons data
        // Fire rates for each weapon
        std::map<ITEM_TYPE, float> weapon_fire_rate_map{
            {ITEM_TYPE::WEAPON_NONE, 0.f},
            {ITEM_TYPE::WEAPON_SHURIKEN, 250.f},
            {ITEM_TYPE::WEAPON_CROSSBOW, 750.f},
            {ITEM_TYPE::WEAPON_SHOTGUN, 500.f},
            {ITEM_TYPE::WEAPON_MACHINEGUN, 100.f},
        };

        // Projectile velocities for each weapon
        std::map<ITEM_TYPE, float> weapon_projectile_velocity_map{
            {ITEM_TYPE::WEAPON_NONE, 0.f},
            {ITEM_TYPE::WEAPON_SHURIKEN, 10.f},
            {ITEM_TYPE::WEAPON_CROSSBOW, 10.f},
            {ITEM_TYPE::WEAPON_SHOTGUN, 15.f},
            {ITEM_TYPE::WEAPON_MACHINEGUN, 15.f},
        };

        // Weapon damages
        std::map<ITEM_TYPE, int> weapon_damage_map{
            {ITEM_TYPE::WEAPON_NONE, 0},
            {ITEM_TYPE::WEAPON_SHURIKEN, 10},
            {ITEM_TYPE::WEAPON_CROSSBOW, 15},
            {ITEM_TYPE::WEAPON_SHOTGUN, 7},
            {ITEM_TYPE::WEAPON_MACHINEGUN, 5},
        };

        // Weapon projectile textures
        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> projectile_textures_map{
            {ITEM_TYPE::WEAPON_SHURIKEN, TEXTURE_ASSET_ID::WEAPON_SHURIKEN},
            {ITEM_TYPE::WEAPON_CROSSBOW, TEXTURE_ASSET_ID::WEAPON_ARROW},
        };

        // Current weapon upgrade levels
        std::map<ITEM_TYPE, int> weapon_level {
            {ITEM_TYPE::WEAPON_NONE, 0},
            {ITEM_TYPE::WEAPON_SHURIKEN, 0},
            {ITEM_TYPE::WEAPON_CROSSBOW, 0},
            {ITEM_TYPE::WEAPON_SHOTGUN, 0},
            {ITEM_TYPE::WEAPON_MACHINEGUN, 0},
        };

        /// @brief Fires the shuriken based on upgrade level
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the projectile
        void fireShuriken(float player_x, float player_y, float player_angle);

        /// @brief Fires the crossbow based on upgrade level
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the projectile
        void fireCrossbow(float player_x, float player_y, float player_angle);

        /// @brief Fires the shotgun based on upgrade level
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the projectile
        void fireShotgun(float player_x, float player_y, float player_angle);

        /// @brief  Fires the machine gun based on upgrade level
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the projectile
        void fireMachineGun(float player_x, float player_y, float angle);

        /// @brief Applies a slow to the mob
        /// @param mob Entity to apply the slow to
        /// @param duration_ms How long in ms the slow lasts
        /// @param slow_ratio How much to slow it by [0.0, 1.0] (1.0 is no slow)
        void applySlow(Entity mob, float duration_ms, float slow_ratio);
        
        /// @brief Creates a projectile
        /// @param renderer Pointer to the render system
        /// @param pos Position to draw the projectile
        /// @param angle Angle of the drawn projectile
        /// @return The projectile entity
        Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle);

        /// @brief Checks if the given ITEM_TYPE is a weapon
        /// @param test the ITEM_TYPE to be tested
        /// @return bool
        bool isValidWeapon(ITEM_TYPE test);
};