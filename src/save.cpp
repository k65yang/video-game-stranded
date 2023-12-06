#include "save.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

void to_json(json& j, const Player& p) {
    j = json{ 
        {"health", p.health}, 
        {"food", p.food}, 
        {"is_home", p.is_home}, 
        {"has_collected_quest_item", p.has_collected_quest_item}, 
        {"has_entered_spaceship", p.has_entered_spaceship}, 
    };
}

void from_json(const json& j, Player& p) {
    j.at("health").get_to(p.health);
    j.at("food").get_to(p.food);
    j.at("is_home").get_to(p.is_home);
    j.at("has_collected_quest_item").get_to(p.has_collected_quest_item);
    j.at("has_entered_spaceship").get_to(p.has_entered_spaceship);
}

void to_json(json& j, const Mob& mob) {
    j = json{ 
        {"damage", mob.damage}, 
        {"aggro_range", mob.aggro_range}, 
        {"health", mob.health}, 
        {"speed_ratio", mob.speed_ratio},  
        {"type", mob.type} };
}

void from_json(const json& j, Mob& mob) {
    j.at("damage").get_to(mob.damage);
    j.at("aggro_range").get_to(mob.aggro_range);
    j.at("health").get_to(mob.health);
    j.at("speed_ratio").get_to(mob.speed_ratio);
    j.at("type").get_to(mob.type);
}

void to_json(json& j, const Item& item) {
    j = json{ {"data", item.data } };
}

void from_json(const json& j, Item& item) {
    j.at("data").get_to(item.data);
}

void to_json(json& j, const Motion& m) {
    j = json{ {"position_x", m.position[0]}, {"position_y", m.position[1]}, {"angle", m.angle}, {"velocity_x", m.velocity[0]}, {"velocity_y", m.velocity[1]}, {"scale_x", m.scale[0]}, {"scale_y", m.scale[1]} };
}

void from_json(const json& j, Motion& m) {
    j.at("position_x").get_to(m.position[0]);
    j.at("position_y").get_to(m.position[1]);
    j.at("angle").get_to(m.angle);
    j.at("velocity_x").get_to(m.velocity[0]);
    j.at("velocity_y").get_to(m.velocity[1]);
    j.at("scale_x").get_to(m.scale[0]);
    j.at("scale_y").get_to(m.scale[1]);
}

void to_json(json& j, const Weapon& weapon) {
    j = json{
            {"can_fire", weapon.can_fire},
            {"weapon_type", weapon.weapon_type }, 
            {"ammo_count", weapon.ammo_count},
            {"level", weapon.level},
        };
}

void from_json(const json& j, Weapon& weapon) {
    j.at("can_fire").get_to(weapon.can_fire);
    j.at("weapon_type").get_to(weapon.weapon_type);
    j.at("ammo_count").get_to(weapon.ammo_count);
    j.at("level").get_to(weapon.level);
}

void to_json(json& j, const SpaceshipHome& spaceshipHome) {
    j = json{ {"health_storage", spaceshipHome.health_storage}, {"food_storage", spaceshipHome.food_storage}, {"ammo_storage", spaceshipHome.ammo_storage} };
}

void from_json(const json& j, SpaceshipHome& spaceshipHome) {
    j.at("health_storage").get_to(spaceshipHome.health_storage);
    j.at("food_storage").get_to(spaceshipHome.food_storage);
    j.at("ammo_storage").get_to(spaceshipHome.ammo_storage);
}

// NOTE: Currently saving every field of these structs - but this isn't necessary, just for cleanliness of code. If we need more space we can trim here :)
void SaveGame(
    Player& player, 
    Motion& player_motion,
    ITEM_TYPE active_weapon,
    std::vector<Weapon> weapons,
    std::vector<std::pair<Mob&, Motion&>> mobs, 
    std::vector<std::pair<Item&, Motion&>> items, 
    std::vector<QUEST_ITEM_STATUS> quest_item_statuses, 
    SpaceshipHome& spaceshipHome, 
    ITEM_TYPE powerup) {
    json data;

    data["player"] = player;
    data["player_motion"] = player_motion;
    data["active_weapon"] = active_weapon;
    data["weapons"] = weapons;
    data["mobs"] = mobs;
    data["items"] = items;
    data["quest_item_statuses"] = quest_item_statuses;
    data["spaceshipHome"] = spaceshipHome;
    data["powerup"] = powerup;

    // std::cout << data.dump(4);

    // Save/create into a file
    std::ofstream file;
    file.open("save.json");
    file << data;
    file.close();
}