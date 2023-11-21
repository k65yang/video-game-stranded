#include "common.hpp"
#include "components.hpp"
#include <nlohmann/json.hpp>

void SaveGame(Player& player, Motion& player_motion, std::vector<std::pair<Mob&, Motion&>> mobs, std::vector<std::pair<Item&, Motion&>> items, std::vector<bool> quests, ITEM_TYPE weapon);
