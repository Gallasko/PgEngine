#include <iostream>

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

#include "application.h"

#include "logger.h"

//[TODO] Variant using operator* dereferencing to recast to the original type
int main(int argc, char *argv[])
{
    // Enable log in console
    // auto terminalSink = pg::Logger::registerSink<pg::TerminalSink>(true);
    //TODO fix FilterFile
    //terminalSink->addFilter("Input Filter", new Logger::LogSink::FilterScope("Input"));
    // terminalSink->addFilter("Serializer Filter", new Logger::LogSink::FilterFile("src/Engine/serialization.cpp"));
    // terminalSink->addFilter("Renderer Filter", new Logger::LogSink::FilterFile("src/Engine/Renderer/renderer.h"));
    // terminalSink->addFilter("Parser Filter", new pg::Logger::LogSink::FilterScope("Parser"));
    // terminalSink->addFilter("Font Filter", new pg::Logger::LogSink::FilterScope("Font Loader"));
    // terminalSink->addFilter("Registry Filter", new pg::Logger::LogSink::FilterScope("Component Registry"));
    // terminalSink->addFilter("Configuration Filter", new Logger::LogSink::FilterFile("src/Engine/configuration.cpp"));
    // terminalSink->addFilter("Editor Filter", new Logger::LogSink::FilterFile("src/app.cpp"));
    // terminalSink->addFilter("Log Level Filter", new pg::Logger::LogSink::FilterLogLevel(pg::Logger::InfoLevel::log));

    // auto fileSink = pg::Logger::registerSink<pg::FileSink>();

    // fileSink->addFilter("Parser Filter", new Logger::LogSink::FilterScope("Parser"));

    pg::PgApplication app("Tecla eats");

	// EditorWindow editor;
    // //game.resize(640, 480);
    // editor.resize(800, 480);
    // //game.setWindowState(Qt::WindowFullScreen);
    // editor.setAnimating(true);
    // //game.showFullScreen();
    // editor.show();

    return app.exec();
}


