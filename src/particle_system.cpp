#include "particle_system.hpp"

void ParticleSystem::step(float elapsed_ms) {
    // Iterate through all existing particle trails
    auto& particle_trail_container = registry.particleTrails;
    std::vector<int> particle_trails_to_remove;
    std::vector<Entity> particles_to_remove;
    for (uint i = 0; i < particle_trail_container.size(); i++) {

        // Iterate through the list of particles for each particle trail
        ParticleTrail& particle_trail = particle_trail_container.components[i];

        // Track completely dead particle trails
        if (!particle_trail.is_alive && particle_trail.particles.size() == 0) {
            particle_trails_to_remove.push_back(i);
            continue;
        }

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

    for (auto& idx_particle_trail: particle_trails_to_remove) {
        // Erase the particle trails from the container by index
        particle_trail_container.components.erase(
            particle_trail_container.components.begin() + idx_particle_trail);
    }
}

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
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-0.1, 0.1);
	motion.position = motion_component_ptr->position + vec2(dist(gen), dist(gen));

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
