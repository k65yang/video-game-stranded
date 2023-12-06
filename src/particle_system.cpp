#include "particle_system.hpp"

vec2 rotateByDegree(vec2 vector, float angleInDegree) {

    float angle = glm::radians(angleInDegree);
    glm::mat2 rotationMatrix = glm::mat2(cos(angle), -sin(angle), sin(angle), cos(angle));

    return rotationMatrix * vector;
}

// reference from wiki: https://en.wikipedia.org/wiki/Linear_interpolation#:~:text=In%20mathematics%2C%20linear%20interpolation%20is,set%20of%20known%20data%20points.
float lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

// reference on particle system: https://www.youtube.com/watch?v=GK0jHlv3e3w
void ParticleSystem::step(float elapsed_ms) {
    // Iterate through all particles
    auto& particle_component_container = registry.particles.components;
    auto& particle_entity_container = registry.particles.entities;

    for (int i = 0; i < particle_entity_container.size(); i++) {
        auto& particle = particle_component_container[i];

        if (!particle.active) {

            continue;
        }

        // kills the particle when no lifetime left
        if (particle.lifeTimeRemaining <= 0.0f) {
        
            particle.active = false; //unnessary?
            registry.remove_all_components_of(particle_entity_container[i]);

            continue;

        }

        // updates lifetime remaining
        particle.lifeTimeRemaining -= elapsed_ms;

        float life = (particle.lifeTimeRemaining / particle.lifeTime);

        // updates alpha

        if (registry.colors.has(particle_entity_container[i])) {
            registry.colors.get(particle_entity_container[i]).a *= life;
        }

        // updates size
        if (registry.motions.has(particle_entity_container[i])) {
            registry.motions.get(particle_entity_container[i]).scale = vec2(lerp(particle.sizeEnd, particle.sizeBegin, life));
        }

   
    }
}



// Create a particle entity based on the template particle 
Entity ParticleSystem::emit(Entity templateParticleEntity) {
    // Reserve an entity
    auto entity = Entity();
    
    // Set particle components using template particle's entity
    registry.particles.emplace(entity);

    auto& template_particle = registry.particles.get(templateParticleEntity);
    auto& entity_particle = registry.particles.get(entity);

    entity_particle.lifeTime = template_particle.lifeTime;
    entity_particle.lifeTimeRemaining = template_particle.lifeTime;
    entity_particle.texture = template_particle.texture;
    entity_particle.sizeBegin = template_particle.sizeBegin;
    entity_particle.sizeEnd = template_particle.sizeEnd;
    entity_particle.active = true;

    // Initialize the position, scale, and physics components
    registry.motions.emplace(entity);

    auto& template_motion = registry.motions.get(templateParticleEntity);
    auto& entity_motion = registry.motions.get(entity);
    
    entity_motion.position = template_motion.position;
    entity_motion.velocity = template_motion.velocity;
    entity_motion.scale = template_motion.scale;

    // Set the colour of the particle
    registry.colors.emplace(entity);

    auto& template_color = registry.colors.get(templateParticleEntity);
    auto& entity_color = registry.colors.get(entity);

    entity_color = template_color;


    
    return entity;
}

// Creates particle trail effects for a specified entity with given texture, scale, number of particles each frame.
// Used for speed power up
void ParticleSystem::createParticleTrail(Entity targetEntity, TEXTURE_ASSET_ID texture, int numberOfParticles, vec2 scale) {

    // create the template particle
    auto template_entity = Entity();

    // particle component for template
    auto& template_particle = registry.particles.emplace(template_entity);
    template_particle.active = false;
    template_particle.lifeTime = 500.f;
    template_particle.lifeTimeRemaining = 0.f;
    template_particle.texture = texture;

    // not used for now since the speed effects doesnt need it
    template_particle.sizeBegin = scale.x;
    template_particle.sizeEnd = scale.x;


    // motion component for template
    auto& template_motion = registry.motions.emplace(template_entity);
    template_motion.scale = scale;
    template_motion.position = registry.motions.get(targetEntity).position;
    


    // color component for template using target color scheme
    auto& template_color = registry.colors.emplace(template_entity);
    template_color = registry.colors.get(targetEntity);


    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    std::vector<Entity> entities;

    
    // Create particles based on template
    for (int i = 0; i < numberOfParticles; i++) {

        // randomize the position
        template_motion.position = template_motion.position + vec2(dist(gen), dist(gen));
        entities.push_back(emit(template_entity));

    }

    // create one instanced render request 
    registry.instancedRenderRequests.insert(
        entities[0],
        { entities,
            registry.particles.get(entities[0]).texture,
            EFFECT_ASSET_ID::TEXTUREPARTICLE,
            GEOMETRY_BUFFER_ID::SPRITE,
            RENDER_LAYER_ID::LAYER_1
            });

    
}

