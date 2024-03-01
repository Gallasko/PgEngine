#pragma once

#include "ECS/system.h"

#include "UI/uisystem.h"

#include "constant.h"

namespace pg
{
    struct CollisionCell
    {
        CollisionCell(const constant::Vector2D& pagePos, const constant::Vector2D& pos, const constant::Vector2D& size) : pagePos(pagePos), pos(pos), size(size) {}

        /** Position of the associated page */
        const constant::Vector2D pagePos;        
        /** Position of the cell */
        const constant::Vector2D pos;
        /** Size of the cell */
        const constant::Vector2D size;

        std::set<_unique_id> ids;
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

    struct CollisionPage
    {
        CollisionPage(const constant::Vector2D& pos, const constant::Vector2D& size, const constant::Vector2D& cellSize) : pos(pos), size(size), cellSize(cellSize)
        {
            for (auto x = 0; x < size.x; x++)
            {
                for (auto y = 0; y < size.y; y++)
                {
                    cells.emplace_back(pos, constant::Vector2D{pos.x + x, pos.y + y}, cellSize);
                }
            }
        }

        void addId(CollisionComponent& comp, const constant::Vector2D& startPos, const constant::Vector2D& endPos)
        {
            auto endX = endPos.x < (pos.x + 1) * size.x ? endPos.x : (pos.x + 1) * size.x;
            auto endY = endPos.y < (pos.y + 1) * size.y ? endPos.y : (pos.y + 1) * size.y;

            for (auto y = startPos.y; y < endY; y++)
            {
                for (auto x = startPos.x; x < endX; x++)
                {
                    cells[x + y * size.x].ids.insert(comp.entityId);
                    comp.cells.emplace_back(&cells[x + y * size.x]);
                }
            }
        }
        
        inline bool isEmpty()
        {
            bool ret = true;

            for (auto& cell : cells)
            {
                if (not cell.ids.empty())
                {
                    ret = false;
                    break;
                }
            }

            return ret;
        }

        /** Position of the page in the grid */
        const constant::Vector2D pos;
        
        /** Size of the page (width = number of cell per row, height = number of rows)
            Total absolute size of page is width = this->size.x * cell.size.x, same for height */ 
        const constant::Vector2D size;

        /** Absolute size of a single cell */
        const constant::Vector2D cellSize;
        
        /** Array of cell managed by this page */
        std::vector<CollisionCell> cells;
    };

    struct CollisionSystem : public System<Own<CollisionComponent>, Ref<UiComponent>, InitSys>
    {
        virtual void init() override;

        void addComponentInGrid(UiComponent* pos, CollisionComponent* comp)
        {
            int xPagePos = pos->pos.x / (pageSize.x * cellSize.x);
            int yPagePos = pos->pos.y / (pageSize.y * cellSize.y);

            int normalizedX = pos->pos.x - xPagePos * (pageSize.x * cellSize.x);
            int normalizedY = pos->pos.y - yPagePos * (pageSize.y * cellSize.y);

            int startingXPos = xPagePos;

            for (int remainingHeight = pos->height; remainingHeight > 0; remainingHeight -= (pageSize.y * cellSize.y) - normalizedY)
            {
                for (int remainingWidth = pos->width; remainingWidth > 0; remainingWidth -= (pageSize.x * cellSize.x) - normalizedX)
                {
                    auto key = Strfy() << xPagePos << "_" << yPagePos;
                    loadedPages[key.getData()].addId(*comp, {normalizedX, normalizedY}, {normalizedX + remainingWidth, normalizedY + remainingHeight});

                    xPagePos++;

                    normalizedX = xPagePos * (pageSize.x * cellSize.x);
                }

                xPagePos = startingXPos;

                normalizedX = pos->pos.x - xPagePos * (pageSize.x * cellSize.x);

                yPagePos++;

                normalizedY = yPagePos * (pageSize.y * cellSize.y);
            }
        }

        void removeComponentFromGrid(CollisionComponent* comp)
        {
            std::set<std::string> affectedPages;

            for (auto cell : comp->cells)
            {
                auto key = Strfy() << cell->pagePos.x << "_" << cell->pagePos.y;
                affectedPages.insert(key.getData());

                cell->ids.erase(comp->entityId);
            }

            for (const auto& page : affectedPages)
            {
                if (loadedPages[page].isEmpty())
                    loadedPages.erase(page);
            }

            comp->cells.clear();
        }

        constant::Vector2D pageSize;
        constant::Vector2D cellSize;

        std::unordered_map<std::string, CollisionPage> loadedPages;
    };
}