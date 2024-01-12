#ifndef APPLICATION_H
#define APPLICATION_H

#include "window.h"

namespace pg
{

    class PgApplication
    {
    public:
        PgApplication(const std::string& appName);
        ~PgApplication();

        int exec();

    private:
        pg::Window window;
    };

}


#endif