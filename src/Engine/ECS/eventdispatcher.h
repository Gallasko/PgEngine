#pragma once

#include "system.h"

namespace pg::ecs
{
    struct Event : public Component
    {
        Event() : Component("Event") {}
    };

    class EventDispatcher : public System<Own<Event>>
    {

    };
}