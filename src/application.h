#ifndef APPLICATION_H
#define APPLICATION_H

#include "Event/eventloop.h"

#include "window.h"

namespace pg
{

    class PgApplication
    {
    public:
        PgApplication(const std::string& appName);
        ~PgApplication();

        void resize(int width, int height);

        void show();

        int exec();

    private:
        EventLoop eventLoop;

        pg::Window window;
    };

}


#endif