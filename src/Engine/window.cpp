#include "gl_debug.hpp"

#include "window.h"

#include <string>
#include <chrono>

#include "logger.h"

#include "Renderer/renderer.h"
#include "Ui/uisystem.h"
#include "Input/input.h"

#include "UI/texture.h"


namespace
{
    static const char * const DOM = "Window";
}

namespace pg
{
    Window::Window(const std::string &title) : title(title)
    {
        LOG_THIS_MEMBER(DOM);

        inputHandler = new Input();
    }

    Window::~Window()
    {
        LOG_THIS_MEMBER(DOM);

        ecs.stop();

        if(inputHandler != nullptr)
            delete inputHandler;

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool Window::init(int width, int height, bool isFullscreen)
    {
        LOG_THIS_MEMBER(DOM);

        // this->width = width;
        // this->height = height;

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

        masterRenderer = ecs.createSystem<MasterRenderer>();

        ecs.createSystem<UiComponentSystem>();

        ecs.createSystem<TextureComponentSystem>(masterRenderer);

        masterRenderer->registerShader("default", "shader/default.vs", "shader/default.fs");

        masterRenderer->registerShader("gui", "shader/default.vs", "shader/default.fs");
        masterRenderer->registerShader("text", "shader/textrendering.vs", "shader/textrendering.fs");

        masterRenderer->registerTexture("menu", "res/menu/Menu.png");

        masterRenderer->setWindowSize(width, height);

        // Ecs task scheduling
        ecs.succeed<MasterRenderer, UiComponentSystem>();

        // Log taskflow for this window
        ecs.dumbTaskflow();

        screenEntity = ecs.createEntity();
        screenUi = ecs.attach<UiComponent>(screenEntity);
        screenUi->width = 400;
        screenUi->height = 400;
        screenUi->setZ(-1);

        auto e = makeUiTexture(&ecs, 160, 90, "menu");
        auto c = e.get<UiComponent>();

        // c->setBottomAnchor(screenUi->bottom);
        // c->setRightAnchor(screenUi->right);

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
                if(event.key.keysym.sym == SDLK_ESCAPE)
                    needToQuit = true;
                break;
                
        }
    }

    void Window::resize(int width, int height)
    {
        LOG_THIS_MEMBER(DOM);

        this->width = width;
        this->height = height;

        glViewport(0, 0, width, height);

        if(screenUi->width != width)
        {
            screenUi->setWidth(width);
            masterRenderer->setWindowSize(width, height);
        }
            
        if(screenUi->height != height)
        {
            screenUi->setHeight(height);
            masterRenderer->setWindowSize(width, height);
        }
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
    }
}