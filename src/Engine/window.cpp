#include "gl_debug.hpp"

#include "window.h"

#include <string>

#include "logger.h"


namespace
{
    static const char * const DOM = "Window";
}

namespace pg
{
    Window::Window(const std::string &title) : title(title)
    {
        LOG_THIS_MEMBER(DOM);
    }

    Window::~Window()
    {
        LOG_THIS_MEMBER(DOM);

        SDL_GL_DeleteContext(context);
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

        if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
        {
            LOG_INFO(DOM, "Subsystems initialised");

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

            // WindowSdl
            window = std::unique_ptr<SDL_Window, SdlWindowDestroyer>(
                SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags));

            if (window)
            {
                LOG_INFO(DOM, "WindowSdl initialised");
            }
            else
            {
                LOG_ERROR(DOM, "WindowSdl init failed");
                return false;
            }
                
            // OpenGL context
            context = SDL_GL_CreateContext(window.get());

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

    void Window::resize(int width, int height)
    {
        LOG_THIS_MEMBER(DOM);

        glViewport(0, 0, width, height);
    }

    void Window::swapBuffer()
    {
        // Check OpenGL error
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            LOG_ERROR(DOM, "OpenGL error: " << err);
        }

        SDL_GL_SwapWindow(window.get());
    }
}