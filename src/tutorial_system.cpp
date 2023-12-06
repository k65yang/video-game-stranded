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
