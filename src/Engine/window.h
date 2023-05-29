#pragma once

#ifdef __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <GL/gl.h>

#include <memory>

#include "ECS/entitysystem.h"

// Used by SDL_Window unique pointer
struct SdlWindowDestroyer
{
    void operator()(SDL_Window *window) const
    {
        SDL_DestroyWindow(window);
    }
};

// Todo see if we support multiple window rendering in which case we need to correctly send the events to correct window

namespace pg
{
    // Type forwarding
    class MasterRenderer;
    class Input;
    class UiComponent;

    class Window
    {
    public:
        Window(const std::string &title);
        virtual ~Window();

        bool init(int width, int height, bool isFullscreen);
        virtual bool initEngine();

        virtual void processEvents(const SDL_Event& event);

        void resize(int width, int height);

        virtual void render();

        inline bool requestQuit() const { return needToQuit; }

    protected:
        void swapBuffer();

    private:
        std::unique_ptr<SDL_Window, SdlWindowDestroyer> window;
        SDL_GLContext context;

        const std::string &title;
        bool needToQuit = false;

        uint64_t currentTime = 0;

        uint32_t nbFrame = 0;

        int width = 1, height = 1;

        EntitySystem ecs;

        MasterRenderer *masterRenderer = nullptr;
        Input *inputHandler = nullptr;

        EntityRef screenEntity;
        CompRef<UiComponent> screenUi;
    };
}