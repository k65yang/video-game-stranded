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
        /// @param status Status for quest items
        void resetQuestSystem(std::vector<QUEST_ITEM_STATUS> status);

        /// @brief Marks a quest item as found and updates its indicator
        /// @param type The quest item that was found
        void foundQuestItem(ITEM_TYPE type);

        /// @brief Marks quest item(s) as submitted and updates quest item indicator(s)
        void submitQuestItems();

    private:
        RenderSystem* renderer;

        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> quest_item_indicator_not_found_textures_map {
            {ITEM_TYPE::QUEST_ONE, TEXTURE_ASSET_ID::QUEST_1_NOT_FOUND},
            {ITEM_TYPE::QUEST_TWO, TEXTURE_ASSET_ID::QUEST_2_NOT_FOUND}
        };
        
        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> quest_item_indicator_found_textures_map {
            {ITEM_TYPE::QUEST_ONE, TEXTURE_ASSET_ID::QUEST_1_FOUND},
            {ITEM_TYPE::QUEST_TWO, TEXTURE_ASSET_ID::QUEST_2_FOUND}
        };

        std::map<ITEM_TYPE, TEXTURE_ASSET_ID> quest_item_indicator_submitted_textures_map {
            {ITEM_TYPE::QUEST_ONE, TEXTURE_ASSET_ID::QUEST_1_SUBMITTED},
            {ITEM_TYPE::QUEST_TWO, TEXTURE_ASSET_ID::QUEST_2_SUBMITTED}
        };

        /// @brief Creates a quest item indicator
        /// @param position The position of the indicator
        /// @param type The quest item the indicator is for
        /// @param texture The texture to use for the indicator
        Entity createQuestItemIndicator(vec2 position, ITEM_TYPE type, TEXTURE_ASSET_ID texture);

        /// @brief Marks a quest item as submitted and updates its indicator
        /// @param type The quest item that was submitted
        void submitQuestItem(ITEM_TYPE type);
};
