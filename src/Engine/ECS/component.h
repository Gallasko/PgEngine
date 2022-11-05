#pragma once

#include <string>

#include "entity.h"
#include "uniqueid.h"

#include "logger.h"

namespace pg
{
    namespace ecs
    {
        template<class T>struct tag{using type=T;};

        struct AbstractComponent : virtual public Entity
        {
            AbstractComponent() : Entity(generateId()) { LOG_THIS_MEMBER("Abstract Component"); }

            // Todo is this really necessary
            virtual ~AbstractComponent() { LOG_THIS_MEMBER("Abstract Component"); }
            // virtual const _unique_id& getComponentId() const = 0;
        
            // Todo remove
            std::string name;

            // todo add on remove and on removed variable
            // bool detached = false;
            // virtual ~AbstractComponent() { if(detached) ecs->detach(id, this); }
        };

        template<typename Comp>
        struct Component : public AbstractComponent
        {
            Component(const std::string& name) : AbstractComponent() { LOG_THIS_MEMBER("Component"); this->name = name; }

            Component() : Component(typeid(Comp).name()) { LOG_THIS_MEMBER("Component"); }

            virtual ~Component() { LOG_THIS_MEMBER("Component"); }

            // virtual const _unique_id& getComponentId() const override { return Component::componentId; }

            static _unique_id componentId;   
        };

        template<typename Comp>
        _unique_id Component<Comp>::componentId = 0;

        template<typename Comp>
        struct NamedComponent : public Component<Comp>
        {
            NamedComponent(const std::string& name) : Component<Comp>(name) { LOG_THIS_MEMBER("Component"); }

            virtual ~NamedComponent() { LOG_THIS_MEMBER("Component"); }
        };

        template<typename Derived, typename Base>
        struct IsA : public Base, public Component<Derived>
        {
            using Component<Derived>::componentId;

            IsA() { LOG_THIS_MEMBER("Component"); }
            
            template<typename... Args>
            IsA(const Args&... args) : Base(args...) { LOG_THIS_MEMBER("Component"); }

            virtual ~IsA() { LOG_THIS_MEMBER("Component"); }
        };
    }
}