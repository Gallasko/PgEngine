#include "application.h"

#include "logger.h"

namespace
{
    static const char* const DOM = "PGApplication";
}

namespace pg
{
    PgApplication::PgApplication(const std::string& appName) : window(appName)
    {
        LOG_THIS_MEMBER(DOM);
    }

    PgApplication::~PgApplication()
    {
        LOG_THIS_MEMBER(DOM);
    }

    int PgApplication::exec()
    {   
        LOG_THIS_MEMBER(DOM);

        LOG_INFO(DOM, "Window init...");

        window.init(300, 300, false);

        LOG_INFO(DOM, "Window init done !");

        LOG_INFO(DOM, "Initializing engine ...");

        window.initEngine();

        LOG_INFO(DOM, "Initializing engine done !");

        LOG_INFO(DOM, "Starting SDL event loop, waiting for events...");

        window.resize(300, 300);

        while(true)
        {
            SDL_Event event;

            while(SDL_PollEvent(&event))
            {
                window.processEvents(event);
            }

            window.render();

            if(window.requestQuit())
                return 0;
        }

        return 0;
    }

}