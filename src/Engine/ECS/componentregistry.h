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

        class ComponentRegistry
        {
            struct Storage {};

        public:
            template <typename Type>
            void store(Own<Type>* owner)
            {
                LOG_THIS_MEMBER("Component Registry");

                struct Delegate : public Storage, public Own<Type> { };

                storageMap[Type::componentId] = static_cast<Storage*>(static_cast<Delegate*>(owner));
            }

            template <typename Type>
            Own<Type>* retrieve() const
            {
                LOG_THIS_MEMBER("Component Registry");

                struct Delegate : public Storage, public Own<Type> { };

                return static_cast<Own<Type>*>(static_cast<Delegate*>(storageMap.at(Type::componentId)));
            }

        private:
            std::unordered_map<_unique_id, Storage*> storageMap;
        };

        template <typename Type>
        struct Ref 
        {
            // Take an unique id only to have the same signature as Own object
            Ref(_unique_id) { LOG_THIS_MEMBER("Ref"); }

            Ref(Own<Type> *ref) : ref(ref) { LOG_THIS_MEMBER("Ref"); }

            virtual ~Ref() { LOG_THIS_MEMBER("Ref"); }

            void setRegistry(ComponentRegistry* registry)
            {
                LOG_THIS_MEMBER("Ref");

                ref = registry->retrieve<Type>();
            }

            template <typename... Args>
            Type* internalCreateComponent(Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("Ref");

                return ref->internalCreateComponent(entity, args...);
            }

            void internalRemoveComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("Ref");

                ref->internalRemoveComponent(entity);
            }

            SparseSet::SparseSetList<Type> view() const
            {
                LOG_THIS_MEMBER("Ref");

                return ref->view();
            }

            const _unique_id& getComponentId() const { return Type::componentId; }

            Own<Type> *ref;
        };

        template<typename Type>
        struct Own : public Ref<Type>
        {
            Own(_unique_id id) : Ref<Type>(this)
            {
                LOG_THIS_MEMBER("Own");

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
            Type* internalCreateComponent(Entity& entity, const Args&... args)
            {
                LOG_THIS_MEMBER("Own");

                // Create a new component
                auto comp = new Type(args...);

                // Store it in a sparse set along with the entity id using it
                components.add(entity, comp);

                // Add the component to the entity component list for fast 
                entity.componentList[Type::componentId] = comp;

                // Return the component for possible modification
                return comp;
            }

            void internalRemoveComponent(Entity& entity)
            {
                LOG_THIS_MEMBER("Own");

                components.remove(entity);

                entity.componentList.erase(Type::componentId);
            }

            SparseSet::SparseSetList<Type> view() const
            {
                LOG_THIS_MEMBER("Own");

                return components.view<Type>();
            }

            const _unique_id& getComponentId() const { return Type::componentId; }

            SparseSet components;
        };
    }
}