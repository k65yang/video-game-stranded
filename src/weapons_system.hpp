#pragma once

#include <map>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"

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
        ///       (If we do not limit weapon spawns we can be create/equip the same weapon multiple times)
        /// @param weapon_type The type of weapon to be created
        /// @return The created entity
        Entity createWeapon(ITEM_TYPE weapon_type);

        /// @brief Create the weapon projectile and sets its motion
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the player
        void fireWeapon(float player_x, float player_y, float angle);

    private:
        // Keeps track of the weapon
        ITEM_TYPE active_weapon_type;
        Entity active_weapon_entity;
        Weapon* weapon_component = nullptr; // starts off by pointing to nothing

        // Pointer to rendering system for projectiles
        RenderSystem* renderer;

        // Hardcoded weapons data
        // Fire rates for each weapon
        std::map<ITEM_TYPE, float> weaponFireRateMap{
            {ITEM_TYPE::WEAPON_NONE, 0.f},
            {ITEM_TYPE::WEAPON_GENERIC, 1000.f},
        };

        // Projectile velocities for each weapon
        std::map<ITEM_TYPE, float> weaponProjectileVelocityMap{
            {ITEM_TYPE::WEAPON_NONE, 0.f},
            {ITEM_TYPE::WEAPON_GENERIC, 10.f},
        };

        // Weapon damages
        std::map<ITEM_TYPE, int> weaponDamageMap{
            {ITEM_TYPE::WEAPON_NONE, 0},
            {ITEM_TYPE::WEAPON_GENERIC, 10},
        };

        /// @brief Creates a projectile
        /// @param renderer Pointer to the render system
        /// @param pos Position to draw the projectile
        /// @param angle Angle of the drawn projectile
        /// @return The projectile entity
        Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle);
};