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

        void unsetRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Listener");

            registry->removeEventListener<Event>(this);
        }
    };

    template<>
    struct Listener<StandardEvent>
    {
        virtual void onEvent(const StandardEvent& event) = 0;

        void addListenerToStandardEvent(const std::string& name)
        {
            LOG_THIS_MEMBER("Listener");

            _stardardEventListenerList.push_back(name);

            __stardardEventRegistry->addStandardEventListener(name, this);
        }

        void setRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Listener");

            LOG_INFO("Listener", "Add standard listener");

            __stardardEventRegistry = registry;
        }

        void unsetRegistry(ComponentRegistry* registry)
        {
            LOG_THIS_MEMBER("Listener");

            for (auto eventName : _stardardEventListenerList)
            {
                registry->removeStandardEventListener(eventName, this);
            }
        }

        std::vector<std::string> _stardardEventListenerList;

        ComponentRegistry* __stardardEventRegistry;
    };
}