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

    // Todo Create a different id gen for systems so that components id are smaller and more packed
    
    // Forward declarations
    class ComponentRegistry;
    class AbstractSystem;

    /**
     * @brief Structure tag used to specify onCreation member on a component
     *
     * @todo find a better class name 
     */
    struct Ctor
    {
        virtual void onCreation(Entity* entity) = 0;
    };

    /**
     * @brief Structure tag used to specify onCreation member on a component
     *
     * @todo find a better class name 
     */
    struct Dtor
    {
        virtual void onDeletion(Entity* entity) = 0;
    };

    class EntitySystem
    {
    friend class Entity;
    friend class CommandDispatcher;
    public:
        EntitySystem();
        ~EntitySystem();

        EntityRef createEntity()
        {
            LOG_THIS_MEMBER("ECS");
            
            if(running)
                return cmdDispatcher.createEntity();
            else
            {
                const auto& id = registry.idGenerator.generateId();
                return entityPool.addComponent(id, id, this);
            }
        }

        void removeEntity(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            if(running)
                cmdDispatcher.deleteEntity(entity);
            else
                deleteEntityFromPool(entity);
        }

        // TODO: Add a getSystem and destroySystem functions
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
        CompRef<Type> attach(Entity* entity, Args&&... args) noexcept
        {
            LOG_THIS_MEMBER("ECS");

            try
            {
                Type* component;

                // Todo change this to add the command in the command dispatcher to avoid the dangling pointer
                if(running)
                {
                    component = cmdDispatcher.attachComp<Type>(std::forward<Args>(args)...);
                }
                else
                {
                    component = registry.retrieve<Type>()->internalCreateComponent(entity, std::forward<Args>(args)...);
                }

                auto res = CompRef<Type>(component, entity->id, this, not running);

                if constexpr(std::is_base_of_v<Ctor, Type>)
                    res->onCreation(entity);

                return res;
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ECS", "Can't attach component [" << typeid(Type).name() << "]: " << e.what());
            }

            return CompRef<Type>();
        }

        template <typename Type>
        void dettach(Entity* entity) const noexcept
        {
            
        }

        template <typename Event>
        inline void sendEvent(const Event& event) { registry.processEvent(event); }

        template <typename Comp>
        inline const _unique_id& getId() const noexcept { return registry.getTypeId<Comp>(); }

        void executeAll();

        // MasterRenderer* getMasterRenderer() { return registry.masterRenderer; }

        /** Return the registry of the ECS, mainly for testing purposes */
        inline constexpr const ComponentRegistry* getComponentRegistry() const noexcept { return &registry; }

        inline size_t getNbEntities() const { return entityPool.nbElements() - 1; }

        inline Entity* getEntity(_unique_id id) const { return entityPool.atEntity(id); }

        template <typename Comp>
        inline Comp* getComponent(_unique_id id) const { return registry.retrieve<Comp>()->getComponent(id); }

    private:
        void addEntityToPool(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            entityPool.addComponent(entity, *entity);
        }

        void deleteEntityFromPool(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            for(auto& comp : entity->componentList)
            {
                if(comp.entityHeldType == Entity::EntityHeld::EntityHeldType::id)
                {
                    // Todo Remove all attached component
                }
            }

            entityPool.removeComponent(entity);
        }

        template <typename Type>
        void addComponentToPool(Type* component)
        {
            LOG_THIS_MEMBER("ECS");

            if(component)
            {
                LOG_MILE("ECS", "addComponentToPool");
            }
            // entity->componentList.emplace(registry.getTypeId<Type>());
        }

        template <typename Type>
        void detachComponentFromPool(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            try
            {
                // TODO: Add this somewhere (ie in ecs.detach or in sparset.removeComponent)
                if constexpr(std::is_base_of_v<Dtor, Type>)
                {
                    auto res = getComponent<Type>(entity->id);
                    res->onDeletion(entity);
                }

                registry.retrieve<Type>()->internalRemoveComponent(entity);            
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ECS", e.what());
            }
        }

        bool running = false;
        ComponentRegistry registry;

        CommandDispatcher cmdDispatcher;

        /** Store all systems added to the ECS */
        std::vector<AbstractSystem*> systems;

        /** All the entities generated from the ECS */
        ComponentSet<Entity> entityPool;

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
        LOG_THIS("Entity");

        if(not ecsRef)
            return false;
        
        const auto& componentId = ecsRef->getId<Comp>();

        return has(componentId);
    }

    template <typename Comp>
    inline Comp* Entity::get() noexcept
    {
        LOG_THIS("Entity");

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

    template <typename Comp>
    void CompRef<Comp>::operator=(const CompRef& rhs)
    {
        LOG_THIS("Comp ref");

        if(rhs.initialized)
        {
            initialized = rhs.initialized;
            component   = rhs.component;
            entityId    = rhs.entityId;
            ecsRef      = rhs.ecsRef;
        }
        else
        {
            entityId = rhs.entityId;

            if(entityId != 0)
            {
                component = rhs.ecsRef->getComponent<Comp>(entityId);
                ecsRef = rhs.ecsRef;
                initialized = true;

                // Todo see if we propagate back the finding of the entity to the base ref !
                // rhs.entity = entity
                // rhs.initialized = true
                // Note that it needs to make the rhs not const or we need to make the member entity mutable !
            }
            else
            {
                LOG_ERROR("Comp ref", "Copy of a reference to an invalid entity");

                initialized = rhs.initialized;
                component   = rhs.component;
                ecsRef      = rhs.ecsRef;
            }
        }
    }

    template <typename Comp>
    Comp* CompRef<Comp>::operator->() const
    {
        if (initialized)
            return ecsRef->getComponent<Comp>(entityId);
        else
            return component;
    }

    template <typename Comp>
    CompRef<Comp>::operator Comp*() const
    {
        if (initialized)
            return ecsRef->getComponent<Comp>(entityId);
        else
            return component;
    }

    template <typename Type, typename... Types>
    template <typename Set>
    inline void Group<Type, Types...>::addEventToSet(Set setN)
    {
        LOG_THIS_MEMBER("Ecs Group");

        setN->onComponentCreation.emplace(id, [](Entity *entity) {
            LOG_INFO("Group", "On component creation for entity " << entity->id << ", sending event !");
            entity->world()->sendEvent(OnCompCreatedCheckForGroup<Group<Type, Types...>>{entity});
        });
    }

    template <typename Type, typename... Types>
    void Group<Type, Types...>::process()
    {
        LOG_THIS_MEMBER("Ecs Group");

        if(this->registry == nullptr)
            return;

        populateList(setList, 0, registry->retrieve<Type>(), registry->retrieve<Types>()...);

        size_t smallestSetIndex = 0;

        for(size_t i = 0; i < nbOfSets; ++i)
        {
            if(setList[i]->set->nbElements() < setList[smallestSetIndex]->set->nbElements())
                smallestSetIndex = i;
        }

        SetHolder<Type, Types...>* smallestSetHolder = setList[smallestSetIndex];
        const SparseSet* smallestSet = smallestSetHolder->set;

        setList[smallestSetIndex] = setList[nbOfSets - 1];

        setList[nbOfSets - 1] = smallestSetHolder;

        // const auto& elements = {registry->retrieve<Type>()->components, registry->retrieve<Types>()->components...};

        // const SparseSet& set = smallestSet(registry->retrieve<Type>()->components, registry->retrieve<Types>()->components...);

        LOG_INFO("Ecs Group", "Smallest set has: " + std::to_string(smallestSet->nbElements()) + " elements");

        // Todo add reserve and multiple emplace back in the component/sparse set
        // elements.reserve(smallestSet->nbElements()); // May need a -1

        // Todo check Branch: Parallel-Ecs to create a parallal implementation of grouping
        for(const auto& id : smallestSet->view())
        {
            GroupElement<Type, Types...> element(registry->world()->getEntity(id), this->world(), id);

            for(size_t j = 0; j < nbOfSets - 1; j++)
            {
                setList[j]->setElement(setList[j]->set, element, id);    
            }

            if(not element.toBeDeleted)
                elements.addComponent(id, element);
        }

        // Todo add this on ~Group() !
        // for(size_t i = 0; i < nbOfSets; i++)
        //     delete setList[i];

        // Add support for thread pools by passing a pool in this function and add the task inside of this pool
        // checkEntityInGroup<Type, Types...>(this->registry->getThreadPool(), elements);

        // const auto& it = elements.viewComponents();

        // Remove all elements that miss at least one component from the group
        // Todo Do not delete the component but make the iterator skip element to be deleted !
        // for(size_t i = 1; i < elements.nbElements(); i++)
        // {
            // const auto& element = elements[i];
            // if(element->toBeDeleted)
                // elements.removeComponent(element->entityId);
        // }
        //std::remove_if(it.begin(), it.end(), [](const GroupElement<Type, Types...>& element) { return element.toBeDeleted; });
    
        // Todo sort the group
    }
}