#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>

#include <taskflow.hpp>

#include "componentregistry.h"
#include "entity.h"
#include "system.h"
#include "commanddispatcher.h"

#include "serialization.h"

#include "logger.h"
#include "Memory/memorypool.h"

namespace pg
{
    // Todo create a queue that hold all entity id that got deleted to reattribute them later on

    // Todo Create a different id gen for systems so that components id are smaller and more packed
    
    // Forward declarations
    class ComponentRegistry;
    class AbstractSystem;

    // Todo add batching for entity and component creation

    template <typename Comp>
    struct CompListGetter
    {
        CompListGetter(CompRef<Comp> comp) : comp(comp) {}

        inline CompRef<Comp> get() const { return comp; } 

        CompRef<Comp> comp;
    };

    template<typename... Comps>
    struct CompList : public CompListGetter<Comps>...
    {
        CompList(EntityRef entity, CompRef<Comps>... comps) : CompListGetter<Comps>(comps)..., entity(entity) { }

        template<typename Comp>
        inline CompRef<Comp> get() const { return static_cast<const CompListGetter<Comp>*>(this)->get(); }

        EntityRef entity;
    };

    class EntitySystem
    {
    friend class Entity;
    friend class CommandDispatcher;
    public:
        EntitySystem();
        ~EntitySystem();

        inline void start()
        {
            LOG_THIS_MEMBER("ECS");

            if(running)
                return;

            running = true;

            runningThread = std::thread(&EntitySystem::executeAll, this);
        }

        inline void stop()
        {
            LOG_THIS_MEMBER("ECS");

            if (not running)
                return;

            running = false;

            executor.wait_for_all();

            if (runningThread.joinable())
                runningThread.join();
        }

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

        // TODO: Add a destroySystem function
        template <class Sys, typename... Args>
        Sys* createSystem(const Args&... args)
        {
            LOG_THIS_MEMBER("ECS");

            auto system = new Sys(args...);
            system->id = registry.getTypeId<Sys>();

            system->addToRegistry(&registry);

            systems.emplace(system->id, system);

            // Only add the system to the taskflow if the execution policy is set to sequential or independent !
            if(system->executionPolicy == ExecutionPolicy::Sequential)
            {
                auto task = taskflow.emplace([system](){system->execute();}).name(std::to_string(system->id));

                // Put the task after every other basic task
                task.succeed(basicTask);

                // Register the task in case we need to call precede and succeed
                tasks[system->id] = task;
            }
            else if (system->executionPolicy == ExecutionPolicy::Independent)
            {
                auto task = taskflow.emplace([system](){system->execute();}).name(std::to_string(system->id));

                // Register the task in case we need to call precede and succeed
                tasks[system->id] = task;
            }

            return system;
        }

        // Todo make proceed
        template <typename SysAfter, typename SysBefore>
        void succeed()
        {
            LOG_THIS_MEMBER("ECS");

            auto sys1Id = registry.getTypeId<SysAfter>();
            auto sys2Id = registry.getTypeId<SysBefore>();

            auto it1 = tasks.find(sys1Id);
            auto it2 = tasks.find(sys2Id);
            
            if(it1 != tasks.end() and it2 != tasks.end())
            {
                it1->second.succeed(it2->second);
                LOG_INFO("ECS", "System " << sys1Id << " will run after system " << sys2Id << " !");
            }
            else if(it1 == tasks.end() and it2 != tasks.end())
            {
                LOG_ERROR("ECS", "Systems " << sys1Id << " is not a registered task in ecs can't reorder task !");
            }
            else if(it1 != tasks.end() and it2 == tasks.end())
            {
                LOG_ERROR("ECS", "Systems " << sys2Id << " is not a registered task in ecs can't reorder task !");
            }
            else
            {
                LOG_ERROR("ECS", "Both systems " << sys1Id << " and " << sys2Id << " are not registered task in ecs can't reorder their task !");
            }
        }

        inline void dumbTaskflow() const
        {
            LOG_THIS_MEMBER("ECS");

            taskflow.dump(std::cout);
        }

        //TODO make a template specialization capable of attaching an entity to an entity

