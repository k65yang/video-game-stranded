#pragma once

#include <map>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "physics_system.hpp"

//  A weapons system class that handles everything weapons related
class WeaponsSystem
{
    public:
        /// @brief WeaponsSystem constructor
        WeaponsSystem()
        {
        }

        /// @brief When the game restarts (without exiting), 
        ///        we must clean the weapons system
        void resetWeaponsSystem();

        /// @brief  Initializes the render system
        /// @param renderer_arg Pointer to the render system
        void init(RenderSystem* renderer_arg, PhysicsSystem* physics_arg);

        /// @brief Function to update weapon systems in world time.
        ///        Currently controls the fire rate of the equipped weapon only.
        /// @param elapsed_ms how much ms elapsed since last update
        void step(float elapsed_ms);

        /// @brief Function to set the dynamic properties of a weapon 
        /// @param weapon_type The weapon type
        /// @param weapon_can_fire If the weapon can fire or not
        /// @param weapon_ammo_count The ammo count of the weapon
        /// @param weapon_level The level of the weapon
        void setWeaponAttributes(ITEM_TYPE weapon_type, bool weapon_can_fire, int weapon_ammo_count, int weapon_level);

        /// @brief Increases the ammo for the specified weapon
        /// @param weapon_type The weapon type
        /// @param amount The amount of ammo to increase
        /// @return The amount of ammo actually increased
        int increaseAmmo(ITEM_TYPE weapon_type, int amount);
        
        /// @brief Creates a weapon with no ammo and adds it to the weapons registry.
        ///        Updates the weapon component pointers as well
        /// NOTE: Does not check if the weapon exists already. 
        ///       (If we do not limit weapon spawns we can create/equip the same weapon multiple times)
        /// @param weapon_type The type of weapon to be created
        /// @return The created entity
        Entity createWeapon(ITEM_TYPE weapon_type);

        /// @brief Create the weapon projectile and sets its motion
        /// @param player_x The x coordinate of the player
        /// @param player_y The y coordinate of the player
        /// @param angle The angle of the player
        /// @return The type of weapon that was just fired. WEAPON_NONE if nothing was fired.
        ITEM_TYPE fireWeapon(float player_x, float player_y, float angle);

        /// @brief Applies weapon weapon effects if any (slow, poison, knockback)
        /// @param mob Entity to apply the effects to
        void applyWeaponEffects(Entity projectile, Entity mob);

        /// @brief Increases the active weapon level by 1
        void upgradeWeapon();

        /// @brief Increases the specified weapon level by 1
        /// @param weapon_type The weapon type to upgrade
        void upgradeWeapon(ITEM_TYPE weapon_type);

        /// @brief Gets the current active weapon
        /// @return the active weapon type 
        ITEM_TYPE getActiveWeapon();

        /// @brief Sets the specified weapon to the active weapon type
        void setActiveWeapon(ITEM_TYPE weapon_type);

        /// @brief Gets the ammo count for the active weapon
        /// @return The ammo count (0 if not weapon equipped)
        int getActiveWeaponAmmoCount();

    private:
        // Keeps track of the active weapon
        ITEM_TYPE active_weapon_type;
        Entity active_weapon_entity;                // technically not needed, but track it anyways
        Weapon* active_weapon_component = nullptr;  // starts off by pointing to nothing

        // Tracking the weapon and ammo indicator
        Entity weapon_indicator;
        Entity ammo_indicator;

        // Pointer to rendering system for projectiles
        RenderSystem* renderer;

        // Pointer to physics system for projectiles
        PhysicsSystem* physics;

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


        // Weapon knockbacks
        std::map<ITEM_TYPE, float> weapon_knockback_map{
            {ITEM_TYPE::WEAPON_NONE, 0.f},
            {ITEM_TYPE::WEAPON_SHURIKEN, 0.4f},
            {ITEM_TYPE::WEAPON_CROSSBOW, 0.2f},
            {ITEM_TYPE::WEAPON_SHOTGUN, 0.f},
            {ITEM_TYPE::WEAPON_MACHINEGUN, 0.1f},

        // Weapon ammo capacity
        std::map<ITEM_TYPE, int> weapon_ammo_capacity_map{
            {ITEM_TYPE::WEAPON_NONE, 0},
            {ITEM_TYPE::WEAPON_SHURIKEN, 15},
            {ITEM_TYPE::WEAPON_CROSSBOW, 15},
            {ITEM_TYPE::WEAPON_SHOTGUN, 8},
            {ITEM_TYPE::WEAPON_MACHINEGUN, 30},

        };

        // Weapon projectile textures
        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> projectile_textures_map{
            {ITEM_TYPE::WEAPON_SHURIKEN, TEXTURE_ASSET_ID::WEAPON_SHURIKEN},
            {ITEM_TYPE::WEAPON_CROSSBOW, TEXTURE_ASSET_ID::WEAPON_ARROW},
        };

        // Weapon textures
        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> weapon_indicator_textures_map {
            {ITEM_TYPE::WEAPON_SHURIKEN, TEXTURE_ASSET_ID::ICON_SHURIKEN},
            {ITEM_TYPE::WEAPON_CROSSBOW, TEXTURE_ASSET_ID::ICON_CROSSBOW},
            {ITEM_TYPE::WEAPON_SHOTGUN, TEXTURE_ASSET_ID::ICON_SHOTGUN},
            {ITEM_TYPE::WEAPON_MACHINEGUN, TEXTURE_ASSET_ID::ICON_MACHINE_GUN},
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

        /// @brief Applies a knoback to the mob
        /// @param mob Entity to apply the knockback to
        /// @param knockback_force How much position to push it 
        void applyKnockback(Entity proj, Entity mob, float knockback_force);

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
        Entity createProjectile(RenderSystem* renderer, PhysicsSystem* physics, vec2 pos, float angle);

        /// @brief Creates the weapon indicator
        /// @param renderer Pointer to the render system
        /// @param position The position of the weapon indicator (unused)
        /// @param weapon_texture The texture of the weapon
        /// @return The entity of the weapon indicator
        Entity createWeaponIndicator(RenderSystem* renderer, TEXTURE_ASSET_ID weapon_texture);

        /// @brief Create the ammo bar
        /// @param renderer The rendering system
        /// @return The entity of the ammo bar
        Entity createAmmoBar(RenderSystem* renderer);

        /// @brief Checks if the given ITEM_TYPE is a weapon
        /// @param test the ITEM_TYPE to be tested
        /// @return bool
        bool isValidWeapon(ITEM_TYPE test);

        /// @brief Changes the angle of a projectile so it is not completely accurate
        /// @param angle Original angle of the projectile
        /// @param inaccuracy_percent How much inaccuracy there is for the projectile as a percentage [0, 1.0] (0 means no inaccuracy)
        void applyProjectileInaccuracy(float& angle, float inaccuracy_percent);

        /// @brief Helper to create all weapons
        void createAllWeapons();

        /// @brief Helper to update the scale of the ammo bar
        void updateAmmoBar();
};