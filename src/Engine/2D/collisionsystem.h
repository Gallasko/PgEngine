#pragma once

#include "ECS/system.h"

#include "UI/uisystem.h"

#include "constant.h"

namespace pg
{
    struct CollisionCell
    {
        CollisionCell(const constant::Vector2D& pos, const constant::Vector2D& size) : pos(pos), size(size) {}
        
        /** Position of the cell */
        const constant::Vector2D pos;
        /** Size of the cell */
        const constant::Vector2D size;

        std::set<_unique_id> ids;
    };

    struct CollisionPage
    {
        CollisionPage(const constant::Vector2D& pos, const constant::Vector2D& size, const constant::Vector2D& cellSize) : pos(pos), size(size), cellSize(cellSize)
        {
            for (auto x = 0; x < size.x; x++)
            {
                for (auto y = 0; y < size.y; y++)
                {
                    cells.emplace_back(constant::Vector2D{pos.x + x, pos.y + y}, cellSize);
                }
            }
        }

        void addId(_unique_id id, const constant::Vector2D& startPos, const constant::Vector2D& endPos)
        {
            auto endX = endPos.x < (pos.x + 1) * size.x ? endPos.x : (pos.x + 1) * size.x;
            auto endY = endPos.y < (pos.y + 1) * size.y ? endPos.y : (pos.y + 1) * size.y;

            for (auto y = startPos.y; y < endY; y++)
            {
                for (auto x = startPos.x; x < endX; x++)
                {
                    cells[x + y * size.x].ids.insert(id);
                }
            }
        }

        void removeId(_unique_id id, const constant::Vector2D& startPos, const constant::Vector2D& endPos)
        {
            auto endX = endPos.x < (pos.x + 1) * size.x ? endPos.x : (pos.x + 1) * size.x;
            auto endY = endPos.y < (pos.y + 1) * size.y ? endPos.y : (pos.y + 1) * size.y;

            for (auto y = startPos.y; y < endY; y++)
            {
                for (auto x = startPos.x; x < endX; x++)
                {
                    cells[x + y * size.x].ids.erase(id);
                }
            }
        }
        
        /** Position of the page in the grid */
        const constant::Vector2D pos;
        
        /** Size of the page (width = number of cell per row, height = number of rows)
            Total absolute size of page is width = this->size.x * cell.size.x, same for height */ 
        const constant::Vector2D size;

        /** Absolute size of a single cell */
        const constant::Vector2D cellSize;
        
        std::vector<CollisionCell> cells;
    };

    struct CollisionComponent : public Ctor
    {
        virtual void onCreation(EntityRef entity) override
        {
            ecsRef = entity->world();

            entityId = entity->id;
        }

        EntitySystem* ecsRef = nullptr;

        _unique_id entityId = 0;

        std::vector<CollisionCell*> cells;
    };

    struct CollisionSystem : public System<Own<CollisionComponent>, Ref<UiPosition>, InitSys>
    {

        std::unordered_map<std::string, CollisionPage> loadedPages;
    };
}