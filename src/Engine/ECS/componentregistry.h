#pragma once

#include <memory>
#include <unordered_map>

#include "sparseset.h"
#include "entity.h"

#include "logger.h"

namespace pg
{
    namespace ecs
    {
        template <typename Type>
        struct Own;

        template <typename Type, typename... Types>
        struct Group;

        // TODO in destructor delete all groups !
        class ComponentRegistry
        {
            struct Storage {};

        public:
            ~ComponentRegistry()
            {
                LOG_THIS_MEMBER("Component Registry");

                for(auto group : groupStorageMap)
                    delete group.second;
            }
            template <typename Type>
            void store(Own<Type>* owner)
            {
                LOG_THIS_MEMBER("Component Registry");

                struct Delegate : public Storage, public Own<Type> { };

                componentStorageMap.emplace(Type::componentId, static_cast<Storage*>(static_cast<Delegate*>(owner)));
            }

            template <typename Type>
            Own<Type>* retrieve() const
            {
                LOG_THIS_MEMBER("Component Registry");

                struct Delegate : public Storage, public Own<Type> { };

                return static_cast<Own<Type>*>(static_cast<Delegate*>(componentStorageMap.at(Type::componentId)));
            }

            template <typename Type, typename... Types>
            void storeGroup(Group<Type, Types...>* group)
            {
                LOG_THIS_MEMBER("Component Registry");

                struct Delegate : public Storage, public Group<Type, Types...> { virtual ~Delegate() {} };

                groupStorageMap.emplace(Group<Type, Types...>::groupId, static_cast<Storage*>(static_cast<Delegate*>(group)));
            }

            template <typename Type, typename... Types>
            Group<Type, Types...>* retrieveGroup() const
            {
                LOG_THIS_MEMBER("Component Registry");

                struct Delegate : public Storage, public Group<Type, Types...> { virtual ~Delegate() {} };

                return static_cast<Group<Type, Types...>*>(static_cast<Delegate*>(groupStorageMap.at(Group<Type, Types...>::groupId)));
            }

        private:
            std::unordered_map<_unique_id, Storage*> componentStorageMap;
            std::unordered_map<_unique_id, Storage*> groupStorageMap;
        };

        template <class Type>
        struct Ref 
        {
            // Take an unique id only to have the same signature as Own object
            Ref(_unique_id) { LOG_THIS_MEMBER("Ref"); }

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
            inline Type* internalCreateComponent(Entity& entity, Args&&... args)
            {
                LOG_THIS_MEMBER("Ref");

                return ref->internalCreateComponent(entity, std::forward<Args>(args)...);
            }

            inline void internalRemoveComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("Ref");

                ref->internalRemoveComponent(entity);
            }

            inline typename ComponentSet<Type>::ComponentSetList view() const
            {
                LOG_THIS_MEMBER("Ref");

                return ref->view();
            }

            inline const _unique_id& getComponentId() const { LOG_THIS_MEMBER("Ref"); return Type::componentId; }

            Own<Type> *ref;
        };

        template<class Type>
        struct Own : public Ref<Type>
        {
            Own(_unique_id id) : Ref<Type>(this)
            {
                LOG_THIS_MEMBER("Own");

                LOG_INFO("Own", "Type: " + std::string(typeid(Type).name()) + " get the id: " + std::to_string(id));

                // Todo check if Type::componentId is not != 0 but it should never happen
                Type::componentId = id;
            }

            virtual ~Own() { LOG_THIS_MEMBER("Own"); }

            void setRegistry(ComponentRegistry* registry)
            {
                LOG_THIS_MEMBER("Own");

                registry->store<Type>(this);
            }

            template <typename... Args>
            inline Type* internalCreateComponent(Entity& entity, Args&&... args)
            {
                LOG_THIS_MEMBER("Own");

                // Create a new component and store it in a sparse set along with the entity id using it
                return components.addComponent(entity, std::forward<Args>(args)...);
            }

            inline void internalRemoveComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("Own");

                components.removeComponent(entity);

                // TODO
                // entity.componentList.erase(Type::componentId);
            }

            inline typename ComponentSet<Type>::ComponentSetList view() const
            {
                LOG_THIS_MEMBER("Own");

                return components.viewComponents();
            }

            inline const _unique_id& getComponentId() const { LOG_THIS_MEMBER("Own"); return Type::componentId; }

            ComponentSet<Type> components;
        };
    }
}