#include <QGuiApplication>
#include <iostream>

#include "app.h"

#include <QTimer>

//[TODO] Variant using operator* dereferencing to recast to the original type
int main(int argc, char *argv[])
{
    // Enable log in console
    auto terminalSink = pg::Logger::registerSink<pg::TerminalSink>(true);
    //TODO fix FilterFile
    //terminalSink->addFilter("Input Filter", new Logger::LogSink::FilterScope("Input"));
    // terminalSink->addFilter("Serializer Filter", new Logger::LogSink::FilterFile("src/Engine/serialization.cpp"));
    // terminalSink->addFilter("Renderer Filter", new Logger::LogSink::FilterFile("src/Engine/Renderer/renderer.h"));
    terminalSink->addFilter("Parser Filter", new Logger::LogSink::FilterScope("Parser"));
    terminalSink->addFilter("Font Filter", new Logger::LogSink::FilterScope("Font Loader"));
    // terminalSink->addFilter("Configuration Filter", new Logger::LogSink::FilterFile("src/Engine/configuration.cpp"));
    // terminalSink->addFilter("Editor Filter", new Logger::LogSink::FilterFile("src/app.cpp"));
    terminalSink->addFilter("Log Level Filter", new Logger::LogSink::FilterLogLevel(Logger::InfoLevel::log));

    auto fileSink = pg::Logger::registerSink<pg::FileSink>();

    fileSink->addFilter("Parser Filter", new Logger::LogSink::FilterScope("Parser"));
    
    QSurfaceFormat format;
    format.setSwapInterval(0);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);

    QSurfaceFormat::setDefaultFormat(format);

	QGuiApplication app(argc, argv);

	EditorWindow editor;
    //game.resize(640, 480);
    editor.resize(800, 480);
    //game.setWindowState(Qt::WindowFullScreen);
    editor.setAnimating(true);
    //game.showFullScreen();
    editor.show();

    QTimer* timer = new QTimer();
    QObject::connect( timer, SIGNAL( timeout() ), &editor, SLOT( renderNow() ) );
    timer->start(0);

    QObject::connect(&editor, SIGNAL(quitApp()), &app, SLOT(quit()));
    return app.exec();
}