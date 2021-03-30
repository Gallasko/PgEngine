#include <QGuiApplication>
#include <iostream>

#include "game.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	GameWindow game;
    game.resize(640, 480);
    game.setAnimating(true);
    game.show();

    QObject::connect(&game, SIGNAL(quitApp()), &app, SLOT(quit()));


	return app.exec();
}