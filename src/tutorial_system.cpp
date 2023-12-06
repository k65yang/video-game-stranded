#pragma once

#include "tutorial_system.hpp"

void TutorialSystem::step(float elapsed_ms) {
    // Count down timers for tutorials
    for (Entity entity : registry.tutorials.entities) {
        Tutorial& tutorial = registry.tutorials.get(entity);

        tutorial.timer_ms -= elapsed_ms;

        // Remove tutorial if timer expired
        if (tutorial.timer_ms < 0) {
            registry.remove_all_components_of(entity);
        }
    }
};

void TutorialSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;
};

void TutorialSystem::resetTutorialSystem() {
    is_help_dialog_open = false;
    createHelpButton();
};

void TutorialSystem::openHelpDialog() {
    help_dialog = createHelpDialog();
    is_help_dialog_open = true;
};

void TutorialSystem::closeHelpDialog() {

};

bool TutorialSystem::isHelpDialogOpen() {
    return false;
};

bool TutorialSystem::isMouseOverElement(vec2 mouse_pos, TEXTURE_ASSET_ID element) {
    if (element == TEXTURE_ASSET_ID::HELP_BUTTON) {
        // Help button is a circle so check if the distance between the mouse and button is less than the radius of the button
        vec2 element_pos = HELP_BUTTON_POSITION;
        float radius = HELP_BUTTON_SCALE.x / 2;
        float dist = distance(mouse_pos, element_pos);

        return dist <= radius;
    }

    return false;
};

Entity TutorialSystem::createTutorialText(TUTORIAL_TYPE type) {
    vec2 position;
    std::string str;
    switch(type) {
        case TUTORIAL_TYPE::QUEST_ITEM_TUTORIAL:
            position = QUEST_ITEM_TUTORIAL_TEXT_POSITION;
            str = QUEST_ITEM_TUTORIAL_TEXT;
            break;
        case TUTORIAL_TYPE::SPACESHIP_HOME_TUTORIAL:
            position = SPACESHIP_HOME_TUTORIAL_TEXT_POSITION;
            str = SPACESHIP_HOME_TUTORIAL_TEXT;
            break;
        case TUTORIAL_TYPE::GAME_SAVED:
            position = GAME_SAVED_TEXT_POSITION;
            str = GAME_SAVED_TEXT;
            break;
        case TUTORIAL_TYPE::GAME_LOADED:
            position = GAME_LOADED_TEXT_POSITION;
            str = GAME_LOADED_TEXT;
            break;
    }

    // Remove any tutorials that are currently being displayed
    for (Entity entity : registry.tutorials.entities) {
        registry.remove_all_components_of(entity);
    }

    // Create tutorial text
    Entity tutorial_text = createText(renderer, position, str, TUTORIAL_TEXT_SCALE);
    registry.tutorials.emplace(tutorial_text);

    return tutorial_text;
};

Entity TutorialSystem::createHelpButton() {
    auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = HELP_BUTTON_POSITION;
    motion.scale = HELP_BUTTON_SCALE;

    // Add entity to screen UI registry
    registry.screenUI.insert(entity, HELP_BUTTON_POSITION);
    
    registry.renderRequests.insert(
		entity,
		{ 
            TEXTURE_ASSET_ID::HELP_BUTTON,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 
        }
    );

	return entity;
};

Entity TutorialSystem::createHelpDialog() {
    auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = HELP_DIALOG_POSITION;
    motion.scale = HELP_DIALOG_SCALE;

    // Add entity to screen UI registry
    registry.screenUI.insert(entity, HELP_DIALOG_POSITION);
    
    registry.renderRequests.insert(
		entity,
		{ 
            TEXTURE_ASSET_ID::HELP_DIALOG,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_4 
        }
    );

	return entity;
};
