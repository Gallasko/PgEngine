#pragma once

#include <vector>

#include "component.h"
#include "uniqueid.h"

namespace pg
{
    namespace ecs
    {
        struct OwnedComponent
        {
            template <typename Comp, typename... Args>
            Component* create(const Args&... args);

            void remove(Component* component);

            std::vector<Component* > components;
        };

        template <typename Type>
        struct Own : public OwnedComponents
        {
            Own()

            
        };

        struct ComponentHolder
        {

        };

        template <typename Type>
        struct Ref : public ComponentHolder
        {

        };

        struct AbstractSystem
        {
        };

        struct System : public AbstractSystem
        {
        };
    }
}