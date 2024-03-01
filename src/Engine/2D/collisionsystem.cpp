#include "collisionsystem.h"

namespace pg
{
    void CollisionSystem::init()
    {
        auto group = registerGroup<UiComponent, CollisionComponent>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Collision System", "Add entity " << entity->id << " to ui - collision group !");

            auto ui = entity->get<UiComponent>();
            auto collision = entity->get<CollisionComponent>();

            auto sys = entity->world()->getSystem<CollisionSystem>();

            sys->addComponentInGrid(ui, collision);
        });

        // group->removeOfGroup([](EntitySystem* ecsRef, _unique_id id) {
        //     LOG_INFO("Collision  System", "Remove entity " << id << " of ui - collision group !");

        //     auto sys = ecsRef->getSystem<CollisionComponent>();

        //     sys->removeElement(id);
        // });
    }
}