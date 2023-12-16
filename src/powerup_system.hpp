#pragma once
#include "components.hpp"
#include "tiny_ecs.hpp"
#include "tiny_ecs_registry.hpp"
#include "render_system.hpp"
#include "particle_system.hpp"


class PowerupSystem 
{
	

	public:

		PowerupSystem()
		{
			this->particles = nullptr;
			this->renderer = nullptr;
			
			
		}


		~PowerupSystem() {
			registry.powerups.clear();
		}
		
		float old_speed;
		float start_duration_ms = 10000.0f;
		bool disable_pathfinding_invisible_powerup = false;
		bool disable_bullet_consumption = false;
		void init(RenderSystem* renderer_arg, ParticleSystem* particle_system_arg);


		// update durations for powerup, handle powerup deletion and cleanup
		void step(float elapsed_ms);


		// apply a powerup to the target entity
		void applyPowerup(POWERUP_TYPE powerup_type);
		//void loadPowerup(std::vector<Powerup> powerups);
		void resetPowerupSystem(Entity player_entity_arg);
		void setPowerup(float duration_ms, POWERUP_TYPE type_arg);

		


	private:
		// Pointer to rendering system 
		RenderSystem* renderer;

		// Pointer to particle systems
		ParticleSystem* particles;

		Entity player_entity;
		
		

		// Make sure that the heal interval is always larger than the light up interval
		
		float heal_interval_ms = 1000.f;				// Player health will increase after this interval
		float remaining_time_for_next_heal = 1000.f;	// Time since the player last healed
		int	heal_amount = 5;							// The amount healed

		float remaining_time_for_next_particle = 0.f; // used for delaying particle effects
		float particle_spawn_interval = 100.0f;

		/*
		float light_up_duration_ms = 1000.f;			// The health bar will light up to indicate healing
		float light_up_timer_ms = 300.f;					// The time remaining for health bar to be lit up
		*/

		void disablePowerupEffect(POWERUP_TYPE powerup_type);
		void applySpeedPowerup();
		void applyHealthRegenPowerup();
		void applyInvisiblePowerup();
		void applyInfiniteBulletPowerup();



};