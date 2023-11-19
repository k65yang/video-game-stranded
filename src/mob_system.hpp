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
            {MOB_TYPE::DISRUPTOR, 5}
        };

        // Aggro range for each mob
        const std::map<MOB_TYPE, int> mob_aggro_range_map = {
            {MOB_TYPE::GHOST, 10},
            {MOB_TYPE::SLIME, 10},
            {MOB_TYPE::BRUTE, 10},
            {MOB_TYPE::DISRUPTOR, 10}
        };

        // Health for each mob
        const std::map<MOB_TYPE, int> mob_health_map = {
            {MOB_TYPE::GHOST, 50},
            {MOB_TYPE::SLIME, 30},
            {MOB_TYPE::BRUTE, 100},
            {MOB_TYPE::DISRUPTOR, 50}
        };

        // Speed for each mob
        const std::map<MOB_TYPE, float> mob_speed_ratio_map = {
            {MOB_TYPE::GHOST, 2.5f},
            {MOB_TYPE::SLIME, 2.f},
            {MOB_TYPE::BRUTE, 1.f},
            {MOB_TYPE::DISRUPTOR, 1.5f}
        };

        // Texture for each mob
        const std::map<MOB_TYPE, TEXTURE_ASSET_ID> mob_textures_map = {
            {MOB_TYPE::GHOST, TEXTURE_ASSET_ID::GHOST},
            {MOB_TYPE::SLIME, TEXTURE_ASSET_ID::SLIME},
            {MOB_TYPE::BRUTE, TEXTURE_ASSET_ID::BRUTE},
            {MOB_TYPE::DISRUPTOR, TEXTURE_ASSET_ID::DISRUPTOR}
        };

        /// @brief Creates a mob and adds it to the mobs registry
        /// @param mob_position The initial position of the mob
        /// @param mob_type The type of mob to be created
        /// @return The created entity
        Entity create_mob(vec2 mob_position, MOB_TYPE mob_type);
};
