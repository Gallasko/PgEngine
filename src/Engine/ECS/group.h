#pragma once

#include "uniqueid.h"
#include "sparseset.h"

namespace pg
{
    namespace ecs
    {
        // Type forwarding
        class ComponentRegistry;

        template <typename Type>
        struct Getter
        {
            Getter(Type* value) : value(value) {}

            Type* get() const { return value; }

            Type* value; 
        };

        template <typename... Types>
        struct GroupElement : public Getter<Types>...
        {
            GroupElement(Types*... values) : Getter<Types>(values)... {}

            template <typename Type>
            Type* get() const { return static_cast<Getter<Type>*>(this)->get(); }

            Entity* entity;
        };

        template <typename Type, typename... Types>
        struct Group
        {
            // Group(){}
            Group(_unique_id id) { Group<Type, Types...>::groupId = id; }

            void setRegistry(ComponentRegistry* registry);

            void process();

            inline const SparseSet& smallestSet(const SparseSet& set1, const SparseSet& set2) const
            {
                return set1.nbElements() < set2.nbElements() ? set1 : set2;
            }

            template <typename Set, typename... Sets>
            inline const SparseSet& smallestSet(const SparseSet& set1, const SparseSet& set2, const Set& setN, const Sets&... sets) const
            {
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