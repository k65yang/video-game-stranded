#include "common.hpp"
#include "components.hpp"
#include <nlohmann/json.hpp>

void SaveGame(
    Player& player, 
    Motion& player_motion, 
    ITEM_TYPE active_weapon,
    std::vector<Weapon> weapons,
    std::vector<std::pair<Mob&, Motion&>> mobs, 
    std::vector<std::pair<Item&, Motion&>> items, 
    std::vector<QUEST_ITEM_STATUS> quest_item_statuses,
    SpaceshipHome& spaceshipHome,
    std::vector<Powerup> powerups);
