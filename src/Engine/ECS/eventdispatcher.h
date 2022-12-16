#pragma once

#include "system.h"

namespace pg
{
    struct Event
    {
        Event() : Component("Event") {}
    };

    class EventDispatcher : public System<Own<Event>>
    {

    };
}