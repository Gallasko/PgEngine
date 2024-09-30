#include "app.h"

#include "logger.h"

#include "Audio/audiosystem.h"

#include "Systems/basicsystems.h"

#include "2D/simple2dobject.h"
#include "2D/texture.h"

#include "Scene/scenemanager.h"

#include "Systems/oneventcomponent.h"

#include "keyconfig.h"

#include "2D/collisionsystem.h"

#include "titlescreen.h"
#include "game.h"

using namespace pg;

namespace
{
    static const char* const DOM = "Gmtk app";
}

GmtkApp::GmtkApp(const std::string& appName) : appName(appName)
{
    LOG_THIS_MEMBER(DOM);
}

GmtkApp::~GmtkApp()
{
    LOG_THIS_MEMBER(DOM);
}

struct FlagOwner : public System<Own<PlayerFlag>, Own<EnemyFlag>, Own<AllyBulletFlag>, StoragePolicy> {};

std::thread *initThread;
pg::Window *mainWindow = nullptr;
std::atomic<bool> initialized = {false};
bool init = false;
bool running = true;

void initWindow(const std::string& appName)
{
#ifdef __EMSCRIPTEN__
    mainWindow = new pg::Window(appName, "/save/savedData.sz");
#else
    mainWindow = new pg::Window(appName);
#endif

    LOG_INFO(DOM, "Window init...");

    initialized = true;
}

void initGame()
{
    printf("Initializing engine ...\n");

    mainWindow->initEngine();

    printf("Engine initialized ...\n");

    // mainWindow->ecs.createSystem<ConfiguredKeySystem<TetrisConfig>>(scancodeMap);

    // mainWindow->ecs.createSystem<FpsSystem>();

    mainWindow->ecs.createSystem<FlagOwner>();

    mainWindow->ecs.createSystem<CollisionSystem>();
    mainWindow->ecs.createSystem<MoveToSystem>();

    mainWindow->ecs.createSystem<Texture2DAnimatorSystem>();

    mainWindow->ecs.succeed<Texture2DAnimatorSystem, Texture2DComponentSystem>();

    mainWindow->ecs.succeed<MoveToSystem, CollisionSystem>();
    
    mainWindow->ecs.start();

    mainWindow->render();

    mainWindow->resize(820, 640);

    // mainWindow->ecs.createSystem<TestSceneSystem>();


    // mainWindow->ecs.createSystem<GameCanvas>();

    // mainWindow->ecs.getSystem<SceneElementSystem>()->loadSystemScene<TestScene>();
    // mainWindow->ecs.getSystem<SceneElementSystem>()->loadSystemScene<GameScene>();
    mainWindow->ecs.getSystem<SceneElementSystem>()->loadSystemScene<TitleScreen>();

    mainWindow->ecs.sendEvent(StartAudio{"res/audio/ost.ogg", -1});

    // makeSentence(&mainWindow->ecs, 200, 120, {"I am me"});

    // auto rem = makeSentence(&mainWindow->ecs, 0, 0, {"Hello World!"});

    printf("Canvas initialized ...\n");

    // mainWindow->ecs.start();

    // mainWindow->render();

    // mainWindow->resize(820, 600);

    printf("Engine initialized\n");
}

void mainloop(void* arg)
{    
    if (not initialized.load())
        return;

    if (not init)
    {
        if (initThread)
        {
            printf("Joining thread...\n");

            initThread->join();

            delete initThread;

            printf("Thread joined...\n");
        }

        init = true;

        mainWindow->init(820, 640, false, static_cast<SDL_Window*>(arg));

        printf("Window init done !\n");

        initGame();
    }

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        mainWindow->processEvents(event);
    }

    mainWindow->render();

// // Todo don't do this every frame but only when something is written to emscripten filesystem
#ifdef __EMSCRIPTEN__
//         static auto start = std::chrono::steady_clock::now();

//         static auto end = std::chrono::steady_clock::now();

//         end = std::chrono::steady_clock::now();

//         if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() >= 1)
//         {
            EM_ASM(
                FS.syncfs(false, function (err) {
                    assert(!err);
                });
            );

//             start = end;
//         }
#endif

    if (mainWindow->requestQuit())
    {
        LOG_ERROR("Window", "RequestQuit");
        std::terminate();
    }
}

int GmtkApp::exec()
{
    // pg::Logger::registerSink<pg::TerminalSink>(true);
#ifdef __EMSCRIPTEN__
    // Todo only do this once !!!
    // Make this save folder configurable
    EM_ASM(
        // Make a directory other than '/'
        FS.mkdir('/save');
        // Then mount with IDBFS type
        FS.mount(IDBFS, {autoPersist: true}, '/save');           
        // Then sync

        FS.syncfs(true, function (err) {
            // Error
        });
    );

    printf("Start init thread...\n");
    initThread = new std::thread(initWindow, appName);
    printf("Detach init thread...\n");

    SDL_Window *pWindow = 
        SDL_CreateWindow("Hello Triangle Minimal", 
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         820, 640, 
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    // initThread->join();

    emscripten_set_main_loop_arg(mainloop, pWindow, 0, 1);
    // emscripten_set_main_loop(mainloop, 0, 1);

#else
    LOG_THIS_MEMBER(DOM);

    initWindow(appName);

    mainWindow->init(820, 640, false);

    LOG_INFO(DOM, "Window init done !");

    initGame();

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
