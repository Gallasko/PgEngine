#include "commanddispatcher.h"

#include "entity.h"
#include "entitysystem.h"

namespace pg
{
    namespace
    {
        constexpr const char * const DOM = "Command Dispatcher";
    }

    EntityRef CommandDispatcher::createEntity()
    {
        LOG_THIS_MEMBER(DOM);

        auto entity = new Entity(ecsRef->registry.idGenerator.generateId(), ecsRef);

        if(not entityQueue.enqueue(EntityCommand{entity, EntityCommand::EntityCommandType::creation}))
        {
            LOG_ERROR(DOM, "Could not enqueue the creation of entity " << entity->id);
            return nullptr;
        }

        return {entity, false};
    }

    void CommandDispatcher::deleteEntity(Entity* entity)
    {
        LOG_THIS_MEMBER(DOM);

        if(not entityQueue.enqueue(EntityCommand{entity, EntityCommand::EntityCommandType::deletion}))
        {
            LOG_ERROR(DOM, "Could not enqueue the deletion of entity " << entity->id);
        }
    }

    void CommandDispatcher::process()
    {
        LOG_THIS_MEMBER(DOM);

        EntityCommand item(nullptr, EntityCommand::EntityCommandType::creation);

        ComponentCommand item2;

        bool found = componentQueue.try_dequeue(item2);

        while (found or entityQueue.try_dequeue(item))
        {
            while (entityQueue.try_dequeue(item))
            {
                if(item.type == EntityCommand::EntityCommandType::creation)
                {
                    ecsRef->addEntityToPool(item.entity);
                    delete item.entity;
                }
                else if(item.type == EntityCommand::EntityCommandType::deletion)
                {
                    ecsRef->deleteEntityFromPool(item.entity);
                }
            }

            if(found or item2.type == ComponentCommand::ComponentCommandType::creation)
            {
                item2.addInEcs(ecsRef, item2.entity, item2.component);
                item2.deleteComp(item2.component);
            }

            found = componentQueue.try_dequeue(item2);
        }
    }
}