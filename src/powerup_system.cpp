#include "powerup_system.hpp"

void PowerupSystem::init(RenderSystem* renderer_arg, ParticleSystem* particle_system_arg) {
	this->renderer = renderer_arg;
	this->particles = particle_system_arg;
	
};

void PowerupSystem::step(float elapsed_ms) {

	auto& powerups_component_container = registry.powerups.components;
	auto& player_component = registry.players.get(player_entity);

	for (int i = 0; i < powerups_component_container.size(); i++) {
		auto& powerup = powerups_component_container[i];

		powerup.duration_ms -= elapsed_ms;

		if (powerup.duration_ms < 0) {

			powerup.duration_ms = 0;
			disablePowerupEffect(powerup.type);
			
			
		} else if (powerup.type == POWERUP_TYPE::HEALTH_REGEN) {
			
			remaining_time_for_next_heal -= elapsed_ms;

			if (player_component.health < PLAYER_MAX_HEALTH && remaining_time_for_next_heal <= 0.0f) {
				// heal player
				remaining_time_for_next_heal = heal_interval_ms;
				player_component.health = std::min(PLAYER_MAX_HEALTH, player_component.health + heal_amount);

				// creating particles effects for healing
				particles->createFloatingHeart(player_entity, TEXTURE_ASSET_ID::HEART_PARTICLE, 6);

				
			}
		} else if (powerup.type == POWERUP_TYPE::SPEED) {
			particles->createParticleTrail(player_entity, TEXTURE_ASSET_ID::PLAYER_PARTICLE, 2, vec2{ 0.7f, 1.0f });
		}


	}

}

// apply a powerup to the target entity
void PowerupSystem::applyPowerup(POWERUP_TYPE powerup_type) {


	switch (powerup_type)
	{
	case POWERUP_TYPE::SPEED:
		applySpeedPowerup();
		break;
	case POWERUP_TYPE::HEALTH_REGEN:
		applyHealthRegenPowerup();
		break;
	case POWERUP_TYPE::INVISIBLE:
		break;
	case POWERUP_TYPE::INFINITE_BULLET:
		break;
	default:
		break;
	}

		
}

void PowerupSystem::disablePowerupEffect(POWERUP_TYPE powerup_type) {
	auto& player_component = registry.players.get(player_entity);
	switch (powerup_type)
	{
	case POWERUP_TYPE::SPEED:
		// reset speed
		player_component.current_speed = old_speed;

		break;
	case POWERUP_TYPE::HEALTH_REGEN:
		// nothing to reset

		break;
	case POWERUP_TYPE::INVISIBLE:
		// set mob tracking back on

		break;
	case POWERUP_TYPE::INFINITE_BULLET:
		// set infinite bullet ammo back on
		break;

	default:
		break;
	}


}

// reset speed powerup timer, increase speed if needed
void PowerupSystem::applySpeedPowerup() {

	// loop through powerups to get speed power up
	for (auto& powerup : registry.powerups.components) {
		if (powerup.type == POWERUP_TYPE::SPEED) {

			// re-apply speed if timer was zero
			if (powerup.duration_ms <= 0) {

				auto& player_component = registry.players.get(player_entity);

				old_speed = player_component.current_speed;
				player_component.current_speed *= 2;
			}

			// extend timer
			powerup.duration_ms = start_duration_ms;
			std::cout << (int)powerup.type << "'s duration is EXTENED " << powerup.duration_ms << std::endl;

		}
	
	}

	
}

// reset health regen timer
void PowerupSystem::applyHealthRegenPowerup() {

	// loop through powerups to get speed power up
	for (auto& powerup : registry.powerups.components) {
		if (powerup.type == POWERUP_TYPE::HEALTH_REGEN) {

			// re-apply speed if timer was zero
			if (powerup.duration_ms <= 0) {
				remaining_time_for_next_heal = 0.0f;
			}

			// extend timer
			powerup.duration_ms = start_duration_ms;
			std::cout << (int)powerup.type << "'s duration is EXTENED " << powerup.duration_ms << std::endl;

		}

	}


	/*
	
		// remove the speed power up if already equipped
	if (registry.speedPowerup.has(player_salmon)) {
		// reset speed to original speed
		current_speed = registry.speedPowerup.get(player_salmon).old_speed;
		registry.speedPowerup.remove(player_salmon);

	}

	// Give health powerup to player. Use default values in struct definition.
	registry.healthPowerup.emplace(player_salmon);

	// Add the powerup indicator
	if (user_has_powerup)
	registry.remove_all_components_of(powerup_indicator);
	powerup_indicator = createPowerupIndicator(renderer, { 9.5f + 0.5, 6.f }, TEXTURE_ASSET_ID::ICON_POWERUP_HEALTH);
	user_has_powerup = true;

	
	


	




	// health updates
	if (registry.healthPowerup.has(player_salmon)) {
		HealthPowerup& hp = registry.healthPowerup.get(player_salmon);

		// update the time since last heal and the light up duration
		hp.remaining_time_for_next_heal -= elapsed_ms_since_last_update;
		hp.light_up_timer_ms -= elapsed_ms_since_last_update;

		// light up expired
		if (hp.light_up_timer_ms < 0 && registry.colors.has(health_bar)) {
			registry.colors.remove(health_bar);
		}

		// check if we need and can heal
		



			// light up the health bar
			if (registry.colors.has(health_bar)) {
				// can happen when the light up period is longer than the heal interval
			}
			else {
				vec4& color = registry.colors.emplace(health_bar);
				color = vec4(.5f, .5f, .5f, 1.f);
			}
			hp.light_up_timer_ms = hp.light_up_duration_ms;

		}
	}
	
	*/
}



// reset powerup system on game restart
void PowerupSystem::resetPowerupSystem(Entity player_entity_arg) {

	registry.powerups.clear();

	auto& player_component = registry.players.get(player_entity_arg);

	// initialize old speed as current speed;
	old_speed = player_component.current_speed;

	// store player entity reference
	player_entity = player_entity_arg;
	
	// initialize powerups for player by attaching all powerups 
	// duration are intialized with 0.0f
	// disables effect for each powerup
	auto& powerup = registry.powerups.emplace_with_duplicates(player_entity);
	powerup.type = POWERUP_TYPE::SPEED;
	disablePowerupEffect(powerup.type);

	auto& powerup1 = registry.powerups.emplace_with_duplicates(player_entity);
	powerup1.type = POWERUP_TYPE::HEALTH_REGEN;
	disablePowerupEffect(powerup1.type);

	auto& powerup2 = registry.powerups.emplace_with_duplicates(player_entity);
	powerup2.type = POWERUP_TYPE::INVISIBLE;
	disablePowerupEffect(powerup2.type);

	auto& powerup3 = registry.powerups.emplace_with_duplicates(player_entity);
	powerup3.type = POWERUP_TYPE::INFINITE_BULLET;
	disablePowerupEffect(powerup3.type);
}

void PowerupSystem::setPowerup(float duration_ms, POWERUP_TYPE type_arg) {

	for (auto& powerup : registry.powerups.components) {
		if (powerup.type == type_arg) {
			powerup.duration_ms = duration_ms;
			std::cout << powerup.duration_ms << " is loaded to type " << (int)powerup.type << std::endl;
		}
	
	}

}




