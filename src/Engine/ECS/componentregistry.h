#pragma once

#include <memory>
#include <map>
#include <unordered_map>
#include <functional>

#include "sparseset.h"
#include "entity.h"

#include "logger.h"
#include "serialization.h"

#include "uniqueid.h"

namespace pg
{
    class InputSystem;
    class ClassInstance;
    class InterpreterSystem;
    // class MasterRenderer;

    template <typename Type>
    struct Own;

    template <typename Type, typename... Types>
    struct Group;

    class EntitySystem;

    /**
     * @brief Structure tag used to specify onCreation member on a component
     *
     * @todo find a better class name 
     */
    struct Ctor
    {
        virtual void onCreation(EntityRef entity) = 0;

        virtual ~Ctor() {}
    };

    /**
     * @brief Structure tag used to specify onCreation member on a component
     *
     * @todo find a better class name 
     */
    struct Dtor
    {
        virtual void onDeletion(EntityRef entity) = 0;

        virtual ~Dtor() {}
    };

    /**
     * @brief Structure tag used to specify onCreation member on a component
     *
     * @todo find a better class name 
     */
    struct Copy
    {
        virtual void onCopy(EntityRef entity) = 0;

        virtual ~Copy() {}
    };

    /**
     * @brief Structure tag used to specify a component as a singleton component
     * 
     * When this tag is set for a component. That means that the component should only be created once
     * and when a system or a component need this component we can use the same refererence everywhere.
     * 
     * So when a component with this tag is created we can directly save the ref in the ecs and used
     * it throughout !
     * 
     * @todo make this
     */
    struct SingletonComp
    {

    };

    class ComponentRegistry
    {
        struct Storage {};
        struct AbstractEvent {};

    public:
        ComponentRegistry(EntitySystem *ecs);

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
                LOG_ERROR("Component Registry", "Trying to recreate a system that already existing with id: " << id << "Exiting");
                return;
            }
#endif

            componentDeleteMap.emplace(id, [owner](Entity* entity) {
                if constexpr(std::is_base_of_v<Dtor, Type>)
                {
                    auto res = owner->getComponent(entity->id);
                    res->onDeletion(entity);
                }

                owner->internalRemoveComponent(entity);
            });

            componentSerializeMap.emplace(id, [owner](Archive& archive, const Entity* entity) {
                serialize(archive, *(owner->getComponent(entity->id)));
            });

            componentStorageMap.emplace(id, static_cast<Storage*>(static_cast<Delegate*>(owner)));

            owner->_componentId = id;
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
                LOG_ERROR("Component Registry", "Trying to retrieve a system that doesn't exist, Exiting");

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

        void addEventListener(_unique_id eventId, InterpreterSystem *listener);

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

        inline bool hasGroup(_unique_id groupId) const
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
                LOG_ERROR("Component Registry", "Trying to recreate a group that already existing with id: " << id << "Exiting");
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
                LOG_ERROR("Component Registry", "Trying to retrieve a group that doesn't exist, Exiting");

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
            
            // Todo add a variable to keep track of the running state of the ECS
            if(not idMap[this])
            {
                idMap[this] = idGenerator.generateId();

                LOG_INFO("Component Registry", "Type: " << typeid(Type).name() << ", get id: " << idMap[this]);
            }

            return idMap[this];
            
            // This can't work as the static make this id the same through all the different object
            // static const _unique_id id = idGenerator.generateId();
            // return id;
        }

        inline void detachComponentFromEntity(Entity* entity, _unique_id id) const
        {
            componentDeleteMap.at(id)(entity);
        }

        inline void serializeComponentFromEntity(Archive& archive, const Entity* entity, _unique_id id) const
        {
            componentSerializeMap.at(id)(archive, entity);
        }

        inline EntitySystem* world() const noexcept { return ecsRef; }

        // Common singleton system
    public:
        mutable UniqueIdGenerator idGenerator;

    private:
        EntitySystem* const ecsRef;

        std::unordered_map<_unique_id, Storage*> componentStorageMap;
        std::unordered_map<_unique_id, std::function<void(Entity*)>> componentDeleteMap;
        std::unordered_map<_unique_id, std::function<void(Archive&, const Entity*)>> componentSerializeMap;
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

        inline _unique_id getId() const
        {
            return ref->getId();
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

        /**
         * @brief Add this owner object to the registry.
         * 
         * @param registry The registry where to add the owner object.
         */
        void setRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Own");

            LOG_INFO("Own", "Type: " << typeid(Type).name() << " get the id: " << registry->getTypeId<Type>());

            // Store a pointer to this owner object in the registry
            registry->store<Type>(this);
        }

        /**
         * @brief Create a new component of this type in the underlaying sparse set.
         * 
         * @tparam Args Types of the arguments to pass to the constructor of the component.
         * 
         * @param entity The entity to attach the new component to. 
         * @param args Arguments to pass to the constructor of the component.
         * 
         * @return Type* A pointer to the newly created component. 
         */
        template <typename... Args>
        inline Type* internalCreateComponent(Entity* entity, Args&&... args)
        {
            LOG_THIS_MEMBER("Own");

            // Create a new component and store it in a sparse set along with the entity id using it
            auto comp = components.addComponent(entity, std::forward<Args>(args)...);

            // Add the component to the entity
            entity->componentList.emplace(_componentId);

            // Call the on component creation callbacks to register the component in potential groups
            for(const auto& callback : onComponentCreation)
                callback.second(entity);

            return comp;
        }

        /**
         * @brief Remove a component of this type from the underlaying sparse set.
         * 
         * @param entity The entity to remove the component from. 
         */
        inline void internalRemoveComponent(Entity* entity)
        {
            LOG_THIS_MEMBER("Own");

            // Call the on component deletion callbacks to remove the component from potential groups
            for(const auto& callback : onComponentDeletion)
                callback.second(entity);

            // Erase the component from the entity
            auto it = std::find(entity->componentList.begin(), entity->componentList.end(), _componentId);
            
            if(it != entity->componentList.end())
                entity->componentList.erase(it);

            // Remove the component from the sparse set
            components.removeComponent(entity);
        }

        /**
         * @brief Get the component of an entity from the underlaying sparse set.
         * 
         * @param id The id of the entity that has the component.
         * 
         * @return A pointer to the component. 
         */
        inline Type* getComponent(_unique_id id) const
        {
            return components.atEntity(id);
        }

        inline typename ComponentSet<Type>::ComponentSetList view() const
        {
            LOG_THIS_MEMBER("Own");

            return components.viewComponents();
        }

        /**
         * @brief Get the id of the component.
         * 
         * @return _unique_id the id of the component. 
         */
        inline _unique_id getId() const
        {
            return _componentId;
        }

        ComponentSet<Type> components;

        std::map<_unique_id, void(*)(Entity*)> onComponentCreation;
        std::map<_unique_id, void(*)(Entity*)> onComponentDeletion;

        _unique_id _componentId = 0;
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

        // Todo always check if the component was not initialized in between calls to make sure to update the correct one
        Comp* operator->() const;

        operator Comp*() const;

        inline bool empty() const { return component == nullptr; }

        mutable bool initialized;
        mutable Comp* component;
        _unique_id entityId;
        const EntitySystem* ecsRef;  
    };
}