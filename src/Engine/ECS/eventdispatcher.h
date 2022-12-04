#pragma once

#include "system.h"

namespace pg::ecs
{
    struct Event : public NamedComponent<Event>
    {
        Event() : NamedComponent<Event>("Event") {}
    };

    class EventDispatcher : public System<Own<Event>>
    {

    };
}