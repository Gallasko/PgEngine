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

        void resize(int width, int height);

        void show();

        int exec();

    private:
        pg::Window window;
    };

}


#endif