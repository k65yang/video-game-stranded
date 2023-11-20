#include "save_load.hpp"

using json = nlohmann::json;

void SaveGame(Player& player, std::vector<std::pair<Mob&, Motion&>> mobs, std::vector<std::pair<Item&, Motion&>> items) {
	json data;

    // below is using to and from json
    data["player"] = player;
    data["mobs"] = mobs;
    data["items"] = items;

    // Save/create into a file
    std::ofstream file("save.json");
    file << data;
}

void LoadGame() {

}



void to_json(json& j, const Player& p) {
    j = json{ {"health", p.health}, {"food", p.food} };
}

// FILL out the rest of the player struct with defaults? if needed
void from_json(const json& j, Player& p) {
    j.at("health").get_to(p.health);
    j.at("food").get_to(p.food);
}

void to_json(json& j, const Mob& mob) {
    j = json{ {"damage", mob.damage}, {"aggro_range", mob.aggro_range}, {"health", mob.health}, {"speed_ratio", mob.speed_ratio},  {"type", mob.type} };
}

// FILL out the rest of the mob struct with defaults? if needed
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