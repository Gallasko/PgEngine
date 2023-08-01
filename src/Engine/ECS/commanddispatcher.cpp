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

        std::lock_guard<std::mutex> lock(mutex);

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

        std::lock_guard<std::mutex> lock(mutex);

        if(not entityQueue.enqueue(EntityCommand{entity, EntityCommand::EntityCommandType::deletion}))
        {
            LOG_ERROR(DOM, "Could not enqueue the deletion of entity " << entity->id);
        }
    }

    void CommandDispatcher::process()
    {
        LOG_THIS_MEMBER(DOM);

        std::lock_guard<std::mutex> lock(mutex);

        EntityCommand item(nullptr, EntityCommand::EntityCommandType::creation);

        ComponentCommand item2;

        bool found = entityQueue.try_dequeue(item);

        bool found2 = componentQueue.try_dequeue(item2);

        while (found or found2)
        {
            while (found)
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

                found = entityQueue.try_dequeue(item);
            }

            if(found2 and item2.type == ComponentCommand::ComponentCommandType::creation)
            {
                item2.addInEcs(ecsRef, item2.entity, item2.component);
                item2.deleteComp(item2.component);
            }

            found2 = componentQueue.try_dequeue(item2);
        }
    }
}