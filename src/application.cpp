#include "application.h"

#include "logger.h"

namespace pg
{

    PgApplication::PgApplication(const std::string& appName) : window(appName)
    {
        eventLoop.start();

        LOG_INFO(DOM, "Event Loop started, waiting for events...");
    }

    PgApplication::~PgApplication()
    {
        eventLoop.stop();
    }

    void PgApplication::resize(int width, int height)
    {   

    }

    void PgApplication::show()
    {   

    }

    int PgApplication::exec()
    {   
        std::string str;
        // KeyEvent* event;

        do
        {
            // std::cin >> str;
            // event = new KeyEvent(str);
            // eventLoop.queueEvent(event);

            if(str == "tOn")
            {
                // std::thread t (&PgApplication::timer, this);

                // t.detach();
            }
            // else if(str == "tOff")
                // timerRunning = false;

        } while (str != "quit");

        return eventLoop.wait();
    }

}