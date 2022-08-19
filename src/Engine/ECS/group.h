#pragma once

#include "uniqueid.h"
#include "sparseset.h"

#include "componentregistry.h"

#include "logger.h"

#include <iostream>

namespace pg
{
    namespace ecs
    {
        // Type forwarding
        class ComponentRegistry;

        template <typename Type>
        struct Getter
        {
            Getter(Type* value) : value(value) { LOG_THIS_MEMBER("Ecs Group"); }

            Type* get() const { LOG_THIS_MEMBER("Ecs Group"); return value; }

            Type* value; 
        };

        template <typename... Types>
        struct GroupElement : public Getter<Types>...
        {
            GroupElement(Types*... values) : Getter<Types>(values)... { LOG_THIS_MEMBER("Ecs Group"); }

            template <typename Type>
            Type* get() const { LOG_THIS_MEMBER("Ecs Group"); return static_cast<Getter<Type>*>(this)->get(); }

            Entity* entity;
        };

        template <typename Type, typename... Types>
        struct Group
        {
            Group(_unique_id id) { LOG_THIS_MEMBER("Ecs Group"); Group<Type, Types...>::groupId = id; }
            virtual ~Group() { LOG_THIS_MEMBER("Ecs Group"); }

            void setRegistry(ComponentRegistry* registry)
            {
                LOG_THIS_MEMBER("Ecs Group");

                this->registry = registry;
                registry->storeGroup<Type, Types...>(this);
            }

            void process()
            {
                LOG_THIS_MEMBER("Ecs Group");

                const SparseSet& set = smallestSet(registry->retrieve<Type>()->components, registry->retrieve<Types...>()->components);

                LOG_INFO("Ecs Group", "Smallest set has: " + std::to_string(set.nbElements()) + " elements");
            }

            inline const SparseSet& smallestSet(const SparseSet& set1, const SparseSet& set2) const
            {
                LOG_THIS_MEMBER("Ecs Group");

                return set1.nbElements() < set2.nbElements() ? set1 : set2;
            }

            template <typename Set, typename... Sets>
            inline const SparseSet& smallestSet(const SparseSet& set1, const SparseSet& set2, const Set& setN, const Sets&... sets) const
            {
                LOG_THIS_MEMBER("Ecs Group");

                return set1.nbElements() < set2.nbElements() ? smallestSet(set1, setN, sets...) : smallestSet(set2, setN, sets...);
            }

            ComponentRegistry* registry;
            ComponentSet<GroupElement<Type, Types...>> elements;

            static _unique_id groupId;
        };

        template <typename Type, typename... Types>
    	_unique_id Group<Type, Types...>::groupId = 0;
    }
}