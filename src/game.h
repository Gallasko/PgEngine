#ifndef GAME_H
#define GAME_H

#include <QWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QDateTime>
#include <QMatrix4x4>
#include <QVector3D>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLExtraFunctions>

#include <QKeyEvent>
#include <QMouseEvent>

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// timer includes
#include <chrono>
#include <thread>

#include "ECS/entitysystem.h"

#include "camera.h"
#include "Input/input.h"
#include "Input/inputcomponent.h"
#include "constant.h"
#include "Engine/map.h"
#include "Engine/fontloader.h"
#include "UI/sentencesystem.h"

#include "GameElements/Gui/tileselector.h"

class GameWindow : public QWindow, protected QOpenGLFunctions, public Base
{
	Q_OBJECT

public:
    explicit GameWindow(QWindow *parent = nullptr);
    ~GameWindow();

    virtual void initialize();

    virtual void render(QPainter *painter);
    virtual void render();

    void setAnimating(bool animating);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    //UI callback function
    void changeRandomText(Input* inputHandler, double deltaTime);
    void payTeclaFlooz(Input* inputHandler, double deltaTime);

public slots:
    void renderLater();
    void renderNow();

signals:
    void quitApp();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    void updateGameState(double deltaTime);
    void renderGame();
    void renderUi();
    void tick();
    bool m_animating = false;
    bool ticking = false;

    //Render var
    QOpenGLContext *m_context = nullptr;
    QOpenGLPaintDevice *m_device = nullptr;

    QOpenGLShaderProgram *defaultShaderProgram = nullptr;
    QOpenGLShaderProgram *guiShaderProgram = nullptr;
    QOpenGLShaderProgram *textShaderProgram = nullptr;
    QOpenGLVertexArrayObject *SquareVAO;
	QOpenGLBuffer *SquareVBO;
	QOpenGLBuffer *SquareEBO;

    unsigned int baseTileTexture1;
    unsigned int baseMenu1;
    unsigned int fontTexture;

    Map *gameMap = nullptr;

    EntitySystem ecs;
    TilesLoader *tileLoader;
    FontLoader *fontLoader;
    Input *inputHandler;

    // UI Element that need to be passed around
    // May need to put them in a table
    EntitySystem::Entity* fpsCounter;
    EntitySystem::Entity* mousePosText;
    EntitySystem::Entity* tilePosText;
    EntitySystem::Entity* tileType;
    EntitySystem::Entity* goldText;
    EntitySystem::Entity* nbRenderedGameFrameText;
    EntitySystem::Entity* currentSeedText;
    EntitySystem::Entity* userText;

    MouseInputComponent* mapClickComponent;

    TileSelector *tileSelector;

    std::string randomText;

    Map::MapConstraint mapConstraint;

    long long gold = 0;

    //camera var
    Camera *camera = nullptr;
	bool firstMouse = true;
    float xSensitivity = 1.0f;
    float ySensitivity = 1.0f;

    float gameScale = 100.0f;

    qint64 currentTime = 0;

    QPoint mousePos;

    bool debug = false;
    bool debugSwitched = false;

    //std::map accessibleParam;
};

#endif // Game_h
