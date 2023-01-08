#pragma once

#include "concurrentqueue.h"

#include "logger.h"

namespace pg
{
    // Type forwarding
    class EntitySystem;
    class Entity;

    struct SysCommand
    {
        enum class SysCommandType
        {
            createEntity,
            deleteEntity,
            startSystem,
            stopSystem,
            log
        };

        SysCommandType type;
    };

    /**
     * @brief A base ECS system responsible of system commands generated from other systems.
     * 
     * This system is responsible for creating and running ECS system commands
     * Should be used for every commands that need to be executed asynchronously such as
     * creating or deleting an Entity or a component.
     */
    class CommandDispatcher
    {
    public:
        typedef moodycamel::ConcurrentQueue<SysCommand>::producer_token_t CommandToken;

        struct EntityCommand
        {
            enum class EntityCommandType
            {
                creation = 0,
                deletion = 1
            };

            EntityCommand(Entity *entity, const EntityCommandType& type) : entity(entity), type(type) {}

            Entity *entity;
            EntityCommandType type;
        };

    public:
        CommandDispatcher(EntitySystem *ecs) : ecsRef(ecs) {}

        Entity* createEntity();

        void deleteEntity(Entity* entity);

        inline bool enqueueCommand(const SysCommand& cmd)
        {
            return sysQueue.enqueue(cmd);
        }

        inline bool enqueueCommand(SysCommand&& cmd)
        {
            return sysQueue.enqueue(cmd);
        }

        inline bool enqueueCommand(const CommandToken& token, const SysCommand& cmd)
        {
            return sysQueue.enqueue(token, cmd);
        }

        void process();

    private:
        EntitySystem *const ecsRef;

        moodycamel::ConcurrentQueue<EntityCommand> entityQueue;

        moodycamel::ConcurrentQueue<SysCommand> sysQueue;
    };
}