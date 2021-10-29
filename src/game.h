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
#include "Engine/renderer.h"
#include "UI/sentencesystem.h"
#include "UI/uianimation.h"

#include "Engine/particle.h"

#include "GameElements/Gui/tileselector.h"

//TODO create unary test for all the basic component of the framework

//TODO manage resize event so it doesnt crash the app 

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
    void changeRandomText(Input* inputHandler, double deltaTime...);
    void payTeclaFlooz(Input* inputHandler, double deltaTime);
    void showPigeonWidget(Input* inputHandler, double deltaTime...);
    
    void gameplayTest(Input* inputHandler, double...) { static bool pressed = false; if(inputHandler->isButtonPressed(Qt::LeftButton) && !pressed) gameMap->createPathBetweenHouseAndShop(); if(!inputHandler->isButtonPressed(Qt::LeftButton)) pressed = false;}

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

    QOpenGLVertexArrayObject *SquareVAO;
	QOpenGLBuffer *SquareVBO;
	QOpenGLBuffer *SquareEBO;

    Map *gameMap = nullptr;

    EntitySystem ecs;
    MasterRenderer masterRenderer;
    TilesLoader *tileLoader;
    FontLoader *fontLoader;
    Input *inputHandler;

    // UI Element that need to be passed around
    // May need to put them in a table
    EntitySystem::Entity* fpsCounter;
    EntitySystem::Entity* mousePosText;
    EntitySystem::Entity* gameScaleText;
    EntitySystem::Entity* tilePosText;
    EntitySystem::Entity* tileType;
    EntitySystem::Entity* goldText;
    EntitySystem::Entity* nbRenderedGameFrameText;
    EntitySystem::Entity* currentSeedText;
    EntitySystem::Entity* userText;

    TextureComponent *cmpTexTest;

    EntitySystem::Entity *screenEntity;
    UiComponent *screenUi; 

    ParticleComponent *pComponent;

    MouseInputComponent* mapClickComponent;

    TileSelector *tileSelector;

    std::string randomText;

    Map::MapConstraint mapConstraint;

    KeyboardInputComponent** screenKeyInput;
    KeyboardInputComponent** pigeonShowingKeyboard;

    //TODO pigeon spawner stuff
    AnimationComponent *pigeonReveal;
    AnimationComponent *pigeonHide;

    long long gold = 0;

    //camera var
    Camera *camera = nullptr;
	bool firstMouse = true;
    float xSensitivity = 1.0f;
    float ySensitivity = 1.0f;

    //TODO adapt this value
    float gameScale = 215.0f;

    qint64 currentTime = 0;

    QPoint mousePos;

    bool debug = false;
    bool debugSwitched = false;
};

#endif // Game_h
