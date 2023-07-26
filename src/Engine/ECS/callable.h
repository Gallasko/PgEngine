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
    struct Callable : public AbstractCallable
    {
        Callable(Types&&... args) : event(std::forward<Types>(args)...) {}
        virtual ~Callable() {}

        inline virtual void call(EntitySystem* const ecsRef) override { if(ecsRef) ecsRef->sendEvent(event); }

        Event event;
    };

    template<typename Event, typename... Types>
    inline std::shared_ptr<Callable<Event, Types...>> makeCallable(Types&&... args) { return std::make_shared<Callable<Event, Types...>>(std::forward<Types>(args)...); }

    // Todo make a scriptCallable
}