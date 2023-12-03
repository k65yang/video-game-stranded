#pragma once

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include <iostream>
#include <random>



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
    
    void ParticleSystem::emit(Entity templateParticleEntity);
    void ParticleSystem::createParticleTrail(Entity targetEntity, TEXTURE_ASSET_ID texture, int numberOfParticles, vec2 scale);
    void ParticleSystem::createParticleSplash();

    private:


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

