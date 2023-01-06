#pragma once

#include "concurrentqueue.h"

namespace pg
{
    // Type forwarding
    class EntitySystem;

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
     * creating or deleting an Entity or launching some systems.
     * 
     * It differs from the Event System as this system communicates directly with the ECS root engine whereas
     * the event system allows communication between systems.
     * 
     * @see EventDispatcher
     */
    class CommandDispatcher
    {
    public:
        typedef moodycamel::ConcurrentQueue<SysCommand>::producer_token_t CommandToken;

    public:
        CommandDispatcher(EntitySystem *ecs) : ecsRef(ecs) {}

        inline bool enqueueCommand(const SysCommand& cmd)
        {
            sysQueue.enqueue(cmd);
        }

        inline bool enqueueCommand(SysCommand&& cmd)
        {
            sysQueue.enqueue(cmd);
        }

        inline bool enqueueCommand(const CommandToken& token, const SysCommand& cmd)
        {
            sysQueue.enqueue(token, cmd);
        }

        void process();

    private:
        const EntitySystem* ecsRef;

        moodycamel::ConcurrentQueue<SysCommand> sysQueue;
    };
}