#include <QGuiApplication>
#include <iostream>

#include "game.h"

#include <QTimer>

#include <QFile>
#include <QString>
#include <QTextStream>

//[TODO] Variant using operator* dereferencing to recast to the original type
int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setSwapInterval(0);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    //format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);

    QSurfaceFormat::setDefaultFormat(format);

    //Q_INIT_RESOURCE(qml);

    QFile file(":/res/names/american/female.names");

    if (!file.open(QIODevice::ReadOnly))
    {
        std::cout << "Couldn't open file" << std::endl;
        return -1;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        std::cout << line.toStdString() << std::endl;
    }
    
    /*
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

    */
    return 0;
}