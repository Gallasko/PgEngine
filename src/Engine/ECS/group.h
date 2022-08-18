#pragma once

#include "uniqueid.h"
#include "sparseset.h"

namespace pg
{
    namespace ecs
    {
        // Type forwarding
        class ComponentRegistry;

        SparseSet* smallestSet(SparseSet* set1, SparseSet* set2)
        {
            return set1->nbElements() < set2->nbElements() ? set1 : set2;
        }

        template <class... Sets>
        SparseSet* smallestSet(SparseSet* set1, SparseSet* set2, Sets*... setN)
        {
            return set1->nbElements() < set2->nbElements() ? smallestSet(set1, setN...) : smallestSet(set2, setN...);
        }

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
            Group(){}
            Group(_unique_id id) { Group<Type, Types...>::groupId = id; }

            void setRegistry(ComponentRegistry* registry)
            {
                registry->store<Type, Types...>(this);
            }

            void process()
            {
                auto set = smallestSet(registry->retrieve<Type>()->view<Type>(), registry->retrieve<Types>...()->view<Types>...());
            }
            
            ComponentSet<GroupElement<Type, Types...>> elements;

            static _unique_id groupId;
        };

        template <typename Type, typename... Types>
    	_unique_id Group<Type, Types...>::groupId = 0;
    }
}