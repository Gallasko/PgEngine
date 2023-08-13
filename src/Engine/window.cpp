#include "gl_debug.hpp"

#include "window.h"

#include <string>
#include <chrono>

#include "ECS/loggersystem.h"
#include "ECS/ecsmodule.h"
#include "logger.h"

#include "Renderer/renderer.h"
#include "Renderer/renderermodule.h"

#include "Input/input.h"

#include "UI/uisystem.h"
#include "UI/texture.h"
#include "UI/simple2dobject.h"
#include "UI/focusable.h"
#include "UI/textinput.h"
#include "UI/sentencesystem.h"

#include "Interpreter/pginterpreter.h"
#include "Interpreter/systemfunction.h"

#include "GameElements/Systems/basicsystems.h"

namespace
{
    static const char * const DOM = "Window";

    // Todo change and remove
    class TestPrint : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto v = args.front()->getElement();
            args.pop();

            std::cout << "[Interpreter]: " << v.toString() << std::endl;

            return nullptr;
        }
    };

    class CreateRectangle : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(2, 2);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto x = args.front()->getElement();
            args.pop();

            auto y = args.front()->getElement();
            args.pop();

            std::cout << "[Interpreter]: Creating rec at: (" << x.toString() << ", " << y.toString() << ")" << std::endl;

            auto rec = makeSimple2DShape(ecsRef, Shape2D::Square, 50, 50, {255.0f, 0.0f, 0.0f});
            auto recUi = rec.get<UiComponent>();

            if(x.isNumber() and y.isNumber())
            {
                recUi->setX(x.get<float>());
                recUi->setY(y.get<float>());
            }
            else
            {
                LOG_ERROR("CreateRectangle", "Cannot create a new rectangle values passed are not numbers");
            }

            return makeList(this, {{"x", static_cast<UiSize>(recUi->pos.x)}, {"y", static_cast<UiSize>(recUi->pos.y)}, {"w", recUi->width}, {"h", recUi->height}});
        }

        EntitySystem* ecsRef = nullptr;
    };

    class AddFilterScopeFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(std::shared_ptr<Logger::LogSink> sink)
        {
            setArity(2, 2);
            
            this->sink = sink;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto scope = args.front()->getElement();
            args.pop();

            if(not name.isLitteral() or not scope.isLitteral())
            {
                LOG_ERROR("AddFilterScopeFunction", "Cannot create apply a new filter on sink, values passed are not litterals");
                return nullptr;
            }

            sink->addFilter(name.toString(), new pg::Logger::LogSink::FilterScope(scope.toString()));

            return nullptr;
        }

        std::shared_ptr<Logger::LogSink> sink;
    };

    class AddFilterLevelFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(std::shared_ptr<Logger::LogSink> sink)
        {
            setArity(2, 2);
            
            this->sink = sink;
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto scope = args.front()->getElement();
            args.pop();

            if(not name.isLitteral() or not scope.isLitteral())
            {
                LOG_ERROR("AddFilterLevelFunction", "Cannot create apply a new filter on sink, values passed are not litterals");
                return nullptr;
            }

            pg::Logger::InfoLevel logLevel;

            if(scope.toString() == "log")
                logLevel = pg::Logger::InfoLevel::log;
            else if(scope.toString() == "info")
                logLevel = pg::Logger::InfoLevel::info;
            else if(scope.toString() == "mile")
                logLevel = pg::Logger::InfoLevel::mile;
            else if(scope.toString() == "test")
                logLevel = pg::Logger::InfoLevel::test;
            else if(scope.toString() == "error")
                logLevel = pg::Logger::InfoLevel::error;
            else
            {
                LOG_ERROR("AddFilterLevelFunction", "Trying to filter an unknown log level: " << scope.toString());
                return nullptr;
            }

            sink->addFilter(name.toString(), new pg::Logger::LogSink::FilterLogLevel(logLevel));

            return nullptr;
        }

        std::shared_ptr<Logger::LogSink> sink;
    };

    class OpenTextFileFunction : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto file = UniversalFileAccessor::openTextFile(name.toString());

            return makeVariable(file.data);
        }

        std::shared_ptr<Logger::LogSink> sink;
    };

    class OpenTextFolderFunction : public Function
    {
        using Function::Function;
    public:
        void setUp()
        {
            setArity(1, 1);
        }

        virtual ValuablePtr call(ValuableQueue& args) const override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto folder = UniversalFileAccessor::openTextFolder(name.toString());

            auto list = makeList(this, {});

            for(auto file : folder)
            {
                addToList(list, token, {file.filepath, file.data});
            }

            return list; 
        }

        std::shared_ptr<Logger::LogSink> sink;
    };

    struct LogModule : public SysModule
    {
        LogModule()
        {
            terminalSink = pg::Logger::registerSink<pg::TerminalSink>(true);
            
            addSystemFunction<TestPrint>("log");
            addSystemFunction<AddFilterScopeFunction>("addFilterScope", terminalSink);
            addSystemFunction<AddFilterLevelFunction>("addFilterLevel", terminalSink);
        }

        std::shared_ptr<Logger::LogSink> terminalSink;
    };

    struct UiModule : public SysModule
    {
        UiModule(EntitySystem *ecsRef)
        {
            addSystemFunction<CreateRectangle>("renderSquare", ecsRef);
        }
    };

    struct FileModule : public SysModule
    {
        FileModule()
        {
            addSystemFunction<OpenTextFileFunction>("openTextFile");
            addSystemFunction<OpenTextFolderFunction>("openTextFolder");
        }
    };
}

