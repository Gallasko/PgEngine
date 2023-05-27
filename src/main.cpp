#include <iostream>

// #include "app.h"

// //[TODO] Variant using operator* dereferencing to recast to the original type
// int main(int argc, char *argv[])
// {
//     // Enable log in console
//     auto terminalSink = pg::Logger::registerSink<pg::TerminalSink>(true);
//     //TODO fix FilterFile
//     //terminalSink->addFilter("Input Filter", new Logger::LogSink::FilterScope("Input"));
//     // terminalSink->addFilter("Serializer Filter", new Logger::LogSink::FilterFile("src/Engine/serialization.cpp"));
//     // terminalSink->addFilter("Renderer Filter", new Logger::LogSink::FilterFile("src/Engine/Renderer/renderer.h"));
//     terminalSink->addFilter("Parser Filter", new Logger::LogSink::FilterScope("Parser"));
//     terminalSink->addFilter("Font Filter", new Logger::LogSink::FilterScope("Font Loader"));
//     terminalSink->addFilter("Registry Filter", new Logger::LogSink::FilterScope("Component Registry"));
//     // terminalSink->addFilter("Configuration Filter", new Logger::LogSink::FilterFile("src/Engine/configuration.cpp"));
//     // terminalSink->addFilter("Editor Filter", new Logger::LogSink::FilterFile("src/app.cpp"));
//     terminalSink->addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

//     // auto fileSink = pg::Logger::registerSink<pg::FileSink>();

//     // fileSink->addFilter("Parser Filter", new Logger::LogSink::FilterScope("Parser"));
    
//     QSurfaceFormat format;
//     format.setSwapInterval(0);
//     format.setRenderableType(QSurfaceFormat::OpenGL);
//     //format.setProfile(QSurfaceFormat::CoreProfile);
//     format.setVersion(3, 3);

//     QSurfaceFormat::setDefaultFormat(format);

// 	QGuiApplication app(argc, argv);

// 	EditorWindow editor;
//     //game.resize(640, 480);
//     editor.resize(800, 480);
//     //game.setWindowState(Qt::WindowFullScreen);
//     editor.setAnimating(true);
//     //game.showFullScreen();
//     editor.show();

//     QTimer* timer = new QTimer();
//     QObject::connect( timer, SIGNAL( timeout() ), &editor, SLOT( renderNow() ) );
//     timer->start(0);

//     QObject::connect(&editor, SIGNAL(quitApp()), &app, SLOT(quit()));
//     return app.exec();
// }

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#elif _WIN32
#include <SDL.h>
#include <SDL_opengl.h>
#endif


#include <GL/gl.h>

typedef int32_t i32;
typedef uint32_t u32;
typedef int32_t b32;

#define WinWidth 1000
#define WinHeight 1000

int main (int ArgCount, char **Args)
{

  u32 WindowFlags = SDL_WINDOW_OPENGL;
  SDL_Window *Window = SDL_CreateWindow("OpenGL Test", 100, 100, WinWidth, WinHeight, WindowFlags);
  assert(Window);
  SDL_GLContext Context = SDL_GL_CreateContext(Window);
  
  b32 Running = 1;
  b32 FullScreen = 0;
  while (Running)
  {
    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
      if (Event.type == SDL_KEYDOWN)
      {
        switch (Event.key.keysym.sym)
        {
          case SDLK_ESCAPE:
            Running = 0;
            break;
          case 'f':
            FullScreen = !FullScreen;
            if (FullScreen)
            {
              SDL_SetWindowFullscreen(Window, WindowFlags | SDL_WINDOW_FULLSCREEN_DESKTOP);
            }
            else
            {
              SDL_SetWindowFullscreen(Window, WindowFlags);
            }
            break;
          default:
            break;
        }
      }
      else if (Event.type == SDL_QUIT)
      {
        Running = 0;
      }
    }

    glViewport(0, 0, WinWidth, WinHeight);
    glClearColor(1.f, 0.f, 1.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(Window);
  }
  return 0;
}