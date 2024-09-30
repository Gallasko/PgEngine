#ifndef APPLICATION_H
#define APPLICATION_H

#include "window.h"

class GmtkApp
{
public:
    GmtkApp(const std::string& appName);
    ~GmtkApp();

    int exec();

private:
    std::string appName;
};

#endif