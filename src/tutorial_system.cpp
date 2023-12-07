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

    // Show/hide enter spaceship text
    Entity player = registry.players.entities[0];
    Entity spaceship = registry.spaceships.entities[0];
    Motion& player_motion = registry.motions.get(player);
    Player& player_info = registry.players.get(player);
    Motion& spaceship_motion = registry.motions.get(spaceship);
    
    if (isPlayerNearSpaceship(player_motion.position, spaceship_motion.position) && !player_info.is_home && !isEnterSpaceshipTextShown()) {
        showEnterSpaceshipText();
    } else if (!isPlayerNearSpaceship(player_motion.position, spaceship_motion.position) && isEnterSpaceshipTextShown() || player_info.is_home) {
        hideEnterSpaceshipText();
    }
};

void TutorialSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;
};

void TutorialSystem::resetTutorialSystem() {
    is_help_dialog_open = false;
    is_enter_spaceship_text_shown = false;

    help_button = createHelpButton();
};

void TutorialSystem::openHelpDialog() {
    help_dialog = createHelpDialog();
    is_help_dialog_open = true;
};

void TutorialSystem::closeHelpDialog() {
    registry.remove_all_components_of(help_dialog);
    is_help_dialog_open = false;
};

bool TutorialSystem::isHelpDialogOpen() {
    return is_help_dialog_open;
};

bool TutorialSystem::isMouseOverHelpButton(vec2 mouse_pos) {
    // Help button is a circle so check if the distance between the mouse and button is less than the radius of the button
    Motion& motion = registry.motions.get(help_button);
    float radius = HELP_BUTTON_SCALE.x / 2;
    float dist = distance(mouse_pos, motion.position);

    return dist <= radius;
};

Entity TutorialSystem::createTutorialText(TUTORIAL_TYPE type) {
    removeDisplayedTutorials();

    vec2 position;
    std::string str;
    switch(type) {
        case TUTORIAL_TYPE::GAME_SAVED:
            position = GAME_SAVED_TEXT_POSITION;
            str = GAME_SAVED_TEXT;
            break;
        case TUTORIAL_TYPE::GAME_LOADED:
            position = GAME_LOADED_TEXT_POSITION;
            str = GAME_LOADED_TEXT;
            break;
    }

    Entity tutorial_text = createText(renderer, position, str, TUTORIAL_TEXT_SCALE);

    // Add entity to tutorial registry
    registry.tutorials.emplace(tutorial_text);

    return tutorial_text;
};

Entity TutorialSystem::createTutorialDialog(TUTORIAL_TYPE type) {
    removeDisplayedTutorials();

    auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = TUTORIAL_DIALOG_POSITION;
    motion.scale = TUTORIAL_DIALOG_SCALE;

    // Add entity to screen UI registry
    registry.screenUI.insert(entity, TUTORIAL_DIALOG_POSITION);

    // Add entity to tutorial registry
    registry.tutorials.emplace(entity);

    TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::QUEST_ITEM_TUTORIAL_DIALOG;
    switch(type) {
        case TUTORIAL_TYPE::QUEST_ITEM_TUTORIAL:
            texture = TEXTURE_ASSET_ID::QUEST_ITEM_TUTORIAL_DIALOG;
            break;
        case TUTORIAL_TYPE::SPACESHIP_HOME_TUTORIAL:
            texture = TEXTURE_ASSET_ID::SPACESHIP_HOME_TUTORIAL_DIALOG;
            break;
    }
    
    registry.renderRequests.insert(
		entity,
		{ 
            texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER_ID::LAYER_5 
        }
    );

	return entity;
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
			RENDER_LAYER_ID::LAYER_5
        }
    );

	return entity;
};

void TutorialSystem::showEnterSpaceshipText() {
    Entity spaceship = registry.spaceships.entities[0];
    Motion& spaceship_motion = registry.motions.get(spaceship);
    const vec2 ENTER_SPACESHIP_TEXT_POSITION = { spaceship_motion.position.x - 2.0f, spaceship_motion.position.y - 2.0f };

    enter_spaceship_text = createText(renderer, ENTER_SPACESHIP_TEXT_POSITION, ENTER_SPACESHIP_TEXT, TUTORIAL_TEXT_SCALE);
    registry.screenUI.remove(enter_spaceship_text);    // Remove from screen ui registry so text does not move with camera

    is_enter_spaceship_text_shown = true;
};

void TutorialSystem::hideEnterSpaceshipText() {
    registry.remove_all_components_of(enter_spaceship_text);
    is_enter_spaceship_text_shown = false;
};

bool TutorialSystem::isEnterSpaceshipTextShown() {
    return is_enter_spaceship_text_shown;
};

bool TutorialSystem::isPlayerNearSpaceship(vec2 player_pos, vec2 spaceship_pos) {
    return distance(player_pos, spaceship_pos) < 2.f;
};

void TutorialSystem::removeDisplayedTutorials() {
    for (Entity entity : registry.tutorials.entities) {
        registry.remove_all_components_of(entity);
    }
};
