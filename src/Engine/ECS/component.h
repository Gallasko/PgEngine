#pragma once

#include <string>

#include "uniqueid.h"

namespace pg
{
    namespace ecs
    {
        template<class T>struct tag{using type=T;};

        struct AbstractComponent
        {
            // Todo remove
            std::string name;

            // Todo is this really necessary
            virtual ~AbstractComponent() {}
            // virtual const _unique_id& getComponentId() const = 0;
        };

        template<typename Comp>
        struct Component : public AbstractComponent
        {
            Component(const std::string& name) : AbstractComponent() { this->name = name; }

            Component() : Component(typeid(Comp).name()) { }

            virtual ~Component() {}

            // virtual const _unique_id& getComponentId() const override { return Component::componentId; }

            static _unique_id componentId;   
        };

        template<typename Comp>
        _unique_id Component<Comp>::componentId = 0;

        template<typename Comp>
        struct NamedComponent : public Component<Comp>
        {
            NamedComponent(const std::string& name) : Component<Comp>(name) {}

            virtual ~NamedComponent() {}
        };
    }
}