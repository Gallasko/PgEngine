#pragma once

#include "uniqueid.h"
#include "sparseset.h"

namespace pg
{
    namespace ecs
    {
        // Type forwarding
        class ComponentRegistry;

/**
        SparseSet* smallestSet(SparseSet* set1, SparseSet* set2)
        {
            return set1->nbElements() < set2->nbElements() ? set1 : set2;
        }

        template <class... Sets>
        SparseSet* smallestSet(SparseSet* set1, SparseSet* set2, Sets*... setN)
        {
            return set1->nbElements() < set2->nbElements() ? smallestSet(set1, setN...) : smallestSet(set2, setN...);
        }
*/

        template <typename Type, typename... Types>
        struct Group
        {
            Group(){}
            Group(_unique_id id) { Group<Type, Types...>::groupId = id; }

            void setRegistry(ComponentRegistry* registry)
            {
                registry->store<Type, Types...>(this);
            }

/**
            void process()
            {
                auto set = smallestSet(registry->retrieve<Type>()->view<Type>(), registry->retrieve<Types...>);
            }
*/
            static _unique_id groupId;
        };

        template <typename Type, typename... Types>
    	_unique_id Group<Type, Types...>::groupId = 0;
    }
}