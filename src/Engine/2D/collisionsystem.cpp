#include "collisionsystem.h"

namespace pg
{
    namespace
    {
        constexpr const char * const DOM = "Collision System";
    }

    void CollisionComponent::onDeletion(EntityRef)
    {
        LOG_THIS_MEMBER("Collision Component");

        auto sys = ecsRef->getSystem<CollisionSystem>();

        sys->removeComponentFromGrid(this);        
    }

    CollisionSystem::CollisionSystem() : pageSize(5, 5), cellSize(5, 5)
    {
        LOG_THIS_MEMBER(DOM);
    }

    void CollisionSystem::init()
    {
        LOG_THIS_MEMBER(DOM);

        auto group = registerGroup<UiComponent, CollisionComponent>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO(DOM, "Add entity " << entity->id << " to ui - collision group !");

            auto ui = entity->get<UiComponent>();
            auto collision = entity->get<CollisionComponent>();

            auto sys = entity->world()->getSystem<CollisionSystem>();

            sys->addComponentInGrid(ui, collision);
        });

        // group->removeOfGroup([](EntitySystem* ecsRef, _unique_id id) {
        //     LOG_INFO(DOM, "Remove entity " << id << " of ui - collision group !");

        //     auto sys = ecsRef->getSystem<CollisionComponent>();

        //     sys->removeElement(id);
        // });
    }

    void CollisionSystem::addComponentInGrid(UiComponent* pos, CollisionComponent* comp)
    {
        LOG_THIS_MEMBER(DOM);

        if (pos->height == 0 or pos->width == 0)
        {
            LOG_INFO(DOM, "Object has no area so no collision needed");
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

                it->second.addId(*comp, {startX, startY}, {remainingWidth, remainingHeight});

                usedWidth = ((xPagePos + 1) * pWidth) - startX;

                xPagePos++;

                startX = xPagePos * pWidth;
            }

            xPagePos = startingXPos;

            usedHeight = ((yPagePos + 1) * pHeight) - startY;

            yPagePos++;

            startY = yPagePos * pHeight;
        }

        resolveCollisionList(pos, comp);
    }

    void CollisionSystem::removeComponentFromGrid(CollisionComponent* comp)
    {
        LOG_THIS_MEMBER(DOM);

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

    std::set<_unique_id> CollisionSystem::resolveCollisionList(UiComponent* pos, CollisionComponent* comp)
    {
        LOG_THIS_MEMBER(DOM);

        std::set<_unique_id> touchedIds;

        // Get all ids in the same cell as our comp
        for (auto cell : comp->cells)
        {
            touchedIds.insert(cell->ids.begin(), cell->ids.end());
        }

        // We remove our id as we don't want to test the collision on ourselves
        touchedIds.erase(comp->entityId);

        std::set<_unique_id> collidedIds;

        for (auto id : touchedIds)
        {
            auto ent = ecsRef->getEntity(id);

            auto ui = ent->get<UiComponent>();

            if (ui)
            {
                // Todo have different type of collision (AABB to AABB, circle to AABB, etc)
                auto test = testCollision(pos, ui);

                if (test)
                {
                    detectedCollisions[comp->entityId].insert(id);
                    detectedCollisions[id].insert(comp->entityId);
                }
            }
        }

        return collidedIds;
    }

    // TODO this only test collision for AABB to AABB for now, need to expend on it !
    bool CollisionSystem::testCollision(UiComponent* obj1, UiComponent* obj2) const
    {
        LOG_THIS_MEMBER(DOM);

        auto obj1Xmin = obj1->pos.x;
        auto obj1XMax = obj1->pos.x + obj1->width;
        auto obj1YMin = obj1->pos.y;
        auto obj1YMax = obj1->pos.y + obj1->height;        

        auto obj2XMin = obj2->pos.x;
        auto obj2XMax = obj2->pos.x + obj2->width;
        auto obj2YMin = obj2->pos.y;
        auto obj2YMax = obj2->pos.y + obj2->height;        

        return obj1XMax > obj2XMin &&
               obj1Xmin < obj2XMax &&
               obj1YMax > obj2YMin &&
               obj1YMin < obj2YMax;
    }

     void CollisionSystem::onEvent(const UiComponentChangeEvent& event)
    {
        LOG_THIS_MEMBER(DOM);

        auto entity = ecsRef->getEntity(event.id);
        
        if(not entity or not entity->has<UiComponent>() or not entity->has<CollisionComponent>())
            return;

        auto ui = entity->get<UiComponent>();
        auto comp = entity->get<CollisionComponent>();

        removeComponentFromGrid(comp);
        addComponentInGrid(ui, comp);
    }

    void CollisionSystem::execute()
    {
        LOG_THIS_MEMBER(DOM);

        for (const auto& collision : detectedCollisions)
        {
            auto entity = ecsRef->getEntity(collision.first);

            if (not entity->has<CollisionComponent>())
                continue;

            auto comp = entity->get<CollisionComponent>();

            comp->collidedIds.clear();

            comp->collidedIds = collision.second;

            if (comp->callback)
                comp->callback->call(ecsRef); 
        }

        detectedCollisions.clear();
    }
}