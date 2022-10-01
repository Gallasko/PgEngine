#pragma once

#include <vector>
#include <unordered_map>

#include "uniqueid.h"
#include "component.h"
#include "componentregistry.h"
#include "entity.h"

#include "logger.h"
#include "Memory/threadpool.h"

namespace pg
{
    namespace ecs
    {
        // Todo create a queue that hold all entity id that got deleted to reattribute them later on
        
        // Forward declarations
        class ComponentRegistry;
        class AbstractSystem;
        
        class EntitySystem
        {
        public:
            EntitySystem();
            ~EntitySystem();

            Entity createEntity()
            {
                LOG_THIS_MEMBER("ECS");

                Entity entity(generateId());

                return entity;
            }

            template<class Sys, typename... Args>
            Sys* createSystem(const Args&... args)
            {
                LOG_THIS_MEMBER("ECS");

                auto system = new Sys(args...);
                system->setRegistry(&registry);

                systems.push_back(system);

                return system;
            }

            //TODO make a template specialization capable of attaching an entity to an entity

            template<typename Type, typename... Args>
            Type* attach(Entity& entity, const Args&... args) const
            {
                LOG_THIS_MEMBER("ECS");

                entity.ecsRef = this;

                return registry.retrieve<Type>()->internalCreateComponent(entity, args...);
            }

        private:
            ComponentRegistry registry;

            std::vector<AbstractSystem*> systems;
            ThreadPool pool;
        };
    }
}