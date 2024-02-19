#include <iostream>

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "app.h"

#include "logger.h"

#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES 1
#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL_opengles2.h>
// #include <SDL_opengl_glext.h>
// #include <GLES2/gl2.h>
// #include <GLFW/glfw3.h>
#else
#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#elif _WIN32
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <GL/gl.h>
#endif


//[TODO] Variant using operator* dereferencing to recast to the original type
int main(int argc, char *argv[])
{
#ifdef __EMSCRIPTEN__
    printf("Starting program...\n");
#endif
    TetrisApp app("Tetris clone");

    return app.exec();
}


