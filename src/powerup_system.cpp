#include "powerup_system.hpp"

void PowerupSystem::init(RenderSystem* renderer_arg, ParticleSystem* particle_system_arg) {
	this->renderer = renderer_arg;
	this->particles = particle_system_arg;
	this->player_entity = registry.players.entities[0];

};

void PowerupSystem::step(float elapsed_ms) {

	auto& powerups_component_container = registry.powerups.components;
	

	for (Powerup powerup : powerups_component_container) {
		powerup.duration_ms -= elapsed_ms;

		if (powerup.duration_ms < 0) {
			powerup.duration_ms = 0;

			removePowerupEffect(powerup.type);
		}

		if (powerup.type == POWERUP_TYPE::HEALTH_REGEN) {
			
		
		}
	
	
	}



}

// apply a powerup to the target entity
void PowerupSystem::applyPowerup(POWERUP_TYPE powerup_type) {


	switch (powerup_type) {

		case POWERUP_TYPE::SPEED:
			applySpeedPowerup();
			break;
	
	}

		
}

void PowerupSystem::removePowerupEffect(POWERUP_TYPE powerup_type) {



}

void PowerupSystem::applySpeedPowerup() {

	auto& powerup = registry.powerups.emplace_with_duplicates(player_entity);
	powerup.type = POWERUP_TYPE::SPEED;

	auto& motion = registry.motions.get(player_entity);
	
	auto& player_component = registry.players.get(player_entity);
	


	/*
	
	// remove health power up if already equipped
						if (registry.healthPowerup.has(player_salmon)) {
							// check if the health bar is lit up (i.e. the player just healed)
							HealthPowerup& healthPowerUp = registry.healthPowerup.get(player_salmon);
							if (healthPowerUp.light_up_timer_ms > 0 || registry.colors.has(health_bar) ) {
								registry.colors.remove(health_bar);
							}
							registry.healthPowerup.remove(player_salmon);
						}
						
						// Give the speed power up to the player
						SpeedPowerup& speedPowerup = registry.speedPowerup.emplace(player_salmon);
						speedPowerup.old_speed = current_speed;
						current_speed *= 2;

		

						// Add the powerup indicator
						if (user_has_powerup)
							registry.remove_all_components_of(powerup_indicator);
						powerup_indicator = createPowerupIndicator(renderer, {9.5f+0.5, 6.f}, TEXTURE_ASSET_ID::ICON_POWERUP_SPEED);
						user_has_powerup = true;
	

	// creating particles effects for character upgrades
	if (registry.speedPowerup.has(player_salmon)) {
		particle_system->createParticleTrail(player_salmon, TEXTURE_ASSET_ID::PLAYER_PARTICLE, 2, vec2{ 0.7f, 1.0f });
	}
	*/
}

void PowerupSystem::applyHealthRegenPowerup() {


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
		if (player.health < PLAYER_MAX_HEALTH && hp.remaining_time_for_next_heal < 0) {
			Motion& health = registry.motions.get(health_bar);
			hp.remaining_time_for_next_heal = hp.heal_interval_ms;
			player.health = std::min(PLAYER_MAX_HEALTH, player.health + hp.heal_amount);

			// creating particles effects for healing

			particle_system->createFloatingHeart(player_salmon, TEXTURE_ASSET_ID::HEART_PARTICLE, 6);



			// light up the health bar
			if (registry.colors.has(health_bar)) {
				// can happen when the light up period is longer than the heal interval
			}
			else {
				vec4& color = registry.colors.emplace(health_bar);
				color = vec4(.5f, .5f, .5f, 1.f);
			}
			hp.light_up_timer_ms = hp.light_up_duration_ms;

			vec2 new_health_scale = vec2(((float)player.health / (float)PLAYER_MAX_HEALTH) * HEALTH_BAR_SCALE[0], HEALTH_BAR_SCALE[1]);
			health.scale = interpolate(health.scale, new_health_scale, 1);
		}
	}
	
	*/
}

void PowerupSystem::loadPowerup(std::vector<Powerup> powerups) {

}

void PowerupSystem::resetPowerupSystem() {


}




