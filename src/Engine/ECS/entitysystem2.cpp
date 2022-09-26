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

#include "entitysystem2.h"

#include "system.h"

namespace pg
{
    namespace ecs
    {
        EntitySystem::EntitySystem() : pool(std::thread::hardware_concurrency()), registry(&pool)
        {
        }

        EntitySystem::~EntitySystem()
        {        
            for(size_t i = 0; i < systems.size(); i++)
            {
                delete systems[i];
            }
        }
    }
}