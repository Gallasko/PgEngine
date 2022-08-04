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
            static const std::unique_ptr<ComponentRegistry>& instance() { static std::unique_ptr<ComponentRegistry> ins(new ComponentRegistry()); return ins; }

            template <typename Type>
            static void store(Own<Type>* owner)
            {
                struct Delegate : public Storage, public Own<Type> { };

                ComponentRegistry::instance()->storageMap[typeid(Type).name()] = static_cast<Storage*>(static_cast<Delegate*>(owner));
            }

            template <typename Type>
            static Own<Type>* retrieve()
            {
                struct Delegate : public Storage, public Own<Type> { };

                return static_cast<Own<Type>*>(static_cast<Delegate*>(ComponentRegistry::instance()->storageMap[typeid(Type).name()]));
            }

        private:
            std::unordered_map<std::string, Storage*> storageMap;
        };

        template<typename Type>
        struct Own
        {
            Own()
            {
                ComponentRegistry::store<Type>(this);
            }

            template <typename... Args>
            Type* internalCreateComponent(_entityId id, const Args&... args)
            {
                auto comp = new Type(args...);
                return static_cast<Type*>(components.add(id, comp));
            }

            SparseSet components;
        };

        template <typename Type>
        struct Ref 
        {
            Ref()
            {
                ref = ComponentRegistry::retrieve<Type>();
            }

            template <typename... Args>
            Type* internalCreateComponent(_entityId id, const Args&... args)
            {
                return ref->internalCreateComponent(id, args...);
            }

            Own<Type> *ref;
        };
    }
}