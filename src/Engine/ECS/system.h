#pragma once

#include <string>
#include <unordered_map>

#include <taskflow.hpp>

#include "entity.h"
#include "componentregistry.h"
#include "group.h"
#include "eventlistener.h"

#include "logger.h"

namespace pg
{
    enum class ExecutionPolicy
    {
        Manual      = 0,
        Sequential  = 1,
        Parallel    = 2,
        Independent = 3,
        OnEvent     = 4,
        Storage     = 5
    };

    struct StoragePolicy { };

    struct ManualPolicy { };

    struct IndependentPolicy { };

    struct ParallelPolicy
    {
        virtual void parallelExecute(tf::Taskflow&) = 0;
    };

    /**
     * @brief Abstract representation of a system
     */
    struct AbstractSystem
    {
        virtual ~AbstractSystem() { LOG_THIS_MEMBER("System"); }

        virtual void execute() { LOG_THIS_MEMBER("System"); }

        // Todo
        inline void setPolicy(const ExecutionPolicy& policy) { executionPolicy = policy; }

        ExecutionPolicy executionPolicy = ExecutionPolicy::Sequential;

        ComponentRegistry *registry = nullptr;

        _unique_id id;

        // Todo make function onAdd and onDelete of a component that default to nothing if not used
    };

    template <typename... Comps>
    struct System;

    template <typename Sys>
    void registerComponents(Sys*, ComponentRegistry*) { LOG_THIS("System"); }

    template <typename Comp, typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Own<Comp>>&, const Comps&... comps)
    {
        LOG_THIS("System");

        LOG_INFO("System", Strfy() << "Registering an own to '" << typeid(Comp).name() << "' to the system.");

        static_cast<Own<Comp>*>(system)->setRegistry(registry);
        registerComponents(system, registry, comps...);
    }

    template <typename Comp, typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Ref<Comp>>&, const Comps&... comps)
    {
        LOG_THIS("System");
        
        LOG_INFO("System", Strfy() << "Registering a ref to '" << typeid(Comp).name() << "' to the system.");
        
        static_cast<Ref<Comp>*>(system)->setRegistry(registry);
        registerComponents(system, registry, comps...);
    }

    template <typename Event, typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<Listener<Event>>&, const Comps&... comps)
    {
        LOG_THIS("System");
        
        LOG_INFO("System", Strfy() << "Registering a listener to event '" << typeid(Event).name() << "' to the system.");
        
        static_cast<Listener<Event>*>(system)->setRegistry(registry);
        registerComponents(system, registry, comps...);
    }

    template <typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<StoragePolicy>&, const Comps&... comps)
    {
        LOG_THIS("System");
        
        LOG_INFO("System", "Registering the system as a storage one");

        if(system->executionPolicy != ExecutionPolicy::Sequential)
            LOG_ERROR("System", "Trying to set two different execution policies !");

        system->setPolicy(ExecutionPolicy::Storage);
        
        registerComponents(system, registry, comps...);
    }

    template <typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<ManualPolicy>&, const Comps&... comps)
    {
        LOG_THIS("System");
        
        LOG_INFO("System", "Registering the system as a manual one");

        if(system->executionPolicy != ExecutionPolicy::Sequential)
            LOG_ERROR("System", "Trying to set two different execution policies !");

        system->setPolicy(ExecutionPolicy::Manual);
        
        registerComponents(system, registry, comps...);
    }

    template <typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<ParallelPolicy>&, const Comps&... comps)
    {
        LOG_THIS("System");
        
        LOG_INFO("System", "Registering the system as a parallel one");

        if(system->executionPolicy != ExecutionPolicy::Sequential)
            LOG_ERROR("System", "Trying to set two different execution policies !");

        system->setPolicy(ExecutionPolicy::Parallel);
        
        registerComponents(system, registry, comps...);
    }

    template <typename... Comps, typename Sys>
    void registerComponents(Sys *system, ComponentRegistry *registry, const tag<IndependentPolicy>&, const Comps&... comps)
    {
        LOG_THIS("System");
        
        LOG_INFO("System", "Registering the system as a parallel one");

        if(system->executionPolicy != ExecutionPolicy::Sequential)
            LOG_ERROR("System", "Trying to set two different execution policies !");

        system->setPolicy(ExecutionPolicy::Independent);
        
        registerComponents(system, registry, comps...);
    }

    template <typename... Comps>
    struct System : public AbstractSystem, public Comps...
    {
        System() : AbstractSystem(), Comps()...
        {
            LOG_THIS_MEMBER("System");
        }

        ~System()
        {
            LOG_THIS_MEMBER("System");
        }

        template <typename Event>
        void sendEvent(const Event& event)
        {
            LOG_THIS_MEMBER("System");

            if(this->registry)
                this->registry->processEvent(event);
        }

        void addToRegistry(ComponentRegistry *registry)
        {
            LOG_THIS_MEMBER("System");

            this->registry = registry;

            this->id = this->registry->getTypeId<System<Comps...>>();

            registerComponents(this, registry, tag<Comps>{}...);
        }

        // Todo : move those function (create component and delete component) in the Own and Ref struct
        // And call it create component so the system inherit of the correct one
        template <typename Type, typename... Args>
        inline Type* createComponent(Entity* entity, Args&&... args)
        {
            LOG_THIS_MEMBER("System");

            return this->createRefferedComponent<Type>(entity, std::forward<Args>(args)...);
        }

        template <typename Type, typename... Args>
        inline Type* createOwnedComponent(Entity* entity, Args&&... args)
        {
            LOG_THIS_MEMBER("System");

            return this->Own<Type>::internalCreateComponent(entity, std::forward<Args>(args)...);
        }

        template <typename Type, typename... Args>
        inline Type* createRefferedComponent(Entity* entity, Args&&... args)
        {
            LOG_THIS_MEMBER("System");

            return this->Ref<Type>::internalCreateComponent(entity, std::forward<Args>(args)...);
        }

        template <typename Type>
        inline void removeComponent(Entity* entity)
        {
            LOG_THIS_MEMBER("System");

            this->removeRefferedComponent(entity);
        }

        template <typename Type>
        void removeRefferedComponent(Entity* entity)
        {
            LOG_THIS_MEMBER("System");

            this->Ref<Type>::internalRemoveComponent(entity);
        }

        template <typename Type>
        inline void removeOwnedComponent(Entity* entity)
        {
            LOG_THIS_MEMBER("System");

            this->Own<Type>::internalRemoveComponent(entity);
        }

        template <typename Type>
        inline typename ComponentSet<Type>::ComponentSetList view() const
        {
            LOG_THIS_MEMBER("System");

            return this->Ref<Type>::view();
        }

        template <typename Type, typename... Types>
        const Group<Type, Types...>* group() const
        {
            LOG_THIS_MEMBER("System");

            if(registry == nullptr)
            {
                LOG_ERROR("System", "No registry specified, can't create a group");
                return nullptr;
            }

            if(Group<Type, Types...>::groupId != 0)
                return registry->retrieveGroup<Type, Types...>();
            else
            {
                LOG_INFO("System", "Creating new group");
                // Todo fix this with the new id generation
                // auto group = new Group<Type, Types...>(generateId());
                // 
                // group->setRegistry(registry);
                // group->process();

                // return group;

                return nullptr;
            }
        }
    };
}