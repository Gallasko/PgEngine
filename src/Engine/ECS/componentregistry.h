#pragma once

#include <memory>
#include <unordered_map>

#include "sparseset.h"

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
                struct Delegate : public Storage, public Own<Type> { };

                storageMap[typeid(Type).name()] = static_cast<Storage*>(static_cast<Delegate*>(owner));
            }

            template <typename Type>
            Own<Type>* retrieve() const
            {
                struct Delegate : public Storage, public Own<Type> { };

                return static_cast<Own<Type>*>(static_cast<Delegate*>(storageMap.at(typeid(Type).name())));
            }

        private:
            std::unordered_map<std::string, Storage*> storageMap;
        };

        template <typename Type>
        struct Ref 
        {
            Ref()
            {
            }

            Ref(Own<Type> *ref) : ref(ref) {}

            void setRegistry(ComponentRegistry* registry)
            {
                ref = registry->retrieve<Type>();
            }

            template <typename... Args>
            Type* internalCreateComponent(_entityId id, const Args&... args)
            {
                return ref->internalCreateComponent(id, args...);
            }

            SparseSet::SparseSetList<Type> view() const
            {
                return ref->view();
            }

            Own<Type> *ref;
        };

        template<typename Type>
        struct Own : public Ref<Type>
        {
            Own() : Ref<Type>(this)
            {
            }

            void setRegistry(ComponentRegistry* registry)
            {
                registry->store<Type>(this);
            }

            template <typename... Args>
            Type* internalCreateComponent(_entityId id, const Args&... args)
            {
                auto comp = new Type(args...);
                components.add(id, comp);
                return comp;
            }

            SparseSet::SparseSetList<Type> view() const
            {
                return components.view<Type>();
            }

            SparseSet components;
        };
    }
}