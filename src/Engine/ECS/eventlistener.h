#pragma once

#include "componentregistry.h"

#include "logger.h"

namespace pg
{
    template<typename Event>
    struct Listener
    {
        virtual void onEvent(const Event& event) = 0;

        void setRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Listener");

            registry->addEventListener<Event>(this);
        }
    };
}