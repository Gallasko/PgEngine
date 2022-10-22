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

namespace pg
{
    namespace ecs
    {
        EntitySystem::EntitySystem() : pool(std::thread::hardware_concurrency()), registry(&pool)
        {
            LOG_THIS_MEMBER("ECS");
        }

        EntitySystem::~EntitySystem()
        {        
            LOG_THIS_MEMBER("ECS");

            for(size_t i = 0; i < systems.size(); i++)
            {
                delete systems[i];
            }
        }

        void EntitySystem::executeAll()
        {
            for(const auto& system : systems)
            {
                system->execute();
            }
        }
    }
}