#pragma once

#include <map>
#include <vector>

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"

class QuestSystem
{
    public:
        /// @brief QuestSystem constructor
        QuestSystem()
        {
        }

        /// @brief Function to update quest system in world time
        /// @param elapsed_ms How many ms elapsed since last update
        void step(float elapsed_ms);

        /// @brief Initializes the render system that the quest system uses
        /// @param renderer_arg Pointer to the render system
        void init(RenderSystem* renderer_arg);

        /// @brief Resets the quest system
        /// @param statuses Statuses for quest items
        void resetQuestSystem(std::vector<QUEST_ITEM_STATUS> statuses);

        /// @brief Marks a quest item as found or submitted depending on new_status and updates its indicator
        /// @param type The quest item that was found
        /// @param new_status The new status of the quest item
        void processQuestItem(ITEM_TYPE type, QUEST_ITEM_STATUS new_status);

        /// @brief Marks quest item(s) as submitted and updates quest item indicator(s)
        /// @returns True if all quest items are submitted, false otherwise
        bool submitQuestItems();

    private:
        const int NUM_QUEST_ITEMS = 4;
        const vec2 QUEST_1_INDICATOR_POSITION = { 10.f, -4.f};
        const vec2 QUEST_2_INDICATOR_POSITION = { 10.f, -1.5f};
        const vec2 QUEST_3_INDICATOR_POSITION = { 10.f, 1.0f };
        const vec2 QUEST_4_INDICATOR_POSITION = { 10.f, 3.5f };

        RenderSystem* renderer;

        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> quest_item_indicator_not_found_textures_map {
            {ITEM_TYPE::QUEST_ONE, TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND},
            {ITEM_TYPE::QUEST_TWO, TEXTURE_ASSET_ID::QUEST_2_NOT_FOUND},
            {ITEM_TYPE::QUEST_THREE, TEXTURE_ASSET_ID::QUEST_3_NOT_FOUND},
            {ITEM_TYPE::QUEST_FOUR, TEXTURE_ASSET_ID::QUEST_4_NOT_FOUND}
        };
        
        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> quest_item_indicator_found_textures_map {
            {ITEM_TYPE::QUEST_ONE, TEXTURE_ASSET_ID::QUEST_1_FOUND},
            {ITEM_TYPE::QUEST_TWO, TEXTURE_ASSET_ID::QUEST_2_FOUND},
            {ITEM_TYPE::QUEST_THREE, TEXTURE_ASSET_ID::QUEST_3_FOUND},
            {ITEM_TYPE::QUEST_FOUR, TEXTURE_ASSET_ID::QUEST_4_FOUND}
        };

        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> quest_item_indicator_submitted_textures_map {
            {ITEM_TYPE::QUEST_ONE, TEXTURE_ASSET_ID::QUEST_1_SUBMITTED},
            {ITEM_TYPE::QUEST_TWO, TEXTURE_ASSET_ID::QUEST_2_SUBMITTED},
            {ITEM_TYPE::QUEST_THREE, TEXTURE_ASSET_ID::QUEST_3_SUBMITTED},
            {ITEM_TYPE::QUEST_FOUR, TEXTURE_ASSET_ID::QUEST_4_SUBMITTED}
        };

        /// @brief Creates a quest item indicator
        /// @param position The position of the indicator
        /// @param type The quest item the indicator is for
        /// @param status The status of the quest item
        Entity createQuestItemIndicator(vec2 position, ITEM_TYPE type, QUEST_ITEM_STATUS status);

        /// @brief Creates a quest item build onto spaceship
        /// @param type The quest item the spaceship part is for 
        Entity createSpaceshipPart(ITEM_TYPE type);

};
