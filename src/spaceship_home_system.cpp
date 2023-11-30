#include "spaceship_home_system.hpp"

void SpaceshipHomeSystem::step(float elapsed_ms) {

};

Entity SpaceshipHomeSystem::createSpaceshipHome(vec2 position, bool is_inside, int food_storage, int ammo_storage) {
    return Entity();
};

Entity SpaceshipHomeSystem::createSpaceshipHomeItem(vec2 position, ITEM_TYPE type) {
    return Entity();
};

void SpaceshipHomeSystem::enterSpaceship() {

};

void SpaceshipHomeSystem::resetSpaceshipHome() {
    
};

bool SpaceshipHomeSystem::isHome() {
    return true;
};

void SpaceshipHomeSystem::updateSpaceshipHomeUI() {

};

void SpaceshipHomeSystem::regenerateStat(int& stat, int& storage, int max_stat_value) {

};

void SpaceshipHomeSystem::updateBar(int new_val, Motion& bar, int max_bar_value, vec2 scale_factor, bool is_stat) {

};
