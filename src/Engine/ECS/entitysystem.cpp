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
    constexpr char * DOM = "ECS";
}

namespace pg
{
    namespace ecs
    {
        EntitySystem::EntitySystem(bool emptyEcs)
        {
            LOG_THIS_MEMBER(DOM);

            if(not emptyEcs)
            {

            }
        }

        EntitySystem::~EntitySystem()
        {        
            LOG_THIS_MEMBER(DOM);

            for(size_t i = 0; i < systems.size(); i++)
            {
                delete systems[i];
            }
        }

        void EntitySystem::executeAll()
        {
            LOG_THIS_MEMBER(DOM);

            running = true;

            // executor.run(taskflow).wait();

            // for(const auto& system : systems)
            // {
            //     system->execute();
            // }

            running = false;
        }
    }
}