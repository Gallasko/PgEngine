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

    void sceneModification(Input* inputHandler, double deltaTime...);

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

    EntitySystem ecs;
    MasterRenderer masterRenderer;
    TilesLoader *tileLoader;
    FontLoader *fontLoader;
    Input *inputHandler;

    // UI Element that need to be passed around
    // May need to put them in a table
    EntitySystem::Entity* fpsCounter;
    EntitySystem::Entity* mousePosText;
    EntitySystem::Entity* userText;

    std::vector<EntitySystem::Entity*> entityTable;

    UiComponent *screenUi; 
    UiComponent *sceneUi;
 
    std::string randomText;

    //camera var
    Camera *camera = nullptr;
	bool firstMouse = true;
    float xSensitivity = 1.0f;
    float ySensitivity = 1.0f;

    qint64 currentTime = 0;

    QPoint mousePos;

    bool debug = false;
    bool debugSwitched = false;
};

#endif // Game_h
