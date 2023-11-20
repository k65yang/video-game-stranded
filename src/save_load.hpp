#include "common.hpp"
#include "components.hpp"
#include <nlohmann/json.hpp>

void SaveGame(Player& player, std::vector<std::pair<Mob&, Motion&>> mobs, std::vector<std::pair<Item&, Motion&>> items);
void LoadGame();



