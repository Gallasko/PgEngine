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
    friend class CommandDispatcher;
    public:
        EntitySystem();
        ~EntitySystem();

        Entity* createEntity()
        {
            LOG_THIS_MEMBER("ECS");
            
            if(running)
                return cmdDispatcher.createEntity();
            else
                return entityPool.allocate(registry.idGenerator.generateId(), this);
        }

        void removeEntity(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            if(running)
                cmdDispatcher.deleteEntity(entity);
            else
                deleteEntityFromPool(entity);
        }

        template <class Sys, typename... Args>
        Sys* createSystem(const Args&... args)
        {
            LOG_THIS_MEMBER("ECS");

            auto system = new Sys(args...);
            system->addToRegistry(&registry);

            systems.push_back(system);

            // Only add the system to the taskflow if the execution policy is set to sequential or independent !
            if(system->executionPolicy == ExecutionPolicy::Sequential)
            {
                auto task = taskflow.emplace([system](){system->execute();});

                // Put the task after every other basic task
                task.succeed(basicTask);

                // Register the task in case we need to call precede and succeed
                tasks[system->id] = task;
            }
            else if (system->executionPolicy == ExecutionPolicy::Independent)
            {
                auto task = taskflow.emplace([system](){system->execute();});

                // Register the task in case we need to call precede and succeed
                tasks[system->id] = task;
            }

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

        /** Return the registry of the ECS, mainly for testing purposes */
        inline constexpr const ComponentRegistry* getComponentRegistry() const noexcept { return &registry; }

        inline constexpr size_t getNbEntities() const { return entityPool.getNbElements(); }

    private:
        void addEntityToPool(Entity* entity)
        {
            entityPool.allocate(*entity);
        }

        void deleteEntityFromPool(Entity* entity)
        {
            for(auto& comp : entity->componentList)
            {
                if(comp.entityHeldType == Entity::EntityHeld::EntityHeldType::id)
                {
                    // Todo Remove all attached component
                }
            }

            entityPool.release(entity);
        }

        bool running = false;
        ComponentRegistry registry;

        CommandDispatcher cmdDispatcher;

        /** Store all systems added to the ECS */
        std::vector<AbstractSystem*> systems;

        /** All the entities generated from the ECS */
        AllocatorPool<Entity> entityPool;

        /** Taskflow of all the system of the ecs */
        tf::Taskflow taskflow;

        /** Main executor of the ecs */
        tf::Executor executor;

        /** Map of all the task associated to systems */
        std::unordered_map<_unique_id, tf::Task> tasks;

        /** Last task of the mandatory ecs base systems */
        tf::Task basicTask;
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