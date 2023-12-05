#include "quest_system.hpp"
#include "world_init.hpp"


void QuestSystem::step(float elapsed_ms) {

};

void QuestSystem::init(RenderSystem* render_system_arg) {
    this->renderer = render_system_arg;
};

void QuestSystem::resetQuestSystem(std::vector<QUEST_ITEM_STATUS> statuses) {
    Entity player = registry.players.entities[0];
    
    // Set quest item status
    Inventory& inventory = registry.inventories.get(player);
    inventory.quest_items[ITEM_TYPE::QUEST_ONE] = statuses[0];
    inventory.quest_items[ITEM_TYPE::QUEST_TWO] = statuses[1];
    inventory.quest_items[ITEM_TYPE::QUEST_THREE] = statuses[2];
    inventory.quest_items[ITEM_TYPE::QUEST_FOUR] = statuses[3];

    // Create quest item indicators
    createQuestItemIndicator(QUEST_1_INDICATOR_POSITION, ITEM_TYPE::QUEST_ONE, statuses[0]);
    createQuestItemIndicator(QUEST_2_INDICATOR_POSITION, ITEM_TYPE::QUEST_TWO, statuses[1]);
    createQuestItemIndicator(QUEST_3_INDICATOR_POSITION, ITEM_TYPE::QUEST_THREE, statuses[2]);
    createQuestItemIndicator(QUEST_4_INDICATOR_POSITION, ITEM_TYPE::QUEST_FOUR, statuses[3]);

    if (statuses[0] == QUEST_ITEM_STATUS::SUBMITTED) createSpaceshipPart(ITEM_TYPE::QUEST_ONE);
    if (statuses[1] == QUEST_ITEM_STATUS::SUBMITTED) createSpaceshipPart(ITEM_TYPE::QUEST_TWO);
    if (statuses[2] == QUEST_ITEM_STATUS::SUBMITTED) createSpaceshipPart(ITEM_TYPE::QUEST_THREE);
    if (statuses[3] == QUEST_ITEM_STATUS::SUBMITTED) createSpaceshipPart(ITEM_TYPE::QUEST_FOUR);
};

void QuestSystem::processQuestItem(ITEM_TYPE type, QUEST_ITEM_STATUS new_status) {
    // Remove current indicator
    for (uint i = 0; i < registry.questItemIndicators.size(); i++) {
        Entity e = registry.questItemIndicators.entities[i];
        QuestItemIndicator& c = registry.questItemIndicators.components[i];

        if (c.quest_item == type) {
            registry.remove_all_components_of(e);
        }
    }

    // Create new indicator
    vec2 position;
    switch (type) {
    case ITEM_TYPE::QUEST_ONE:
        position = QUEST_1_INDICATOR_POSITION;
        break;
    case ITEM_TYPE::QUEST_TWO:
        position = QUEST_2_INDICATOR_POSITION;
        break;
    case ITEM_TYPE::QUEST_THREE:
        position = QUEST_3_INDICATOR_POSITION;
        break;
    case ITEM_TYPE::QUEST_FOUR:
        position = QUEST_4_INDICATOR_POSITION;
        break;

    }
    createQuestItemIndicator(position, type, new_status);


    // Update quest item status
    Entity player = registry.players.entities[0];
    Inventory& inventory = registry.inventories.get(player);
    inventory.quest_items[type] = new_status;
};

bool QuestSystem::submitQuestItems() {
    Entity player = registry.players.entities[0];
    Inventory& inventory = registry.inventories.get(player);
    
    int num_submitted = 0;
    for (auto it = inventory.quest_items.begin(); it != inventory.quest_items.end(); it++) {
        if (it->second != QUEST_ITEM_STATUS::NOT_FOUND) {
            num_submitted++;

            if (it->second == QUEST_ITEM_STATUS::FOUND) {
                processQuestItem(it->first, QUEST_ITEM_STATUS::SUBMITTED);
                createSpaceshipPart(it->first);
            }
        }
    }

    return num_submitted == NUM_QUEST_ITEMS;
};

Entity QuestSystem::createQuestItemIndicator(vec2 position, ITEM_TYPE type, QUEST_ITEM_STATUS status) {
    auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
    motion.scale = vec2({ target_resolution.x / tile_size_px * 0.08333333, target_resolution.y / tile_size_px * 0.125 });


    // Add entity to quest item indicator registry
    auto& questItemIndicator = registry.questItemIndicators.emplace(entity);
    questItemIndicator.quest_item = type;

    // Add entity to screen UI registry
    registry.screenUI.insert(entity, position);

    TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND;
    switch(status) {
        case QUEST_ITEM_STATUS::NOT_FOUND:
            texture = quest_item_indicator_not_found_textures_map[type];
            break;
        case QUEST_ITEM_STATUS::FOUND:
            texture = quest_item_indicator_found_textures_map[type];
            break;
        case QUEST_ITEM_STATUS::SUBMITTED:
            texture = quest_item_indicator_submitted_textures_map[type];
            break;
    }

	registry.renderRequests.insert(
		entity,
		{ 
            texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 
        }
    );

	return entity;
};

Entity QuestSystem::createSpaceshipPart(ITEM_TYPE type) {
    auto entity = Entity();

    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };
    motion.position = { 0, -2.5 };
    motion.scale = vec2({ target_resolution.x / tile_size_px * 0.20833333, target_resolution.y / tile_size_px * 0.3125 });

    // Add spaceship to the spaceship_parts registry
    SpaceshipParts& spaceship_parts = registry.spaceshipParts.emplace(entity);

    TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::PLAYER;
    switch (type) {
    case ITEM_TYPE::QUEST_ONE:
        texture = TEXTURE_ASSET_ID::QUEST_1_BUILT;
        break;
    case ITEM_TYPE::QUEST_TWO:
        texture = TEXTURE_ASSET_ID::QUEST_2_BUILT;
        break;
    case ITEM_TYPE::QUEST_THREE:
        texture = TEXTURE_ASSET_ID::QUEST_3_BUILT;
        break;
    case ITEM_TYPE::QUEST_FOUR:
        texture = TEXTURE_ASSET_ID::QUEST_4_BUILT;
        break;
    }
    registry.renderRequests.insert(
        entity,
        {
            texture,
            EFFECT_ASSET_ID::TEXTURED,
            GEOMETRY_BUFFER_ID::SPRITE,
            RENDER_LAYER_ID::LAYER_1
        }
    );

    return entity;
};