// Create floating heart particle effect for the health power up
void ParticleSystem::createFloatingHeart(Entity targetEntity, TEXTURE_ASSET_ID texture, int numberOfParticles) {

  

    // create the template particle
    auto template_entity = Entity();

    // set up particle template
    auto& template_particle = registry.particles.emplace(template_entity);
    template_particle.active = false;
    template_particle.lifeTime = 2000.f;
    template_particle.lifeTimeRemaining = 0.f;
    template_particle.texture = texture;
    template_particle.sizeBegin = 0.3f;
    template_particle.sizeEnd = 0.0f;

    // motion component for template
    auto& template_motion = registry.motions.emplace(template_entity);

    // scale is not used
    template_motion.scale = vec2{1.0f, 1.0f};
    template_motion.position = registry.motions.get(targetEntity).position;
    template_motion.velocity.y = -3.f;

    
    // color component for template using target color scheme
    auto& template_color = registry.colors.emplace(template_entity);
    template_color = registry.colors.get(targetEntity);


    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    std::vector<Entity> entities;


    // Create particles based on template
    for (int i = 0; i < numberOfParticles; i++) {

        // randomize the x, y position and scale of hearts
        template_particle.sizeBegin += dist(gen);
        template_motion.position.x += 4 * dist(gen);
        template_motion.position.y += 4 * dist(gen);
        template_motion.velocity.y += 10 * dist(gen);

        entities.push_back(emit(template_entity));

    }

    // create one instanced render request 
    registry.instancedRenderRequests.insert(
        entities[0],
        { entities,
            registry.particles.get(entities[0]).texture,
            EFFECT_ASSET_ID::TEXTUREPARTICLE,
            GEOMETRY_BUFFER_ID::SPRITE,
            RENDER_LAYER_ID::LAYER_2
        });

}



// create particles effects that start from contact point, and 
// spreads in a specified direction with a cone angle spread, with specified particle amounts and color
void ParticleSystem::createParticleSplash(Entity projectile_entity, Entity mob_entity, int numberOfParticles, vec2 splashDirection) {

    auto& projectile_motion = registry.motions.get(projectile_entity);

    // 1. create the template particle
    auto template_entity = Entity();

    // particle component for template
    auto& template_particle = registry.particles.emplace(template_entity);
    template_particle.active = false;
    template_particle.lifeTime = 400.f;
    template_particle.lifeTimeRemaining = 0.f;
    template_particle.texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;

        // dynamic size not supported for now
    template_particle.sizeBegin = 1.0f;
    template_particle.sizeEnd = 0.0f;

    // motion component for template using projectiles motion
    auto& template_motion = registry.motions.emplace(template_entity);
    template_motion.scale = projectile_motion.scale;
    template_motion.position = projectile_motion.position;
    
        // using inverted projectiles velocity for spalsh direction
    template_motion.velocity = 10.f * splashDirection;

        // adjust angle to facing in opposite direction. might not be needed 
    //template_motion.angle =  projectile_motion.angle + M_PI;
    
    // color component for template using mob color scheme
    auto& template_color = registry.colors.emplace(template_entity);
    template_color = 1.2f * vec4{mob_color_map.at(registry.mobs.get(mob_entity).type), 1.0f};

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-1, 1);
    std::vector<Entity> entities;
    Transform t;
  
    // 2. emit particles based on this template and number of particles per frame
    for (int i = 0; i < numberOfParticles; i++) {
        
        
        // rotate velocity by +- 18 degree maximum randomly
        //float angle = (M_PI/10) * dist(gen);
       // t.rotate(angle);

        template_motion.velocity = rotateByDegree(template_motion.velocity, dist(gen) * 20);
        //vec3 newVelocity = t.mat * vec3{ template_motion.velocity,1.0f};
        //template_motion.velocity.x = newVelocity.x;
       // template_motion.velocity.y = newVelocity.y;

        template_motion.velocity.x = template_motion.velocity.x + 5.f * dist(gen);
        template_motion.velocity.y = template_motion.velocity.y + 5.f * dist(gen);

        entities.push_back(emit(template_entity));
        
        

    }

    // create one render request 
    registry.instancedRenderRequests.insert(entities[0], {
        entities,
        TEXTURE_ASSET_ID::TEXTURE_COUNT,
            EFFECT_ASSET_ID::PARTICLE,
            GEOMETRY_BUFFER_ID::PEBBLE,
            RENDER_LAYER_ID::LAYER_2
            });
}

