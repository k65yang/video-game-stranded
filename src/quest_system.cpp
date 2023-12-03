#include "quest_system.hpp"

void QuestSystem::step(float elapsed_ms) {

};

void QuestSystem::init(RenderSystem* render_system_arg) {

};

void QuestSystem::resetQuestSystem(std::vector<QUEST_ITEM_STATUS> status) {

};

void QuestSystem::foundQuestItem(ITEM_TYPE type) {

};

void QuestSystem::submitQuestItems() {

};

Entity QuestSystem::createQuestItemIndicator(vec2 position, ITEM_TYPE type, TEXTURE_ASSET_ID texture) {
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
