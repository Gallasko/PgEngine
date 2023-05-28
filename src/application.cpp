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

    void PgApplication::resize(int width, int height)
    {   
        LOG_THIS_MEMBER(DOM);

    }

    void PgApplication::show()
    {   
        LOG_THIS_MEMBER(DOM);

    }

    int PgApplication::exec()
    {   
        LOG_THIS_MEMBER(DOM);

        LOG_INFO(DOM, "Window init...");

        window.init(200, 200, false);

        LOG_INFO(DOM, "Window init done !");

        LOG_INFO(DOM, "Starting SDL event loop, waiting for events...");

        for( ;; )
        {
            glClear( GL_COLOR_BUFFER_BIT );

            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_RESIZED) 
                        {
                            LOG_MILE(DOM, "MESSAGE: Resizing window... New width: " << event.window.data1 << ", new height: " << event.window.data2);

                            window.resize(event.window.data1, event.window.data2);
                        }
                        break;

                    case SDL_KEYUP:
                        if( event.key.keysym.sym == SDLK_ESCAPE )
                            return 0;
                        break;
                }
            }

            window.swapBuffer();
        }

        return 0;
    }

}