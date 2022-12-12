#pragma once

#include <string>

#include "entity.h"
#include "uniqueid.h"

#include "logger.h"

namespace pg
{
    template <class T>struct tag{using type=T;};

    struct Component : public Entity
    {
    public:
        Component(const std::string& name) : Entity(), name(name) { LOG_THIS_MEMBER("Component"); }
        Component() : name("") { LOG_THIS_MEMBER("Component"); }

        // Component(Component& mE)              = default;
        // Component(Component&& mE)             = default;
        // Component& operator=(Component& mE)   = default;
        // Component& operator=(Component&& mE)  = default;

        // Todo move the naming of the component inside of the system that own it(?) or in the ecs(?) instead of here !
        // Todo remove
        std::string name;

    protected:
        ~Component() { LOG_THIS_MEMBER("Component"); }
    };
}