/**
 * @file entitysystem.cpp
 * @author Pigeon Codeur (pigeoncodeur@gmail.com)
 * @brief Definition of the entity system
 * @version 0.1
 * @date 2022-08-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "entitysystem.h"

#include "system.h"

namespace
{
    static constexpr char const * DOM = "ECS";
}

namespace pg
{
    template<>
    void serialize(Archive& archive, const EntitySystem& ecs)
    {
        LOG_THIS(DOM);
    }

    // Todo set executor depending on the configuration / env !
    EntitySystem::EntitySystem() : registry(this), cmdDispatcher(this), executor(2)
    {
        LOG_THIS_MEMBER(DOM);

        // Add the command dispatcher as the first element of the task flow
        basicTask = taskflow.emplace([this](){ eventDispatcher.process(); cmdDispatcher.process(); }).name("Basic Task");
    }

    EntitySystem::~EntitySystem()
    {
        LOG_THIS_MEMBER(DOM);

        stop();

        for(auto& sys : systems)
        {
            delete sys.second;
        }
    }

    void EntitySystem::executeOnce()
    {
        LOG_THIS_MEMBER("ECS");

        if (running)
            return;

        running = true;

        executor.run(taskflow).wait();

        running = false;
    }

    void EntitySystem::executeAll()
    {
        LOG_THIS_MEMBER(DOM);

        // runs the taskflow until we stop the system
        executor.run_until(taskflow, [&running = running](){ return not running; });
    }
}