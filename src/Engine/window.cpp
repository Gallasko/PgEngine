#include "gl_debug.hpp"

#include "window.h"

#include <string>
#include <chrono>

#include "ECS/loggersystem.h"
#include "ECS/ecsmodule.h"

#include "Renderer/renderer.h"
#include "Renderer/renderermodule.h"

#include "Input/input.h"
#include "Input/inputmodule.h"

#include "UI/uisystem.h"
#include "UI/focusable.h"
#include "UI/textinput.h"
#include "UI/sentencesystem.h"

#include "2D/simple2dobject.h"
#include "2D/texture.h"

#include "Scene/scenemanager.h"

#include "Interpreter/pginterpreter.h"
#include "Interpreter/systemfunction.h"

#include "Systems/coremodule.h"
#include "Systems/logmodule.h"
#include "Systems/shape2Dmodule.h"
#include "Systems/timemodule.h"
#include "Systems/sentencemodule.h"
#include "Systems/texture2Dmodule.h"
#include "Systems/scenemodule.h"

#include "Audio/audiosystem.h"
#include "Audio/audiomodule.h"

// #include "GameElements/Systems/basicsystems.h"

#include "logger.h"
#include "serialization.h"

namespace pg
{
    namespace
    {
        static const char * const DOM = "Window";

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

                return makeVar(file.data);
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

        struct FileModule : public SysModule
        {
            FileModule()
            {
                addSystemFunction<OpenTextFileFunction>("openTextFile");
                addSystemFunction<OpenTextFolderFunction>("openTextFolder");
            }
        };

        void getAllControllers(Input *inputHandler)
        {
            for (int i = 0; i < SDL_NumJoysticks(); i++)
            {
                if (SDL_IsGameController(i))
                {
                    inputHandler->addGamepad(SDL_GameControllerOpen(i));
                }
            }
        }

    }

    Window::Window(const std::string &title) : title(title)
    {
        LOG_THIS_MEMBER(DOM);

        inputHandler = new Input();

        // [Start] Interpreter definition
        interpreter = ecs.createSystem<PgInterpreter>();

        interpreter->addSystemFunction<TestPrint>("print");
        interpreter->addSystemFunction<DebugPrint>("debugPrint");
        interpreter->addSystemFunction<ToString>("toString");

        interpreter->addSystemModule("log", LogModule{});
        interpreter->addSystemModule("ui", UiModule{&ecs});
        interpreter->addSystemModule("2Dshapes", Shape2DModule{&ecs});
        interpreter->addSystemModule("2Dtexture", Texture2DModule{&ecs});
        interpreter->addSystemModule("time", TimeModule{&ecs});
        interpreter->addSystemModule("ecs", EcsModule{&ecs});
        interpreter->addSystemModule("core", CoreModule{&ecs});
        interpreter->addSystemModule("input", InputModule{&ecs});
        interpreter->addSystemModule("uitext", SentenceModule{&ecs});
        interpreter->addSystemModule("scene", SceneModule{&ecs});
        interpreter->addSystemModule("audio", AudioModule{&ecs});
        
        // Script to configure the logger
        interpreter->interpretFromFile("logManager.pg");
        // [End] Interpreter definition
    }

