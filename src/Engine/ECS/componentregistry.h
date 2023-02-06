#pragma once

#include <memory>
#include <map>
#include <unordered_map>
#include <functional>

#include "sparseset.h"
#include "entity.h"

#include "logger.h"

#include "uniqueid.h"

namespace pg
{
    class InputSystem;
    class MasterRenderer;

    template <typename Type>
    struct Own;

    template <typename Type, typename... Types>
    struct Group;

    class ComponentRegistry
    {
        struct Storage {};
        struct AbstractEvent {};

    public:
        ComponentRegistry();

        ~ComponentRegistry();

        template <typename Type>
        void store(Own<Type>* owner) noexcept
        {
            LOG_THIS_MEMBER("Component Registry");

            struct Delegate : public Storage, public Own<Type> { };

            const auto& id = getTypeId<Type>();

            // Todo see if there is a performance hit to keep this function or does it get optimized as it should be always false in production code
            // Block to find if a system is already registered in the ecs
#ifdef PROD
            if(const auto& it = componentStorageMap.find(id); it != componentStorageMap.end())
            {
                LOG_ERROR("Component Registry", Strfy() << "Trying to recreate a system that already existing with id: " << id << "Exiting");
                return;
            }
#endif

            componentStorageMap.emplace(id, static_cast<Storage*>(static_cast<Delegate*>(owner)));
        }

        template <typename Type>
        Own<Type>* retrieve() const
        {
            LOG_THIS_MEMBER("Component Registry");

            struct Delegate : public Storage, public Own<Type> { };

            const auto& id = getTypeId<Type>();
            // Todo catch errors when component doesnt exist in storage or no system own the component (so it doesnt exist in storage !)

            // Todo see if there is a performance hit to keep this function or does it get optimized as it should be always false in production code
            // Block to find if a group is already registered in the ecs
#ifdef PROD
            if(const auto& it = componentStorageMap.find(id); it == componentStorageMap.end())
            {
                LOG_ERROR("Component Registry", Strfy() << "Trying to retrieve a system that doesn't exist, Exiting");

                throw std::runtime_error(Strfy() << "System  [" << typeid(Type).name() << "] is not registered");
            }
#endif

            return static_cast<Own<Type>*>(static_cast<Delegate*>(componentStorageMap.at(id)));
        }

        template<typename Event, typename EventListener>
        void addEventListener(EventListener* listener)
        {
            LOG_THIS_MEMBER("Component Registry");

            const auto& id = getTypeId<Event>();
            
            eventStorageMap[id].emplace_back([listener](const AbstractEvent& event)
            {
                struct DelegateEvent : public AbstractEvent, public Event { virtual ~DelegateEvent() {} };

                listener->onEvent(static_cast<const Event&>(static_cast<const DelegateEvent&>(event)));
            });
        }

        template<typename Event>
        void processEvent(const Event& event)
        {
            LOG_THIS_MEMBER("Component Registry");

            struct DelegateEvent : public AbstractEvent, public Event { virtual ~DelegateEvent() {} };

            const auto& id = getTypeId<Event>();

            for(auto& eventListener : eventStorageMap[id])
            {
                eventListener(static_cast<const DelegateEvent&>(event));
            }
        }

        bool hasGroup(_unique_id groupId) const
        {
            return groupStorageMap.count(groupId) > 0;
        }

        template <typename Type, typename... Types>
        void storeGroup(Group<Type, Types...>* group) noexcept
        {
            LOG_THIS_MEMBER("Component Registry");

            struct Delegate : public Storage, public Group<Type, Types...> { virtual ~Delegate() {} };

            // Todo see if there is a performance hit to keep this function or does it get optimized as it should be always false in production code
            // Block to find if a group is already registered in the ecs

            const auto& id = getTypeId<Group<Type, Types...>>();

#ifdef PROD
            if(const auto& it = groupStorageMap.find(id); it != groupStorageMap.end())
            {
                LOG_ERROR("Component Registry", Strfy() << "Trying to recreate a group that already existing with id: " << id << "Exiting");
                // Todo see if we need to throw an exception or not
                // throw std::runtime_error("Group already registered");
                return;
            }
#endif

            groupStorageMap.emplace(id, static_cast<Storage*>(static_cast<Delegate*>(group)));
        }

        template <typename Type, typename... Types>
        Group<Type, Types...>* retrieveGroup() const
        {
            LOG_THIS_MEMBER("Component Registry");

            struct Delegate : public Storage, public Group<Type, Types...> { virtual ~Delegate() {} };

            const auto& id = getTypeId<Group<Type, Types...>>();

            // Todo see if there is a performance hit to keep this function or does it get optimized as it should be always false in production code
            // Block to find if a group is already registered in the ecs
#ifdef PROD
            if(const auto& it = groupStorageMap.find(id); it == groupStorageMap.end())
            {
                LOG_ERROR("Component Registry", Strfy() << "Trying to retrieve a group that doesn't exist, Exiting");

                throw std::runtime_error(Strfy() <<  "Group [" << typeid(Type).name() << "] is not registered");
            }
#endif

            return static_cast<Group<Type, Types...>*>(static_cast<Delegate*>(groupStorageMap.at(id)));
        }
    
