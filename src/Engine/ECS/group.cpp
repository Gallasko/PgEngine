#include "group.h"

#include "componentregistry.h"

#include <iostream>

namespace pg
{
    namespace ecs
    {
        template <typename Type, typename... Types>
        void Group<Type, Types...>::setRegistry(ComponentRegistry* registry)
        {
            this->registry = registry;
            registry->storeGroup<Type, Types...>(this);
        }
        
        template <typename Type, typename... Types>
        void Group<Type, Types...>::process()
        {
            auto set = smallestSet(registry->retrieve<Type>()->components, registry->retrieve<Types...>()->components);

            std::cout << "Smallest set has: " << set.nbElements() << " elements" << std::endl;
        }
    }
}