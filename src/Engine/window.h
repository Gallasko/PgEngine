#pragma once

#ifdef __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <GL/gl.h>

#include <memory>

// Used by SDL_Window unique pointer
struct SdlWindowDestroyer
{
    void operator()(SDL_Window *window) const
    {
        SDL_DestroyWindow(window);
    }
};

// namespace std
// {
//     // Forward declaration
//     class string;
// }

namespace pg
{
    class Window
    {
    public:
        Window(const std::string &title);
        virtual ~Window();

        bool init(int xPos, int yPos, int width, int height, bool isFullscreen);

        void resize(int width, int height);

        void swapBuffer();

    private:
        std::unique_ptr<SDL_Window, SdlWindowDestroyer> window;
        SDL_GLContext context;

        const std::string &title;
    };
}