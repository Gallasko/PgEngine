#pragma once

#include "system.h"

#include "logger.h"

namespace pg
{
    // Todo desactivate logger system in prod (only enable it in debug mode)

    struct LogInfoEvent
    {
        LogInfoEvent(const std::string& scope, const std::string& log) : scope(scope), log(log) { }

        std::string scope;
        std::string log;
    };

    struct TerminalLogSystem : public System<Listener<LogInfoEvent>, StoragePolicy>
    {
        virtual void onEvent(const LogInfoEvent& event) override { LOG_THIS_MEMBER("TerminalLogSystem"); LOG_INFO(event.scope, event.log); }
    };

}