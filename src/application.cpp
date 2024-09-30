#include "application.h"

#include "logger.h"

#include "Systems/basicsystems.h"
#include "Systems/oneventcomponent.h"

#include "Gui/contextmenu.h"
#include "Gui/inspector.h"

#include "Scene/scenemanager.h"

#include "UI/ttftext.h"

using namespace pg;
using namespace editor;

namespace
{
    static const char* const DOM = "Editor app";
}

struct EntityFinder : public System<Listener<OnMouseClick>, Ref<UiComponent>, Ref<SceneElement>, InitSys>
{
    virtual void init() override
    {
        registerGroup<UiComponent, SceneElement>();
    }

    virtual void onEvent(const OnMouseClick& event) override
    {
        for (const auto& elem : viewGroup<UiComponent, SceneElement>())
        {
            auto ui = elem->get<UiComponent>();

            if (ui->inClipBound(event.pos.x, event.pos.y))
            {
                LOG_INFO(DOM, "Clicked on entity: " << ui.entityId);

                ecsRef->sendEvent(InspectEvent{elem->entity});
                return;
            }
        }
    }
};

EditorApp::EditorApp(const std::string& appName) : appName(appName)
{
    LOG_THIS_MEMBER(DOM);
}

EditorApp::~EditorApp()
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

    // mainWindow->ecs.createSystem<FlagOwner>();

    // mainWindow->ecs.createSystem<CollisionSystem>();
    mainWindow->ecs.createSystem<MoveToSystem>();

    mainWindow->ecs.createSystem<ContextMenu>();

    mainWindow->ecs.createSystem<InspectorSystem>();

    mainWindow->ecs.createSystem<TTFTextSystem>(mainWindow->masterRenderer);

    mainWindow->ecs.succeed<InspectorSystem, ListViewSystem>();

    auto ent = mainWindow->ecs.createEntity();

    std::function callback = [](const OnMouseClick& event) {
        if (event.button == SDL_BUTTON_RIGHT)
        {
            mainWindow->ecs.sendEvent(ShowContextMenu{mainWindow->getInputHandler()});
        }        
    };

    mainWindow->ecs.attach<OnEventComponent>(ent, callback);

    mainWindow->ecs.createSystem<EntityFinder>();

    // makeTTFText(&mainWindow->ecs, 100, 100, "res/font/Inter/static/Inter_28pt-Black.ttf", "Hello world", 48);

    // mainWindow->ecs.createSystem<Texture2DAnimatorSystem>();

    // mainWindow->ecs.succeed<Texture2DAnimatorSystem, Texture2DComponentSystem>();

    // mainWindow->ecs.succeed<MoveToSystem, CollisionSystem>();
    
    mainWindow->ecs.start();

    mainWindow->render();

    mainWindow->resize(820, 640);

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
            EM_ASM(
                FS.syncfs(false, function (err) {
                    assert(!err);
                });
            );
#endif

    if (mainWindow->requestQuit())
    {
        LOG_ERROR("Window", "RequestQuit");
        std::terminate();
    }
}

int EditorApp::exec()
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

    // Todo Add this !
    // void func() {
    // ..
    // EM_ASM({
    //     FS.syncfs(.., function(err) {
    //     Module._continue();
    //     });
    // });
    // // might want to pause the main loop here, if one is running
    // }

    // extern "C" {
    // void EMSCRIPTEN_KEEPALIVE continue() {
    // // data is now here, continue and use it
    // // can resume main loop, if you are using one
    // }

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
