#pragma once

#include "system.h"

namespace pg::ecs
{
    struct SysCommand : public Component
    {
        enum class SysCommandType
        {
            createEntity,
            deleteEntity,
            startSystem,
            stopSystem,
            log
        };

        SysCommand() : Component("SysCommand") {}
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
    class CommandDispatcher : public System<Own<SysCommand>>
    {

    };
}