        template <typename Type>
        const _unique_id& getTypeId() const noexcept
        {
            // Todo find a better implementation of this
            static std::map<const ComponentRegistry*, _unique_id> idMap;
            
            if(not idMap[this])
                idMap[this] = idGenerator.generateId();

            LOG_INFO("Component Registry", Strfy() << "Type: " << typeid(Type).name() << ", get id: " << idMap[this]);

            return idMap[this];
            
            // This can't work as the static make this id the same through all the different object
            // static const _unique_id id = idGenerator.generateId();
            // return id;
        }

        // Common singleton system
    public:
        MasterRenderer* masterRenderer;

        mutable UniqueIdGenerator idGenerator;

    private:
        std::unordered_map<_unique_id, Storage*> componentStorageMap;
        std::unordered_map<_unique_id, Storage*> groupStorageMap;
        std::unordered_map<_unique_id, std::vector<std::function<void(const AbstractEvent&)>>> eventStorageMap;
    };

    template <class Type>
    struct Ref 
    {
        // Take an unique id only to have the same signature as Own object
        Ref() { LOG_THIS_MEMBER("Ref"); }

        Ref(Own<Type> *ref) : ref(ref) { LOG_THIS_MEMBER("Ref"); }

        // Todo maybe remove the virtual destructor as the only thing that inherit Ref is Own
        // And it only inherits it to be stored in the registry.
        virtual ~Ref() { LOG_THIS_MEMBER("Ref"); }

        void setRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Ref");

            ref = registry->retrieve<Type>();
        }

        template <typename... Args>
        inline Type* internalCreateComponent(Entity* entity, Args&&... args)
        {
            LOG_THIS_MEMBER("Ref");

            return ref->internalCreateComponent(entity, std::forward<Args>(args)...);
        }

        inline void internalRemoveComponent(Entity* entity)
        {
            LOG_THIS_MEMBER("Ref");

            ref->internalRemoveComponent(entity);
        }

        inline typename ComponentSet<Type>::ComponentSetList view() const
        {
            LOG_THIS_MEMBER("Ref");

            return ref->view();
        }

        Own<Type> *ref;
    };

    template <class Type>
    struct Own : public Ref<Type>
    {
        Own() : Ref<Type>(this)
        {
            LOG_THIS_MEMBER("Own");
        }

        virtual ~Own() { LOG_THIS_MEMBER("Own"); }

        void setRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Own");

            LOG_INFO("Own", Strfy() << "Type: " << typeid(Type).name() << " get the id: " << registry->getTypeId<Type>());

            registry->store<Type>(this);
        }

        template <typename... Args>
        inline Type* internalCreateComponent(Entity* entity, Args&&... args)
        {
            LOG_THIS_MEMBER("Own");

            // Create a new component and store it in a sparse set along with the entity id using it
            auto comp = components.addComponent(entity, std::forward<Args>(args)...);

            for(const auto& callback : onComponentCreation)
                callback.second(entity);

            return comp;
        }

        inline void internalRemoveComponent(Entity* entity)
        {
            LOG_THIS_MEMBER("Own");

            components.removeComponent(entity);

            for(const auto& callback : onComponentDeletion)
                callback.second(entity);
        }

        inline Type* getComponent(_unique_id id) const
        {
            return components[id];
        }

        inline typename ComponentSet<Type>::ComponentSetList view() const
        {
            LOG_THIS_MEMBER("Own");

            return components.viewComponents();
        }

        ComponentSet<Type> components;

        std::map<_unique_id, void(Entity*)> onComponentCreation;
        std::map<_unique_id, void(Entity*)> onComponentDeletion;
    };

    template <typename Comp>
    struct CompRef
    {
        CompRef() : initialized(false), component(nullptr), entityId(0), ecsRef(nullptr) {}

        CompRef(Comp* component, _unique_id id, const EntitySystem* ecs, bool initialized = true) : initialized(initialized), component(component), entityId(id), ecsRef(ecs) {}

        CompRef(const CompRef& rhs)
        {
            (*this) = rhs; 
        }

        void operator=(const CompRef& rhs);

        Comp* operator->() const
        {
            return component;
        }

        operator Comp*() const
        {
            return component;
        }

        inline bool empty() const { return component == nullptr; }

        bool initialized;
        Comp* component;
        _unique_id entityId;
        const EntitySystem* ecsRef;  
    };
}