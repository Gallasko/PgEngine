#include <iostream>

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "app.h"

#include "logger.h"

#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <SDL_opengl.h>
#include <emscripten.h>
#else
    #ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
    #elif _WIN32
    #include <SDL.h>
    #include <SDL_opengl.h>
    #endif
#endif

#include <GL/gl.h>

//[TODO] Variant using operator* dereferencing to recast to the original type
int main(int argc, char *argv[])
{
    TetrisApp app("Tetris clone");

    return app.exec();
}


