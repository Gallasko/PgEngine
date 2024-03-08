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
            int endX = endPos.x / cellSize.x < (pos.x + 1) * size.x ? endPos.x / cellSize.x : (pos.x + 1) * size.x;
            int endY = endPos.y / cellSize.y < (pos.y + 1) * size.y ? endPos.y / cellSize.y : (pos.y + 1) * size.y;

            for (int y = startPos.y / cellSize.y; y < endY; y++)
            {
                for (int x = startPos.x / cellSize.x; x < endX; x++)
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
        // Todo make a ctor that load properties (pageSize, cellSi) from serialization
        CollisionSystem();

        virtual void init() override;

        void addComponentInGrid(UiComponent* pos, CollisionComponent* comp)
        {
            if (pos->height  == 0 or pos->width == 0)
            {
                LOG_INFO("Collision System", "Object has no area so no collision needed");
                return;
            }

            auto pWidth = (pageSize.x * cellSize.x);
            auto pHeight = (pageSize.y * cellSize.y);

            int xPagePos = pos->pos.x / pWidth;
            int yPagePos = pos->pos.y / pHeight;

            int startY = pos->pos.y - yPagePos * pHeight;

            int usedWidth = 0, usedHeight = 0;

            int startingXPos = xPagePos;

            for (int remainingHeight = pos->height; remainingHeight > 0; remainingHeight -= usedHeight)
            {
                int startX = pos->pos.x - xPagePos * pWidth;

                for (int remainingWidth = pos->width; remainingWidth > 0; remainingWidth -= usedWidth)
                {
                    auto key = Strfy() << xPagePos << "_" << yPagePos;
                    
                    auto keyStr = key.getData();

                    auto it = loadedPages.find(keyStr);
                    if (it == loadedPages.end())
                    {
                        it = loadedPages.emplace(keyStr, CollisionPage{{xPagePos, yPagePos}, pageSize, cellSize}).first;
                    }

                    it->second.addId(*comp, {startX, startY}, {startX + remainingWidth, startY + remainingHeight});

                    usedWidth = ((xPagePos + 1) * pWidth) - startX;

                    xPagePos++;

                    startX = xPagePos * pWidth;
                }

                xPagePos = startingXPos;

                usedHeight = ((yPagePos + 1) * pHeight) - startY;

                yPagePos++;

                startY = yPagePos * pHeight;
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
                if (loadedPages.at(page).isEmpty())
                    loadedPages.erase(page);
            }

            comp->cells.clear();
        }

        constant::Vector2D pageSize;
        constant::Vector2D cellSize;

        std::unordered_map<std::string, CollisionPage> loadedPages;
    };
}