    Window::~Window()
    {
        LOG_THIS_MEMBER(DOM);

        ecs.stop();

        if (inputHandler != nullptr)
        {
            inputHandler->clearGamepads();

            delete inputHandler;
        }

        if (fontLoader != nullptr)
            delete fontLoader;

        if (audioSystem != nullptr)
            audioSystem->closeSDLMixer();

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

            LOG_INFO(DOM, "Getting all connected controllers...");

            getAllControllers(inputHandler);

            LOG_INFO(DOM, "Got all connected controllers");

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
            glDisable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (glDebugMessageControlARB != NULL)
            {
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

#ifndef __EMSCRIPTEN__
                glDebugMessageCallback((GLDEBUGPROCARB)debugGlErrorCallback, NULL);
                GLuint unusedIds = 0;
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
#endif
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

        ecs.createSystem<TimerSystem>();

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

        ecs.createSystem<Texture2DComponentSystem>(masterRenderer);

        fontLoader = new FontLoader("res/font/fontmap.ft");
        ecs.createSystem<SentenceSystem>(masterRenderer, fontLoader);

        audioSystem = ecs.createSystem<AudioSystem>();

        // ecs.createSystem<FpsSystem>();

        ecs.createSystem<MouseLeftClickSystem>(inputHandler);
        ecs.createSystem<MouseRightClickSystem>(inputHandler);

        ecs.createSystem<MouseLeaveClickSystem>(inputHandler);
        
        ecs.createSystem<TextInputSystem>();

        ecs.createSystem<RunScriptFromTextInputSystem>();

        ecs.createSystem<SceneElementSystem>();

        // Ecs task scheduling

        ecs.succeed<TickingSystem, PgInterpreter>();

        ecs.succeed<TimerSystem, TickingSystem>();

        ecs.succeed<MouseRightClickSystem, TickingSystem>();
        ecs.succeed<MouseLeftClickSystem, TickingSystem>();

        ecs.succeed<UiComponentSystem, MouseRightClickSystem>();
        ecs.succeed<UiComponentSystem, MouseLeftClickSystem>();

        ecs.succeed<MasterRenderer, UiComponentSystem>();

        ecs.succeed<SceneElementSystem, MasterRenderer>();

        // Script to configure all the users systems
        interpreter->interpretFromFile("sysRegister.pg");

        // Log taskflow for this window
        ecs.dumbTaskflow();

        screenEntity = ecs.createEntity();
        screenUi = ecs.attach<UiComponent>(screenEntity);
        screenUi->width = 10;
        screenUi->height = 10;
        screenUi->setZ(-1);

        // ecs.attach<FocusableComponent>(screenEntity);
        // ecs.attach<MouseLeftClickComponent>(screenEntity, makeCallable<OnFocus>(screenEntity.id));

        // Serializer::getSerializer()->serializeObject("Test Focusable", *screenEntity->get<FocusableComponent>());

        // Serializer::getSerializer()->serializeObject("ScreenEntity", *screenEntity.entity);

        // auto tex = makeUiTexture(&ecs, 160, 90, "menu");
        // auto cTex = tex.get<UiComponent>();

        // cTex->setTopAnchor(screenUi->top);
        // cTex->setRightAnchor(screenUi->right);

        // auto s = makeSimple2DShape(&ecs, Shape2D::Triangle, 50, 50, {255.0f, 0.0f, 0.0f});
        // auto c = s.get<UiComponent>();

        // c->setTopAnchor(screenUi->top);
        // c->setRightAnchor(screenUi->right);

        // Serializer::getSerializer()->serializeObject("Menu tex", *tex.entity);

        // auto terminalBackground = makeSimple2DShape(&ecs, Shape2D::Square, 350, 200, {4.0f, 16.0f, 32.0f});
        // auto terminalBackgroundC = terminalBackground.get<UiComponent>();

        // terminalBackgroundC->setBottomAnchor(screenUi->bottom);
        // terminalBackgroundC->setRightAnchor(screenUi->right);

        // Serializer::getSerializer()->serializeObject("Terminal background", *terminalBackground.entity);

        // auto terminalText = makeSentence(&ecs, 200, 200, {"Here"});
        // auto terminalTextC = terminalText.get<UiComponent>();

        // terminalTextC->setBottomAnchor(terminalBackgroundC->bottom);
        // terminalTextC->setLeftAnchor(terminalBackgroundC->left);

        // auto testb = makeSimple2DShape(&ecs, Shape2D::Square, 120, 120, {4.0f, 225.0f, 125.0f});
        // auto testbc = testb.get<UiComponent>();

        // testbc->setBottomAnchor(screenUi->bottom);
        // testbc->setRightAnchor(screenUi->right);

        // testbc->setBottomMargin(550);

        // terminalTextC->setBottomMargin(10);
        // terminalTextC->setLeftMargin(10);

        // ecs.attach<TextInputComponent>(terminalText.entity, makeCallable<TextInputTriggeredEvent>(terminalText.entity), "Here");
        // ecs.attach<FocusableComponent>(terminalText.entity);
        // ecs.attach<MouseLeftClickComponent>(terminalText.entity, makeCallable<OnFocus>(terminalText.entity.id));

        // auto s2 = makeSimple2DShape(&ecs, Shape2D::Square, 100, 100, {0.0f, 255.0f, 0.0f});
        // auto c2 = s2.get<UiComponent>();

        // c2->setTopAnchor(screenUi->top);
        // c2->setLeftAnchor(screenUi->left);

        // ecs.attach<FocusableComponent>(s2.entity);
        
        // // ecs.attach<SentenceText>(s2.entity, "Here");
        // ecs.attach<MouseLeftClickComponent>(s2.entity, makeCallable<LogInfoEvent>("Window", "Left click on the green rectangle"));

        // makeSentence(&ecs, 200, 250, {"And there"});

        // ecs.sendEvent(StartAudio{"res/mainost.mp3"});

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
                        LOG_INFO(DOM, "MESSAGE: Resizing window... New width: " << event.window.data1 << ", new height: " << event.window.data2);

                        this->resize(event.window.data1, event.window.data2);
                        break;

                    default:
                        break;
                }
                break;

            case SDL_KEYUP:
                inputHandler->registerKeyInput(event.key.keysym.scancode, Input::InputState::KEYPRESSED);
                ecs.sendEvent(OnSDLScanCodePressed{event.key.keysym.scancode});
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
            
            case SDL_CONTROLLERDEVICEADDED:
                // Todo
                LOG_INFO(DOM, "Gamepad added !");
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                // Todo
                LOG_INFO(DOM, "Gamepad removed !");
                break;

            case SDL_CONTROLLERBUTTONDOWN:
                ecs.sendEvent(OnSDLGamepadPressed{event.cbutton.which, event.cbutton.button});
                break;

            case SDL_CONTROLLERBUTTONUP:
                ecs.sendEvent(OnSDLGamepadReleased{event.cbutton.which, event.cbutton.button});
                break;

            case SDL_CONTROLLERAXISMOTION:
                ecs.sendEvent(OnSDLGamepadAxisChanged{event.caxis.which, event.caxis.axis, event.caxis.value});
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

        std::lock_guard<std::mutex> lock(renderMutex);

        glViewport(0, 0, width, height);

        if(screenUi->width != width)
        {
            screenUi->setWidth(width);
        }
        
        if(screenUi->height != height)
        {
            screenUi->setHeight(height);
        }

        ecs.sendEvent(ResizeEvent{static_cast<float>(width), static_cast<float>(height)});
    }

    void Window::render()
    {
        currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        static auto lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        std::lock_guard<std::mutex> lock(renderMutex);

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