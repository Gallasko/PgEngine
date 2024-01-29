#include "app.h"

#include "logger.h"

#include "tetromino.h"

using namespace pg;

namespace
{
    static const char* const DOM = "Tetris clone app";
}

TetrisApp::TetrisApp(const std::string& appName) : window(appName)
{
    LOG_THIS_MEMBER(DOM);
}

TetrisApp::~TetrisApp()
{
    LOG_THIS_MEMBER(DOM);
}

int TetrisApp::exec()
{   
    LOG_THIS_MEMBER(DOM);

    LOG_INFO(DOM, "Window init...");

    // Todo if init failed exit app !

    window.init(300, 300, false);

    LOG_INFO(DOM, "Window init done !");

    LOG_INFO(DOM, "Initializing engine ...");

    window.initEngine();

    auto& ecs = window.ecs;

    ecs.createSystem<GameCanvas>();

    LOG_INFO(DOM, "Initializing engine done !");

    LOG_INFO(DOM, "Starting SDL event loop, waiting for events...");

    window.resize(300, 300);

    while (true)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            window.processEvents(event);
        }

        window.render();

        if (window.requestQuit())
            return 0;
    }

    return 0;
}
