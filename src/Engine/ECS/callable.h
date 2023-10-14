#pragma once

#include <tuple>

#include "entitysystem.h"

namespace pg
{
    struct AbstractCallable
    {
        virtual ~AbstractCallable() {}

        virtual void call(EntitySystem* const ecsRef) = 0;
    };

    typedef std::shared_ptr<AbstractCallable> CallablePtr;

    template<typename Event, typename... Types>
    struct CallableEvent : public AbstractCallable
    {
        CallableEvent(Types&&... args) : event(std::forward<Types>(args)...) {}
        virtual ~CallableEvent() {}

        inline virtual void call(EntitySystem* const ecsRef) override { if(ecsRef) ecsRef->sendEvent(event); }

        Event event;
    };

    template<typename Event, typename... Types>
    inline std::shared_ptr<CallableEvent<Event, Types...>> makeCallable(Types&&... args) { return std::make_shared<CallableEvent<Event, Types...>>(std::forward<Types>(args)...); }

    // Todo make a scriptCallable
}