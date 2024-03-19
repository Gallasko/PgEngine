#pragma once

#include "ECS/system.h"
#include "ECS/callable.h"

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

    struct CollisionComponent : public Ctor, Dtor
    {
        CollisionComponent() {}
        CollisionComponent(CallablePtr callback) : callback(callback) {}

        virtual void onCreation(EntityRef entity) override
        {
            ecsRef = entity->world();

            entityId = entity->id;
        }

        virtual void onDeletion(EntityRef entity) override;

        CallablePtr callback = nullptr;

        std::set<_unique_id> collidedIds;

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

        void addId(CollisionComponent& comp, const constant::Vector2D& startPos, const constant::Vector2D& objSize)
        {
            int endX = objSize.x < cellSize.x * size.x ? (objSize.x / cellSize.x) + 1 : size.x;
            int endY = objSize.y < cellSize.y * size.y ? (objSize.y / cellSize.y) + 1 : size.y;

            for (int y = startPos.y / cellSize.y; y < endY; y++)
            {
                for (int x = startPos.x / cellSize.x - pos.x * size.x; x < endX; x++)
                {
                    cells[x + y * size.x].ids.insert(comp.entityId);
                    comp.cells.emplace_back(&cells[x + y * size.x]);
                }
            }
        }
        
        inline bool isEmpty() const
        {
            for (const auto& cell : cells)
            {
                if (not cell.ids.empty())
                {
                    return false;
                }
            }

            return true;
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

    struct CollisionSystem : public System<Own<CollisionComponent>, Ref<UiComponent>, Listener<UiComponentChangeEvent>, InitSys>
    {
        // Todo make a ctor that load properties (pageSize, cellSi) from serialization
        CollisionSystem();

        virtual void init() override;

        void addComponentInGrid(UiComponent* pos, CollisionComponent* comp);

        void removeComponentFromGrid(CollisionComponent* comp);

        std::set<_unique_id> resolveCollisionList(UiComponent* pos, CollisionComponent* comp);

        bool testCollision(UiComponent* obj1, UiComponent* obj2) const;

        virtual void onEvent(const UiComponentChangeEvent& event) override;

        virtual void execute() override;

        constant::Vector2D pageSize;
        constant::Vector2D cellSize;

        std::unordered_map<std::string, CollisionPage> loadedPages;

        std::map<_unique_id, std::set<_unique_id>> detectedCollisions;
    };
}