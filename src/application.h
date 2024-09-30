#ifndef APPLICATION_H
#define APPLICATION_H

#include "window.h"

class EditorApp
{
public:
    EditorApp(const std::string& appName);
    ~EditorApp();

    int exec();

private:
    std::string appName;
};

#endif