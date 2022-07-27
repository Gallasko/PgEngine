#include <QGuiApplication>
#include <iostream>

#include "app.h"

#include <QTimer>

//[TODO] Variant using operator* dereferencing to recast to the original type
int main(int argc, char *argv[])
{
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