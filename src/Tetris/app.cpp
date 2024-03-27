#include "app.h"

#include "logger.h"

#include "Audio/audiosystem.h"

#include "Systems/basicsystems.h"

#include "tetromino.h"

#include "2D/simple2dobject.h"
#include "2D/texture.h"

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

std::thread *initThread;
pg::Window *mainWindow = nullptr;
std::atomic<bool> initialized = {false};
bool init = false;
bool running = true;

void initWindow(const std::string& appName)
{
    mainWindow = new pg::Window(appName);

    LOG_INFO(DOM, "Window init...");

    // Todo if init failed exit app !

    // mainWindow->init(300, 700, false);

    // LOG_INFO(DOM, "Window init done !");

    // LOG_INFO(DOM, "Initializing engine ...");

    // mainWindow->initEngine();

    // auto& ecs = mainWindow->ecs;

    // // ecs.createSystem<FpsSystem>();

    // ecs.createSystem<GameCanvas>();

    initialized = true;

    // while (running)
    // {
    //     SDL_Event event;

    //     while (SDL_PollEvent(&event))
    //     {
    //         printf("Got a sdl event...\n");
    //         mainWindow->processEvents(event);
    //     }
    // }
}

void mainloop()
{    
    if (not initialized.load())
        return;

    if (not init)
    {
        printf("Joining thread...\n");
        initThread->join();
        printf("Thread joined...\n");
        init = true;

        mainWindow->init(300, 700, false);

        printf("Window init done !");

        printf("Initializing engine ...");

        mainWindow->initEngine();

        printf("Engine initialized ...");

        // mainWindow->ecs.createSystem<GameCanvas>();

        makeSentence(&mainWindow->ecs, 0, 0, {"Hello World"});

        makeSentence(&mainWindow->ecs, 200, 150, {"Hi yours truly"});

        // auto tex = makeUiTexture(&mainWindow->ecs, 296, 197, "menu");

        // tex.get<UiComponent>()->setY(250);

        // auto tex2 = makeUiTexture(&mainWindow->ecs, 143, 73, "font");

        // tex2.get<UiComponent>()->setY(500);

        printf("Canvas initialized ...");

        mainWindow->ecs.start();

        mainWindow->render();

        mainWindow->resize(600, 700);

        printf("Engine initialized");
    }

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        mainWindow->processEvents(event);
    }

    mainWindow->render();

    if (mainWindow->requestQuit())
        std::terminate();
}

int TetrisApp::exec()
{   
#ifdef __EMSCRIPTEN__
    printf("Start init thread...\n");
    initThread = new std::thread(initWindow, appName);
    printf("Detach init thread...\n");

    emscripten_set_main_loop(mainloop, 0, 1);

#else
    LOG_THIS_MEMBER(DOM);

    initWindow(appName);

    mainWindow->init(600, 500, false);

    LOG_INFO(DOM, "Window init done !");

    LOG_INFO(DOM, "Initializing engine ...");

    // mainWindow->audioSystem = ecs.createSystem<AudioSystem>();

    mainWindow->initEngine();

    mainWindow->ecs.createSystem<GameCanvas>();

    // auto testRect = makeSimple2DShape(&mainWindow->ecs, Shape2D::Square, 50, 50, {255.0f, 0.0f, 0.0f});

    // auto tex = makeUiTexture(&mainWindow->ecs, 296, 197, "menu");

    // tex.get<UiComponent>()->setY(250);

    makeSentence(&mainWindow->ecs, 200, 120, {"I am me"});

    auto rem = makeSentence(&mainWindow->ecs, 0, 0, {"Hello World!"});

    makeSentence(&mainWindow->ecs, 0, 120, {"Hola ?!"});

    auto test = makeSentence(&mainWindow->ecs, 200, 150, {"Hi yours truly"});

    test.get<SentenceText>()->setText("I am me");

    mainWindow->ecs.removeEntity(rem.entity);

    makeSentence(&mainWindow->ecs, 200, 180, {"I am me"});

    // auto sys = mainWindow->ecs.getSystem<SentenceSystem>();

    LOG_INFO(DOM, "Initializing engine done !");

    LOG_INFO(DOM, "Starting SDL event loop, waiting for events...");

    mainWindow->ecs.start();

    mainWindow->resize(600, 500);

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            mainWindow->processEvents(event);
        }

        mainWindow->render();

        if (mainWindow->requestQuit())
            break;
    }

    delete mainWindow;
#endif

    return 0;
}
