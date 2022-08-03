#pragma once

#include <string>

#include "uniqueid.h"

namespace pg
{
    namespace ecs
    {
        template<class T>struct tag{using type=T;};

        struct Component
        {
            Component(const std::string& name);

            template<typename Comp>
            Component(const tag<Comp>&) : Component(typeid(Comp).name()) { }

            virtual ~Component() {}

            std::string name;
        };

        struct NamedComponent : public Component
        {
            NamedComponent(const std::string& name) : Component(name) {}

            virtual ~NamedComponent() {}
        };
    }
}