        template <class Sys>
        inline Sys* getSystem() const noexcept
        {
            LOG_THIS_MEMBER("ECS");

            const auto& id = registry.getTypeId<Sys>();

            try
            {
                return static_cast<Sys*>(systems.at(id));
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ECS", "Get system failed: " << e.what());
                return nullptr;
            }
        }

        template <typename Type, typename... Args>
        CompRef<Type> attach(EntityRef entity, Args&&... args) noexcept
        {
            LOG_THIS_MEMBER("ECS");

            try
            {
                Type* component;

                // Todo add lock a mutex for running to protect for race conditions or only build component with the cmdDispatcher 
                if(running)
                {
                    component = cmdDispatcher.attachComp<Type>(entity, std::forward<Args>(args)...);
                }
                else
                {
                    component = registry.retrieve<Type>()->internalCreateComponent(entity, std::forward<Args>(args)...);
                }

                std::cout << running << std::endl;
                
                auto res = CompRef<Type>(component, entity.id, this, not running);
                // auto res = CompRef<Type>(component, entity->id, this, false);

                if constexpr(std::is_base_of_v<Ctor, Type>)
                    res->onCreation(entity);

                // Todo make the systems capable of triggering on a component creation

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
            auto id = registry.getTypeId<Type>();

            try
            {
                registry.detachComponentFromEntity(entity, id);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR("ECS", "Can't detach component [" << id << "] from entity [" << entity->id << "]: " << e.what());
            }
        }

        template <typename Event>
        inline void sendEvent(Event&& event) { LOG_THIS_MEMBER("ECS"); registry.processEvent(std::forward<Event>(event)); }

        template <typename Comp>
        inline const _unique_id& getId() const noexcept { LOG_THIS_MEMBER("ECS"); return registry.getTypeId<Comp>(); }

        void executeOnce();

        void executeAll();

        /** Return the registry of the ECS, mainly for testing purposes */
        inline constexpr const ComponentRegistry* getComponentRegistry() const noexcept { return &registry; }

        inline size_t getNbEntities() const { LOG_THIS_MEMBER("ECS"); return entityPool.nbElements() - 1; }

        inline Entity* getEntity(_unique_id id) const { LOG_THIS_MEMBER("ECS"); return entityPool.atEntity(id); }

        template <typename Comp>
        inline Comp* getComponent(_unique_id id) const { LOG_THIS_MEMBER("ECS"); return registry.retrieve<Comp>()->getComponent(id); }

    private:
        friend void serialize<>(Archive& archive, const EntitySystem& ecs);

        void addEntityToPool(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            std::lock_guard<std::mutex> lock(entityMutex);

            entityPool.addComponent(entity, *entity);
        }

        void deleteEntityFromPool(Entity* entity)
        {
            LOG_THIS_MEMBER("ECS");

            componentMutex.lock();

            for(auto& comp : entity->componentList)
            {
                if(comp.entityHeldType == Entity::EntityHeld::EntityHeldType::id)
                {
                    try
                    {
                        registry.detachComponentFromEntity(entity, comp.getId());
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("ECS", "Can't detach component [" << comp.getId() << "] from entity [" << entity->id << "]: " << e.what());
                    }
                }
            }

            componentMutex.unlock();

            std::lock_guard<std::mutex> lock(entityMutex);

            entityPool.removeComponent(entity);
        }

        template <typename Type>
        void addComponentToPool(EntityRef entity, Type* component)
        {
            LOG_THIS_MEMBER("ECS");

            if(component)
            {
                LOG_MILE("ECS", "addComponentToPool");

                std::lock_guard<std::mutex> lock(componentMutex);

                registry.retrieve<Type>()->internalCreateComponent(entity, *component);
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
        std::map<_unique_id, AbstractSystem*> systems;

        /** All the entities generated from the ECS */
        ComponentSet<Entity> entityPool;

        /** Running thread of the ECS */
        std::thread runningThread;

        /** Taskflow of all the system of the ecs */
        tf::Taskflow taskflow;

        /** Main executor of the ecs */
        tf::Executor executor;

        /** Map of all the task associated to systems */
        std::unordered_map<_unique_id, tf::Task> tasks;

        /** Last task of the mandatory ecs base systems */
        tf::Task basicTask;

        /** Mutex to protect the check of existence of an entity */
        mutable std::mutex entityMutex;

        /** Mutex to protect the check of existence of an entity */
        mutable std::mutex componentMutex;
    };

    template <typename Comp>
    inline bool Entity::has() const noexcept
    {
        LOG_THIS_MEMBER("Entity");

        if(not ecsRef)
        {
            LOG_ERROR("Entity", "Entity is not referenced in any ECS");

            return false;
        }
        
        const auto& componentId = ecsRef->getId<Comp>();

        return has(componentId);
    }

    template <typename Comp>
    inline CompRef<Comp> Entity::get() noexcept
    {
        LOG_THIS_MEMBER("Entity");

        if(not ecsRef)
        {
            LOG_ERROR("Entity", "Entity is not referenced in any ECS");

            return CompRef<Comp>();
        }
        
        const auto& componentId = ecsRef->getId<Comp>();

        const auto& it = std::find(componentList.begin(), componentList.end(), componentId);

        if(it != componentList.end())
        {
            auto ent = ecsRef->getEntity(id);
            auto initialized = id != 0 and ent;

            std::cout << "Comp " << initialized << std::endl;

            return CompRef<Comp>(ecsRef->registry.retrieve<Comp>()->getComponent(id), id, ecsRef, initialized);
        }

        LOG_ERROR("Entity", "Entity doesn't have component: " << componentId);

        return CompRef<Comp>();
    }

    template <typename Comp>
    void CompRef<Comp>::operator=(const CompRef& rhs)
    {
        LOG_THIS_MEMBER("Comp ref");

        ecsRef      = rhs.ecsRef;
        entityId    = rhs.entityId;
        initialized = rhs.initialized;
        component   = rhs.component;

        if(not initialized)
        {
            if(entityId != 0)
            {
                auto fetchComponent = rhs.ecsRef->getComponent<Comp>(entityId);

                if(fetchComponent)
                {
                    component   = fetchComponent;
                    initialized = true;
                }
                // Todo see if we propagate back the finding of the entity to the base ref !
                // rhs.entity = entity
                // rhs.initialized = true
                // Note that it needs to make the rhs not const or we need to make the member entity mutable !
            }
            else
            {
                LOG_ERROR("Comp ref", "Copy of a reference to an invalid entity");
            }
        }
    }

    template <typename Comp>
    Comp* CompRef<Comp>::operator->() const
    {
        LOG_THIS_MEMBER("Comp ref");

        if (initialized)
            return ecsRef->getComponent<Comp>(entityId);
        else
        {
            // Try to find the component in the ecs to update this ref
            auto comp = ecsRef->getComponent<Comp>(entityId);
            
            // Component found, updating this entity ref
            if(entityId != 0 and comp)
            {
                component = comp;
                initialized = true;
            }

           return component;
        }
    }

    template <typename Comp>
    CompRef<Comp>::operator Comp*() const
    {
        LOG_THIS_MEMBER("Comp ref");

        if (initialized)
            return ecsRef->getComponent<Comp>(entityId);
        else
        {
            // Try to find the component in the ecs to update this ref
            auto comp = ecsRef->getComponent<Comp>(entityId);
            
            // Component found, updating this entity ref
            if(entityId != 0 and comp)
            {
                component = comp;
                initialized = true;
            }

           return component;
        }

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

    template <typename Type>
    void CommandDispatcher::ComponentCommand::setupFunctions()
    {
        LOG_THIS_MEMBER("Command Dispatcher");

        struct Delegate : public ComponentCommand::ComponentCommand::Storage, public Type {};

        addInEcs = [](EntitySystem* ecs, EntityRef entity, Storage* component) { ecs->addComponentToPool(entity, static_cast<Type*>(static_cast<Delegate*>(component))); };

        deleteComp = [](Storage* component) { delete static_cast<Type*>(static_cast<Delegate*>(component)); };
    }
}