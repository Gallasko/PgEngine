#pragma once

#include <string>

#include "uniqueid.h"

namespace pg
{
    namespace ecs
    {
        struct Component
        {
            Component(const std::string& name);

            template<typename Comp>
            Component() : Component(typeid(Comp).name()) { }
        };

        struct NamedComponent : public Component
        {
            NamedComponent(const std::string& name) : Component(name) {}
        };
    }
}