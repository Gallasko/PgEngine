#pragma once

#include <vector>
#include <unordered_map>

#include <taskflow.hpp>

#include "component.h"
#include "componentregistry.h"
#include "entity.h"
#include "system.h"

#include "logger.h"
#include "Memory/memorypool.h"

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
            EntitySystem(bool emptyEcs = false);
            ~EntitySystem();

            Entity* createEntity()
            {
                LOG_THIS_MEMBER("ECS");
                
                return entityPool.allocate(registry.idGenerator.generateId());
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
            Type* attach(Entity* entity, const Args&... args) const
            {
                LOG_THIS_MEMBER("ECS");

                entity->ecsRef = this;

                try
                {
                    // Todo set the ecs ref of the created component to this as everything created from here should be a component ?
                    auto res = registry.retrieve<Type>()->internalCreateComponent(entity, args...);

                    res->ecsRef = this;

                    return res;
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("ECS", Strfy() << "Can't attach component [" << typeid(Type).name() << "]: " << e.what());
                }

                return nullptr;
            }

            template<typename Type>
            void dettach(Entity* entity) const
            {
                LOG_THIS_MEMBER("ECS");

                try
                {
                    registry.retrieve<Type>()->internalRemoveComponent(entity);            
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("ECS", e.what());
                }
            }

            void executeAll();

            MasterRenderer* getMasterRenderer() { return registry.masterRenderer; }

        private:
            bool running = false;
            ComponentRegistry registry;

            std::vector<AbstractSystem*> systems;
            AllocatorPool<Entity> entityPool;

            /** Store all systems that doesn't have be executed by the ecs (systems tagged as policy = manual, onEvent or storage) */
            std::unordered_map<_unique_id, AbstractSystem*> storageMap;

            // Main executor of the ecs
            tf::Executor executor;

            // Taskflow of all the system of the ecs
            tf::Taskflow taskflow;
        };
    }
}