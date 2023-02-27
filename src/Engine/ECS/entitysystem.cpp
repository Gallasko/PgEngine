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
    EntitySystem::EntitySystem() : cmdDispatcher(this), registry(this)
    {
        LOG_THIS_MEMBER(DOM);

        // Add the command dispatcher as the first element of the task flow
        basicTask = taskflow.emplace([this](){cmdDispatcher.process();});
    }

    EntitySystem::~EntitySystem()
    {
        LOG_THIS_MEMBER(DOM);

        for(auto& sys : systems)
        {
            delete sys.second;
        }
    }

    void EntitySystem::executeAll()
    {
        LOG_THIS_MEMBER(DOM);

        running = true;

        executor.run(taskflow).wait();

        // for(const auto& system : systems)
        // {
        //     system->execute();
        // }

        running = false;
    }
}