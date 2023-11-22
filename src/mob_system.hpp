#pragma once

#include <map>
#include <random>

#include "common.hpp"
#include "components.hpp"
#include "render_system.hpp"
#include "terrain_system.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

// A mob system class that handles everything mob related
class MobSystem
{
    public:
        /// @brief MobSystem constructor
        MobSystem()
        {
        }

        /// @brief Initializes the render and terrain systems that the mob system uses
        /// @param renderer_arg Pointer to the render system
        /// @param terrain_arg Pointer to the terrain system
        void init(RenderSystem* renderer_arg, TerrainSystem* terrain_arg) {
            this->renderer = renderer_arg;
            this->terrain = terrain_arg;
            this->rng = std::default_random_engine(std::random_device()());
        }

        /// @brief Function to update mob system in world time.
        /// @param elapsed_ms how much ms elapsed since last update
        void step(float elapsed_ms);

        /// @brief Spawns mobs at various locations around the map
        void spawn_mobs();

        /// @brief Applies mob attack effects to the player if any (slow, poison, knockback)
        /// @param player The player
        /// @param mob The mob whose effects are to be applied to the player
        void apply_mob_attack_effects(Entity player, Entity mob);
        
        /// @brief Creates a mob and adds it to the mobs registry
        /// @param mob_position The initial position of the mob
        /// @param mob_type The type of mob to be created
        /// @return The created entity
        Entity create_mob(vec2 mob_position, MOB_TYPE mob_type, int current_health = 0);

    private:
        // Pointer to rendering system
        RenderSystem* renderer;

        // Pointer to terrain system
        TerrainSystem* terrain;

        // C++ random number generator
	    std::default_random_engine rng;

        // Harcoded mob data
        // Damage for each mob
        const std::map<MOB_TYPE, int> mob_damage_map = {
            {MOB_TYPE::GHOST, 10},
            {MOB_TYPE::SLIME, 15},
            {MOB_TYPE::BRUTE, 50},
            {MOB_TYPE::DISRUPTOR, 5},
            {MOB_TYPE::TURRET, 5}
        };

        // Aggro range for each mob
        const std::map<MOB_TYPE, int> mob_aggro_range_map = {
            {MOB_TYPE::GHOST, 10},
            {MOB_TYPE::SLIME, 10},
            {MOB_TYPE::BRUTE, 10},
            {MOB_TYPE::DISRUPTOR, 10},
            {MOB_TYPE::TURRET, 5}
        };

        // Health for each mob
        const std::map<MOB_TYPE, int> mob_health_map = {
            {MOB_TYPE::GHOST, 50},
            {MOB_TYPE::SLIME, 30},
            {MOB_TYPE::BRUTE, 100},
            {MOB_TYPE::DISRUPTOR, 50},
            {MOB_TYPE::TURRET, 10}
        };

        // Speed for each mob
        const std::map<MOB_TYPE, float> mob_speed_ratio_map = {
            {MOB_TYPE::GHOST, 2.5f},
            {MOB_TYPE::SLIME, 2.f},
            {MOB_TYPE::BRUTE, 0.5f},
            {MOB_TYPE::DISRUPTOR, 1.5f},
            {MOB_TYPE::TURRET, 0.f}
        };

        // Texture for each mob
        const std::map<MOB_TYPE, TEXTURE_ASSET_ID> mob_textures_map = {
            {MOB_TYPE::GHOST, TEXTURE_ASSET_ID::GHOST},
            {MOB_TYPE::SLIME, TEXTURE_ASSET_ID::SLIME},
            {MOB_TYPE::BRUTE, TEXTURE_ASSET_ID::BRUTE},
            {MOB_TYPE::DISRUPTOR, TEXTURE_ASSET_ID::DISRUPTOR},
            {MOB_TYPE::TURRET, TEXTURE_ASSET_ID::TURRET}
        };

        /// @brief Applies knockback effect to the player
        /// @param player The player
        /// @param mob The mob that is knocking-back the player
        /// @param duration_ms How long (in ms) the knockback lasts
        /// @param knockback_speed_ratio How fast the player travels while being knocked-back
        void apply_knockback(Entity player, Entity mob, float duration_ms, float knockback_speed_ratio);

        /// @brief Applies inaccuracy effect to the player
        /// @param player The player
        /// @param duration_ms How long (in ms) the inaccuracy lasts
        /// @param inaccuracy_percent How much inaccuracy there is for the player as a percentage [0, 1.0] (0 means no inaccuracy)
        void apply_inaccuracy(Entity player, float duration_ms, float inaccuracy_percent);
};