namespace pg
{
    Window::Window(const std::string &title) : title(title)
    {
        LOG_THIS_MEMBER(DOM);

        inputHandler = new Input();

        // [Start] Interpreter definition
        interpreter = ecs.createSystem<PgInterpreter>();

        interpreter->addSystemFunction<TestPrint>("print");
        interpreter->addSystemFunction<ToString>("toString");

        interpreter->addSystemModule("log", LogModule{});
        interpreter->addSystemModule("ui", UiModule{&ecs});
        interpreter->addSystemModule("ecs", EcsModule{&ecs});

        // Script to configure the logger
        interpreter->interpretFromFile("logManager.pg");
        // [End] Interpreter definition
    }

    Window::~Window()
    {
        LOG_THIS_MEMBER(DOM);

        ecs.stop();

        if(inputHandler != nullptr)
            delete inputHandler;

        if(fontLoader != nullptr)
            delete fontLoader;

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool Window::init(int width, int height, bool isFullscreen)
    {
        LOG_THIS_MEMBER(DOM);

        int flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;

        if (isFullscreen)
        {
            flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL;
        }

        LOG_INFO(DOM, "Initializing SDL...");

        if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
        {
            LOG_INFO(DOM, "SDL initialized successfully");

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

            // int err;

            // LOG_INFO(DOM, "Load OpenGL driver...");

            // #if defined(_WIN32)
            // err = SDL_GL_LoadLibrary ("opengl32.dll");
            // #elif defined(__linux__) || defined(__FreeBSD__)
            // err = SDL_GL_LoadLibrary ("libGL.so");
            // #else
            // #error Your platform is not supported
            // err = 1;
            // #endif

            // if (err != 0)
            // {
            //     LOG_ERROR(DOM, "Unable to load OpenGL driver.");
            //     return false;
            // }

            // LOG_INFO(DOM, "OpenGL driver loaded.");

            LOG_INFO(DOM, "Creating WindowSDL...");
            window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

            if (window != NULL)
            {
                LOG_INFO(DOM, "WindowSDL initialised");
            }
            else
            {
                LOG_ERROR(DOM, "WindowSDL init failed");
                return false;
            }

            LOG_INFO(DOM, "Creating OpenGL context...");
                
            // OpenGL context
            context = SDL_GL_CreateContext(window);

            if (context)
            {
                LOG_INFO(DOM, "OpenGL Context initialised");
            }
            else
            {
                LOG_ERROR(DOM, "OpenGL Context init failed");
                return false;
            }

            // OpenGL setup
            glewExperimental = GL_TRUE;
            GLenum initGLEW(glewInit());

            if (initGLEW == GLEW_OK)
            {
                LOG_INFO(DOM, "GLEW initialised");
            }
            else
            {
                LOG_ERROR(DOM, "GLEW init failed");
                return false;
            }
            
            // Get graphics info
            const GLubyte *renderer = glGetString(GL_RENDERER);
            const GLubyte *version = glGetString(GL_VERSION);

            LOG_INFO(DOM, "Renderer: " << renderer);
            LOG_INFO(DOM, "OpenGL version supported " << version);

            glViewport(0, 0, width, height);
            // Todo set this or not
            // glEnable(GL_CULL_FACE);
            // glEnable(GL_BLEND);
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (glDebugMessageControlARB != NULL)
            {
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback((GLDEBUGPROCARB)debugGlErrorCallback, NULL);
                GLuint unusedIds = 0;
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
            }

            return true;
        }
        else
        {
            LOG_ERROR(DOM, "SDL initialisation failed");
            LOG_ERROR(DOM, SDL_GetError());

            return false;
        }
    }

    bool Window::initEngine()
    {
        LOG_THIS_MEMBER(DOM);

        glViewport(0, 0, width, height);

        ecs.createSystem<TickingSystem>();

        ecs.createSystem<TerminalLogSystem>();

        ecs.createSystem<FocusableSystem>();

        // [Start] Master render definition

        masterRenderer = ecs.createSystem<MasterRenderer>();
        interpreter->addSystemModule("renderer", RendererModule{masterRenderer});

        interpreter->interpretFromFile("setupRenderer.pg");

        masterRenderer->setWindowSize(width, height);

        // [End] Master render definition

        ecs.createSystem<UiComponentSystem>();

        ecs.createSystem<Simple2DObjectSystem>(masterRenderer);

        ecs.createSystem<TextureComponentSystem>(masterRenderer);

        fontLoader = new FontLoader("res/font/fontmap.ft");
        ecs.createSystem<SentenceSystem>(masterRenderer, fontLoader);

        makeSentence(&ecs, 200, 50, {"At the start"});

        ecs.createSystem<FpsSystem>();

        ecs.createSystem<MouseLeftClickSystem>(inputHandler);
        ecs.createSystem<MouseRightClickSystem>(inputHandler);

        ecs.createSystem<MouseLeaveClickSystem>(inputHandler);
        
        ecs.createSystem<TextInputSystem>();

        ecs.createSystem<RunScriptFromTextInputSystem>();

        // Ecs task scheduling

        ecs.succeed<MouseRightClickSystem, TickingSystem>();
        ecs.succeed<MouseLeftClickSystem, TickingSystem>();

        ecs.succeed<UiComponentSystem, MouseRightClickSystem>();
        ecs.succeed<UiComponentSystem, MouseLeftClickSystem>();

        ecs.succeed<MasterRenderer, UiComponentSystem>();

        // Log taskflow for this window
        ecs.dumbTaskflow();

        screenEntity = ecs.createEntity();
        screenUi = ecs.attach<UiComponent>(screenEntity);
        screenUi->width = 10;
        screenUi->height = 10;
        screenUi->setZ(-1);

        ecs.attach<FocusableComponent>(screenEntity);
        ecs.attach<MouseLeftClickComponent>(screenEntity, makeCallable<OnFocus>(screenEntity.id));

        auto tex = makeUiTexture(&ecs, 160, 90, "menu");
        auto cTex = tex.get<UiComponent>();

        cTex->setBottomAnchor(screenUi->bottom);
        cTex->setLeftAnchor(screenUi->left);

        auto s = makeSimple2DShape(&ecs, Shape2D::Triangle, 50, 50, {255.0f, 0.0f, 0.0f});
        auto c = s.get<UiComponent>();

        c->setTopAnchor(screenUi->top);
        c->setRightAnchor(screenUi->right);

        auto terminalBackground = makeSimple2DShape(&ecs, Shape2D::Square, 350, 200, {4.0f, 16.0f, 32.0f});
        auto terminalBackgroundC = terminalBackground.get<UiComponent>();

        terminalBackgroundC->setBottomAnchor(screenUi->bottom);
        terminalBackgroundC->setRightAnchor(screenUi->right);

        auto terminalText = makeSentence(&ecs, 200, 200, {"Here"});
        auto terminalTextC = terminalText.get<UiComponent>();

        terminalTextC->setBottomAnchor(terminalBackgroundC->bottom);
        terminalTextC->setLeftAnchor(terminalBackgroundC->left);

        terminalTextC->setBottomMargin(10);
        terminalTextC->setLeftMargin(10);

        ecs.attach<TextInputComponent>(terminalText.entity, makeCallable<TextInputTriggeredEvent>(terminalText.entity), "Here");
        ecs.attach<FocusableComponent>(terminalText.entity);
        ecs.attach<MouseLeftClickComponent>(terminalText.entity, makeCallable<OnFocus>(terminalText.entity.id));

        // auto s2 = makeSimple2DShape(&ecs, Shape2D::Square, 100, 100, {0.0f, 255.0f, 0.0f});
        // auto c2 = s2.get<UiComponent>();

        // c2->setTopAnchor(screenUi->top);
        // c2->setLeftAnchor(screenUi->left);

        // ecs.attach<FocusableComponent>(s2.entity);
        
        // // ecs.attach<SentenceText>(s2.entity, "Here");
        // ecs.attach<MouseLeftClickComponent>(s2.entity, makeCallable<LogInfoEvent>("Window", "Left click on the green rectangle"));

        makeSentence(&ecs, 200, 250, {"And there"});

        ecs.start();

        return true;
    }

    void Window::processEvents(const SDL_Event& event)
    {
        switch(event.type)
        {
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                    // User clicked on 'x' to close the window
                    case SDL_WINDOWEVENT_CLOSE:
                        LOG_INFO(DOM, "User wants to close the window, quiting ...");
                        needToQuit = true;
                        break;

                    // User resized the window
                    case SDL_WINDOWEVENT_RESIZED:
                        LOG_MILE(DOM, "MESSAGE: Resizing window... New width: " << event.window.data1 << ", new height: " << event.window.data2);

                        this->resize(event.window.data1, event.window.data2);
                        break;

                    default:
                        break;
                }
                break;

            case SDL_KEYUP:
                inputHandler->registerKeyInput(event.key.keysym.scancode, Input::InputState::KEYPRESSED);
                break;

            case SDL_KEYDOWN:
                inputHandler->registerKeyInput(event.key.keysym.scancode, Input::InputState::KEYRELEASED);
                ecs.sendEvent(OnSDLScanCode{event.key.keysym.scancode});
                break;
            
            case SDL_MOUSEBUTTONDOWN:
                inputHandler->registerMouseInput(event.button.button, Input::InputState::MOUSEPRESS);
                break;

            case SDL_MOUSEBUTTONUP:
                inputHandler->registerMouseInput(event.button.button, Input::InputState::MOUSERELEASE);
                break;

            case SDL_MOUSEMOTION:
                {
                    MousePos currentPos {static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)};
                    MousePos mouseDelta {(mousePos.x - currentPos.x) * xSensitivity, (currentPos.y - mousePos.y) * ySensitivity};

                    inputHandler->registerMouseMove(currentPos, mouseDelta);

                    mousePos = currentPos;
                }
                break;

            case SDL_TEXTINPUT:
                LOG_INFO(DOM, "MESSAGE: Text input: " << std::string(event.text.text));
                ecs.sendEvent(OnSDLTextInput{std::string(event.text.text)});
                break;
        }
    }

    void Window::resize(int width, int height)
    {
        LOG_THIS_MEMBER(DOM);

        this->width = width;
        this->height = height;

        glViewport(0, 0, width, height);

        if(masterRenderer)
            masterRenderer->startResizing();

        if(screenUi->width != width)
        {
            screenUi->setWidth(width);

            if(masterRenderer)
                masterRenderer->setWindowSize(width, height);
        }
            
        if(screenUi->height != height)
        {
            screenUi->setHeight(height);
            
            if(masterRenderer)
                masterRenderer->setWindowSize(width, height);
        }

        if(masterRenderer)
            masterRenderer->stopResizing();
    }

    void Window::render()
    {
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        static auto lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        masterRenderer->setCurrentTime(currentTime);

        masterRenderer->renderAll();

        inputHandler->updateInput(float(currentTime - lastTime) / 1000);

        lastTime = currentTime;

        nbFrame++;

        swapBuffer();
    }

    void Window::swapBuffer()
    {
        // Check OpenGL error
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            LOG_ERROR(DOM, "OpenGL error: " << err);
        }

        SDL_GL_SwapWindow(window);

        // VSync 0 to disable 1 to activate
        SDL_GL_SetSwapInterval(0);
    }
}