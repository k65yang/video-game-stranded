#include "quest_system.hpp"

void QuestSystem::step(float elapsed_ms) {

};

void QuestSystem::init(RenderSystem* render_system_arg) {

};

void QuestSystem::resetQuestSystem(std::vector<QUEST_ITEM_STATUS> status) {
    Entity player = registry.players.entities[0];
    
    // Set quest item status
    Inventory& inventory = registry.inventories.get(player);
    inventory.quest_items[ITEM_TYPE::QUEST_ONE] = status[0];
    inventory.quest_items[ITEM_TYPE::QUEST_TWO] = status[1];

    // Create quest item indicators
    createQuestItemIndicator(QUEST_1_INDICATOR_POSITION, ITEM_TYPE::QUEST_ONE, status[0]);
    createQuestItemIndicator(QUEST_2_INDICATOR_POSITION, ITEM_TYPE::QUEST_TWO, status[1]);
};

void QuestSystem::foundQuestItem(ITEM_TYPE type) {

};

void QuestSystem::submitQuestItems() {

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
	motion.scale = vec2({ 3.f, 3.f });

    // Add entity to quest item indicator registry
    auto& questItemIndicator = registry.questItemIndicators.emplace(entity);
    questItemIndicator.quest_item = type;

    // Add entity to screen UI registry
    registry.screenUI.insert(entity, position);

    TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND;
    if (type == ITEM_TYPE::QUEST_ONE) {
        if (status == QUEST_ITEM_STATUS::NOT_FOUND) texture = TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND;
        if (status == QUEST_ITEM_STATUS::FOUND)     texture = TEXTURE_ASSET_ID::QUEST_1_FOUND;
        if (status == QUEST_ITEM_STATUS::SUBMITTED) texture = TEXTURE_ASSET_ID::QUEST_1_SUBMITTED;
    } else if (type == ITEM_TYPE::QUEST_TWO) {
        if (status == QUEST_ITEM_STATUS::NOT_FOUND) texture = TEXTURE_ASSET_ID::QUEST_2_NOT_FOUND;
        if (status == QUEST_ITEM_STATUS::FOUND)     texture = TEXTURE_ASSET_ID::QUEST_2_FOUND;
        if (status == QUEST_ITEM_STATUS::SUBMITTED) texture = TEXTURE_ASSET_ID::QUEST_2_SUBMITTED;
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

void QuestSystem::submitQuestItem(ITEM_TYPE type) {

};