// create particles effects that start from contact point, and 
// spreads in a specified direction with a cone angle spread, with specified particle amounts and color
void ParticleSystem::createMuzzleFlash(vec2 playerPosition, int playerDirection, int numberOfParticles) {

   

    // 1. create the template particle
    auto template_entity = Entity();

    // template for particle component. 
    auto& template_particle = registry.particles.emplace(template_entity);
    template_particle.active = false;
    template_particle.lifeTime = 200.f;
    template_particle.lifeTimeRemaining = 0.f;
    template_particle.texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
    template_particle.sizeBegin = 1.0f;
    template_particle.sizeEnd = 0.0f;

    // motion component for template using projectiles motion
    auto& template_motion = registry.motions.emplace(template_entity);
    template_motion.scale = vec2{ 1.0f };
    template_motion.position = playerPosition;

    // using inverted projectiles velocity for spalsh direction
    if (playerDirection == 2) {
    //right
        template_motion.position.x += 1.0f;
        template_motion.velocity.x = 1.0f;
    }
    else if (playerDirection == 4) {
    // down
        template_motion.position.y += 1.0f;
        template_motion.velocity.y = 1.0f;

    }
    else if (playerDirection == 0) {
    // up
        template_motion.position.y -= 1.0f;
        template_motion.velocity.y = -1.0f;

    }
    else {
    //left
        template_motion.position.x -= 1.0f;
        template_motion.velocity.x = -1.0f;

    }



    auto& template_color = registry.colors.emplace(template_entity);
    template_color = vec4{ 1.0f, 0.0f, 0.0f, 1.0f };

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-1, 1);
    std::vector<Entity> entities;
    Transform t;

    // 2. emit particles based on this template and number of particles per frame
    for (int i = 0; i < numberOfParticles; i++) {


        // rotate velocity by +- 18 degree maximum randomly
        //float angle = (M_PI/10) * dist(gen);
       // t.rotate(angle);

        template_motion.velocity = rotateByDegree(template_motion.velocity, dist(gen) * 20);
        //vec3 newVelocity = t.mat * vec3{ template_motion.velocity,1.0f};
        //template_motion.velocity.x = newVelocity.x;
       // template_motion.velocity.y = newVelocity.y;

        //template_motion.velocity.x += 2.f * (dist(gen) + 1);
        //template_motion.velocity.y += 2.f * (dist(gen) + 1);

        entities.push_back(emit(template_entity));



    }

    // create one render request 
    registry.instancedRenderRequests.insert(entities[0], {
        entities,
        TEXTURE_ASSET_ID::TEXTURE_COUNT,
            EFFECT_ASSET_ID::PARTICLE,
            GEOMETRY_BUFFER_ID::PEBBLE,
        });
}




/*
void ParticleSystem::step(float elapsed_ms) {
    // Iterate through all existing particle trails
    auto& particle_trail_container = registry.particleTrails;
    std::vector<Entity> particles_to_remove;
    for (uint i = 0; i < particle_trail_container.size(); i++) {

        // Iterate through the list of particles for each particle trail
        ParticleTrail& particle_trail = particle_trail_container.components[i];

        for (auto& entity_particle : particle_trail.particles) {
            // Update the alpha of existing particles
            // printf("getting particle entity: %i\n", entity_particle);
            auto& particle_color = registry.colors.get(entity_particle);
            particle_color.a -= elapsed_ms * DECAY_RATE;

            // Remove particle if it has decayed
            if (particle_color.a <= 0) {
                registry.remove_all_components_of(entity_particle);
                particles_to_remove.push_back(entity_particle);
            }
        }

        // The decayed particles have been removed from registry, but they still need
        // to be removed from the ParticleTrail struct
        for (auto& p : particles_to_remove) {
            // printf("removing particle entity: %i\n", (int)p);
            particle_trail.particles.erase(p);
        }

        // Now generate new particles for the particle trail if the trail is still alive
        if (particle_trail.is_alive && (int)particle_trail.particles.size() < NUM_PARTICLES - PARTICLES_PER_OBJ_PER_FRAME) {
            for (int j = 0; j < PARTICLES_PER_OBJ_PER_FRAME; j++) {
                particle_trail.particles.insert(
                    createParticle(particle_trail.texture, particle_trail.motion_component_ptr)
                );
            }
        }
    }

    for (auto& particle_trail_entity: registry.particleTrails.entities) {
        if (registry.particleTrails.get(particle_trail_entity).particles.size() == 0) {
            registry.particleTrails.remove(particle_trail_entity);
        }
    }

    // particle_trails_to_remove.clear();
    particles_to_remove.clear();
}
*/

/*
Entity ParticleSystem::createParticle(TEXTURE_ASSET_ID texture, Motion* motion_component_ptr) {
    // Reserve an entity
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = vec2(0.f);
    motion.scale = vec2({ .7f, 1.f });

    // Random number generator

    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    motion.position = motion_component_ptr->position + vec2(dist(random), dist(gen));

    // Add this particle to the particles registry
    auto& particles = registry.particles.emplace(entity);
    // printf("Added particle entity: %i\n", (int)entity);

    // Set the colour of the particle
    auto& color = registry.colors.emplace(entity);
    color = vec4(1.f, 0.8f, 0.8f, 1.f); // (1, .8, .8) is the rgb colour scheme of the player

    // Add the particle to the render requests
    registry.renderRequests.insert(
        entity,
        { texture,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE,
            RENDER_LAYER_ID::LAYER_1 });
    return entity;
}
*/