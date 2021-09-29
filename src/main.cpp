#include <QGuiApplication>
#include <iostream>

#include "game.h"

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

	GameWindow game;
    //game.resize(640, 480);
    game.resize(800, 480);
    //game.setWindowState(Qt::WindowFullScreen);
    game.setAnimating(true);
    //game.showFullScreen();
    game.show();

    QTimer* timer = new QTimer();
    QObject::connect( timer, SIGNAL( timeout() ), &game, SLOT( renderNow() ) );
    timer->start(0);

    QObject::connect(&game, SIGNAL(quitApp()), &app, SLOT(quit()));

	return app.exec();
}