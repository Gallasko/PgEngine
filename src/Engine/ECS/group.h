#pragma once

#include "uniqueid.h"

namespace pg
{
    namespace ecs
    {
        // Type forwarding
        class ComponentRegistry;

        template <typename Type, typename... Types>
        struct Group
        {
            Group(){}
            Group(_unique_id id) { Group<Type, Types...>::groupId = id; }

            void setRegistry(ComponentRegistry* registry)
            {
                registry->store<Type, Types...>(this);
            }

            static _unique_id groupId;
        };

        template <typename Type, typename... Types>
    	_unique_id Group<Type, Types...>::groupId = 0;
    }
}