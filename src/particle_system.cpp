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
Entity ParticleSystem::emit(ParticleTemplate temp) {
    // Reserve an entity
    auto entity = Entity();
    
    // Set particle components using template particle's entity
    registry.particles.emplace(entity);

    auto& entity_particle = registry.particles.get(entity);

    entity_particle.lifeTime = temp.lifeTime;
    entity_particle.lifeTimeRemaining = temp.lifeTimeRemaining;
    entity_particle.texture = temp.texture;
    entity_particle.sizeBegin = temp.sizeBegin;
    entity_particle.sizeEnd = temp.sizeEnd;
    entity_particle.active = temp.active;
    
    // Initialize the position, scale, and physics components
    registry.motions.emplace(entity);
    auto& entity_motion = registry.motions.get(entity);
    
    entity_motion.position = temp.position;
    entity_motion.velocity = temp.velocity;
    entity_motion.scale = vec2{ temp.sizeBegin, temp.sizeBegin };

    // Set the colour of the particle
    registry.colors.emplace(entity);
    registry.colors.get(entity) = temp.color;
  
    
    return entity;
}


// Creates particle trail effects for a specified entity with given texture, scale, number of particles each frame.
// Used for speed power up
void ParticleSystem::createParticleTrail(Entity targetEntity, TEXTURE_ASSET_ID texture, int numberOfParticles, vec2 scale) {

    // create the template particle
    ParticleTemplate temp;

    temp.active = true;
    temp.lifeTime = 500.f;
    temp.lifeTimeRemaining = 500.f;
    temp.texture = texture;
    
    // Ideally this size should be two dimension....
    temp.sizeBegin = scale.x;
    temp.sizeEnd = scale.x;

    temp.position = registry.motions.get(targetEntity).position;
    temp.color = registry.colors.get(targetEntity);


    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    std::vector<Entity> entities;

    
    // Create particles based on template
    for (int i = 0; i < numberOfParticles; i++) {
        ParticleTemplate sample = temp;

        // randomize the position
        sample.position = temp.position + vec2(dist(gen), dist(gen));
        entities.push_back(emit(sample));

    }

    // Create one instanced render request 
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
    ParticleTemplate temp;

    temp.active = true;
    temp.lifeTime = 2000.f;
    temp.lifeTimeRemaining = 2000.f;
    temp.texture = texture;
    temp.sizeBegin = 0.3f;
    temp.sizeEnd = 0.0f;
    temp.position = registry.motions.get(targetEntity).position;
    temp.velocity = {0.0f, -3.0f};
    temp.color = registry.colors.get(targetEntity);


    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    std::vector<Entity> entities;


    // Create particles based on template
    for (int i = 0; i < numberOfParticles; i++) {
        ParticleTemplate sample = temp;

        // randomize the x, y position and scale of hearts
        sample.sizeBegin =  temp.sizeBegin += dist(gen);
        sample.position.x = temp.position.x + 4 * dist(gen);
        sample.position.y = temp.position.y + 4 * dist(gen);
        sample.velocity.y = temp.velocity.y + 10 * dist(gen);

        entities.push_back(emit(sample));

    }

    // Ccreate one instanced render request 
    registry.instancedRenderRequests.insert(
        entities[0],
        { entities,
            registry.particles.get(entities[0]).texture,
            EFFECT_ASSET_ID::TEXTUREPARTICLE,
            GEOMETRY_BUFFER_ID::SPRITE,
            RENDER_LAYER_ID::LAYER_2
        });

}


