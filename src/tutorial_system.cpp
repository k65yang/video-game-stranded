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
    std::string str;
    switch(type) {
        case TUTORIAL_TYPE::QUEST_ITEM_TUTORIAL:
            str = QUEST_ITEM_TUTORIAL_TEXT;
            break;
        case TUTORIAL_TYPE::SPACESHIP_HOME_TUTORIAL:
            str = SPACESHIP_HOME_TUTORIAL_TEXT;
            break;
    }

    Entity tutorial_text = createText(renderer, TUTORIAL_TEXT_POSITION, str, TUTORIAL_TEXT_SCALE);
    registry.tutorials.emplace(tutorial_text);

    return tutorial_text;
};
