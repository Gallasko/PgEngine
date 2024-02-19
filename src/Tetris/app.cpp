#include "app.h"

#include "logger.h"

#include "Audio/audiosystem.h"

#include "Systems/basicsystems.h"

#include "tetromino.h"

using namespace pg;

namespace
{
    static const char* const DOM = "Tetris clone app";
}

TetrisApp::TetrisApp(const std::string& appName) : appName(appName)
{
    LOG_THIS_MEMBER(DOM);
}

TetrisApp::~TetrisApp()
{
    LOG_THIS_MEMBER(DOM);
}

#ifdef __EMSCRIPTEN__

pg::Window *mainWindow = nullptr;

void mainloop()
{
    LOG_INFO(DOM, "Main loop");

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        mainWindow->processEvents(event);
    }

    mainWindow->render();

    // if (mainWindow->requestQuit())
    //     return 0;
}

#endif

int TetrisApp::exec()
{   
#ifdef __EMSCRIPTEN__

    mainWindow = new pg::Window(appName);

    LOG_INFO(DOM, "Window init...");

    // Todo if init failed exit app !

    mainWindow->init(300, 700, false);

    LOG_INFO(DOM, "Window init done !");

    LOG_INFO(DOM, "Initializing engine ...");

    mainWindow->initEngine();

    emscripten_set_main_loop(mainloop, 0, 1);

#else
    LOG_THIS_MEMBER(DOM);

    pg::Window window(appName);

    LOG_INFO(DOM, "Window init...");

    // Todo if init failed exit app !

    window.init(300, 700, false);

    LOG_INFO(DOM, "Window init done !");

    LOG_INFO(DOM, "Initializing engine ...");

    window.initEngine();

    // auto& ecs = window.ecs;

    // ecs.createSystem<FpsSystem>();

    // ecs.createSystem<GameCanvas>();

    // ecs.sendEvent(StartAudio{"res/audio/mainost.mp3"});

    LOG_INFO(DOM, "Initializing engine done !");

    LOG_INFO(DOM, "Starting SDL event loop, waiting for events...");

    window.resize(300, 700);

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
#endif

    return 0;
}
