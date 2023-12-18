#pragma once

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include <iostream>
#include <random>
#include <cmath>



// A particle system that handles everything particles related
class ParticleSystem
{
    
    

    public:
    /// @brief ParticleSystem constructor
    ParticleSystem() {
        
   
    }

    /// @brief ParticleSystem destructor
    ~ParticleSystem() {};

    

    /// @brief Initializes the particle system
    /// @param renderer_arg The render system
    void init(RenderSystem* renderer_arg) {
        this->renderer = renderer_arg;
    }

 


    /// @brief Updates particle effects
    void step(float elapsed_ms);

    /// @brief emit one particle using particleTemplate, motion from entity, texture defaulted to redblock.
    /// @param particleTemplate template 
    /// @param entity To get motion related parameter for the particle
    /// @param texture texture used to represent the particle 
    
    Entity emit(ParticleTemplate temp);

    void createParticleTrail(Entity targetEntity, TEXTURE_ASSET_ID texture, int numberOfParticles, vec2 scale);
    void createFloatingHeart(Entity targetEntity, TEXTURE_ASSET_ID texture, int numberOfParticles);
    void createFloatingBullet(Entity targetEntity, int numberOfParticles);
    void createParticleSplash(Entity projectile_entity, Entity mob_entity, int numberOfParticles, vec2 splashDirection);
    void createWaterSplash(Entity targetEntity, int numberOfParticles, float speedRatio);


    private:

    // mob color lookup table for particle splash
    const std::map<MOB_TYPE, vec3> mob_color_map = {
        //255 255 255 for ghost and turret
        {MOB_TYPE::GHOST, vec3{1.f, 1.f, 1.f}},
        // 97 136 51 for max is 255
        {MOB_TYPE::SLIME, vec3{0.45f, 0.6f, 0.2f}},
        //78 50 108 FOR BRUTE
       // MOB_TYPE::BRUTE, vec3{.31f, .20f, .42f
        {MOB_TYPE::BRUTE, vec3{.76f, .32f, .68f}},
        // 64 62 74
        {MOB_TYPE::DISRUPTOR, vec3{.05f, .53f, .57f}},
        {MOB_TYPE::TURRET, vec3{1.f, 1.f, 1.f}},
    };

   
    

   
    // The number of particles created per object at every frame
    //const int PARTICLES_PER_OBJ_PER_FRAME = 2;

    // The decay rate of particle. How much the alpha channel reduces at every frame
    const float DECAY_RATE = 0.005f;

    // The render system to draw particles
    RenderSystem* renderer;

    /// @brief Creates the particle
    /// @return The entity of the created particle
    //Entity createParticle(TEXTURE_ASSET_ID texture, Motion* motion_component_ptr);
};

