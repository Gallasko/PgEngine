#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include <taskflow.hpp>

#include "componentregistry.h"
#include "entity.h"
#include "system.h"
#include "commanddispatcher.h"

#include "logger.h"
#include "Memory/memorypool.h"

namespace pg
{
    // Todo create a queue that hold all entity id that got deleted to reattribute them later on
    
    // Forward declarations
    class ComponentRegistry;
    class AbstractSystem;

    class EntitySystem
    {
    friend class Entity;
    public:
        EntitySystem(bool emptyEcs = false);
        ~EntitySystem();

        Entity* createEntity()
        {
            LOG_THIS_MEMBER("ECS");

            return entityPool.allocate(registry.idGenerator.generateId(), this);
        }

        void removeEntity(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            // Todo Remove all attached component

            entityPool.release(entity);
        }

        template <class Sys, typename... Args>
        Sys* createSystem(const Args&... args)
        {
            LOG_THIS_MEMBER("ECS");

            auto system = new Sys(args...);
            system->setRegistry(&registry);

            // Todo only add the system to the taskflow if the execution policy permits it !
            systems.push_back(system);

            taskflow.emplace([system](){system->execute();});

            return system;
        }

        //TODO make a template specialization capable of attaching an entity to an entity

        template <typename Type, typename... Args>
        Type* attach(Entity* entity, const Args&... args) const noexcept
        {
            LOG_THIS_MEMBER("ECS");

            try
            {
                // Todo set the ecs ref of the created component to this as everything created from here should be a component ?
                auto res = registry.retrieve<Type>()->internalCreateComponent(entity, args...);

                return res;
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ECS", Strfy() << "Can't attach component [" << typeid(Type).name() << "]: " << e.what());
            }

            return nullptr;
        }

        template <typename Type>
        void dettach(Entity* entity) const noexcept
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

        template <typename Event>
        inline void sendEvent(const Event& event) { registry.processEvent(event); }

        template <typename Comp>
        inline const _unique_id& getId() const noexcept { return registry.getTypeId<Comp>(); }

        void executeAll();

        MasterRenderer* getMasterRenderer() { return registry.masterRenderer; }

    private:
        bool running = false;
        ComponentRegistry registry;

        CommandDispatcher cmdDispatcher;

        std::vector<AbstractSystem*> systems;
        AllocatorPool<Entity> entityPool;

        /** Store all systems that doesn't have be executed by the ecs (systems tagged as policy = manual, onEvent or storage) */
        std::unordered_map<_unique_id, AbstractSystem*> storageMap;

        // Taskflow of all the system of the ecs
        tf::Taskflow taskflow;

        // Main executor of the ecs
        tf::Executor executor;
    };

    template <typename Comp>
    inline bool Entity::has() const noexcept
    {
        if(not ecsRef)
            return false;
        
        const auto& componentId = ecsRef->getId<Comp>();

        return has(componentId);
    }

    template <typename Comp>
    inline Comp* Entity::get() noexcept
    {
        if(not ecsRef)
            return nullptr;
        
        const auto& componentId = ecsRef->getId<Comp>();

        const auto& it = std::find(componentList.begin(), componentList.end(), componentId);

        if(it != componentList.end())
        {
            return ecsRef->registry.retrieve<Comp>()->getComponent(componentId);
        }

        return nullptr;
    }
}