// Create floating heart particle effect for the health power up
void ParticleSystem::createFloatingBullet(Entity targetEntity, int numberOfParticles) {


    // create the template particle
    ParticleTemplate temp;

    temp.active = true;
    temp.lifeTime = 1500.f;
    temp.lifeTimeRemaining = 1500.f;
    temp.sizeBegin = 0.5f;
    temp.sizeEnd = 0.0f;
    temp.position = registry.motions.get(targetEntity).position;
    temp.velocity = { 0.0f, -2.0f };
    temp.color = registry.colors.get(targetEntity);


    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    std::vector<Entity> entities;

    // Create particles based on template
    for (int i = 0; i < numberOfParticles; i++) {
        ParticleTemplate sample = temp;

        // randomize the x, y position and scale of hearts
        sample.sizeBegin = temp.sizeBegin += dist(gen);
        sample.position.x = temp.position.x + 4 * dist(gen);
        sample.position.y = temp.position.y + 5 * dist(gen);
        sample.velocity.y = temp.velocity.y + 10 * dist(gen);

        

        std::uniform_int_distribution<int> dist(0, 3);

        switch (dist(gen)) {
            case 0:
                sample.texture = TEXTURE_ASSET_ID::WEAPON_SHURIKEN;
                break;
            case 1:
                sample.texture = TEXTURE_ASSET_ID::WEAPON_CROSSBOW;
                break;
            case 2:
                sample.texture = TEXTURE_ASSET_ID::WEAPON_SHOTGUN;
                break;
            case 3:
                sample.texture = TEXTURE_ASSET_ID::WEAPON_MACHINEGUN;
                break;
            default:
                sample.texture = TEXTURE_ASSET_ID::WEAPON_SHURIKEN;
                break;
        
        }
       
       
        entities.push_back(emit(sample));

    }

    // Ccreate one instanced render request 
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

    // Create the template particle
    ParticleTemplate temp;
    
    temp.active = true;
    temp.lifeTime = 400.f;
    temp.lifeTimeRemaining = 500.f;
    temp.sizeBegin = 0.6f;
    temp.sizeEnd = 0.0f;
    temp.position = projectile_motion.position;
    temp.velocity = 10.f * splashDirection;
    temp.color = 1.2f * vec4{mob_color_map.at(registry.mobs.get(mob_entity).type), 1.0f};

   

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-1, 1);

    std::vector<Entity> entities;
  
    // Emit particles based on this template and number of particles per frame
    for (int i = 0; i < numberOfParticles; i++) {
        
        ParticleTemplate sampleParticle = temp;

        sampleParticle.sizeBegin = temp.sizeBegin += (0.2 * dist(gen));
        sampleParticle.velocity = rotateByDegree(temp.velocity, dist(gen) * 20);
        sampleParticle.velocity.x = temp.velocity.x + 3.f * dist(gen);
        sampleParticle.velocity.y = temp.velocity.y + 3.f * dist(gen);

        entities.push_back(emit(sampleParticle));
        
    }

    // Create instanced render request 
    registry.instancedRenderRequests.insert(entities[0], {
        entities,
        TEXTURE_ASSET_ID::TEXTURE_COUNT,
            EFFECT_ASSET_ID::PARTICLE,
            GEOMETRY_BUFFER_ID::PEBBLE,
            RENDER_LAYER_ID::LAYER_2
            });

}

void ParticleSystem::createWaterSplash(Entity targetEntity, int numberOfParticles, float speedRatio) {

    // create the template particle
    ParticleTemplate temp;

    temp.active = true;
    temp.lifeTime = 1000.f;
    temp.lifeTimeRemaining = 1000.f;
    temp.sizeBegin = 1.0f;
    temp.sizeEnd = 0.0f;
    temp.position = registry.motions.get(targetEntity).position;
    temp.velocity = { 0.0f, -3.0f };
    if (speedRatio == 0.40f) {
        temp.color = vec4{ 0.28f, 0.67f , 0.49f, 1.00f };
    }
    else {
        temp.color = vec4{ 0.00f, 0.54f, 0.33f, 1.00f };
    }
    

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
    std::vector<Entity> entities;

    // Create particles based on template
    for (int i = 0; i < numberOfParticles; i++) {
        ParticleTemplate sample = temp;

        // randomize the x, y position and scale of hearts
        sample.sizeBegin = temp.sizeBegin += 1.5f * dist(gen);
        sample.position.x = temp.position.x + 4 * dist(gen);
        sample.position.y = temp.position.y + 4 * dist(gen);
        sample.velocity.y = temp.velocity.y + 10 * dist(gen);

        entities.push_back(emit(sample));

    }

    // Create instanced render request 
    registry.instancedRenderRequests.insert(entities[0], {
        entities,
        TEXTURE_ASSET_ID::TEXTURE_COUNT,
            EFFECT_ASSET_ID::PARTICLE,
            GEOMETRY_BUFFER_ID::PEBBLE,
            RENDER_LAYER_ID::LAYER_2
        });


}
