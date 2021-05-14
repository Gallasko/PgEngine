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
#include "constant.h"
#include "Engine/map.h"
#include "Engine/fontloader.h"
#include "UI/sentencesystem.h"

class GameWindow : public QWindow, protected QOpenGLFunctions
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

public slots:
    void renderLater();
    void renderNow();

signals:
    void quitApp();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
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

    EntitySystem::Entity*** tileMap;

    EntitySystem ecs;
    TilesLoader *tileLoader;
    FontLoader *fontLoader;
    Input *inputHandler;

    EntitySystem::Entity* fpsCounter;
    EntitySystem::Entity* mousePosText;

    //camera var
    Camera *camera = nullptr;
	//float lastCameraX = constant::SCREEN_WIDTH / 2.0f;
	//float lastCameraY = constant::SCREEN_HEIGHT / 2.0f;
	bool firstMouse = true;
    float xSensitivity = 1.0f;
    float ySensitivity = 1.0f;

    float gameScale = 100.0f;

    qint64 currentTime = 0;

    QPoint mousePos;
};

#endif // Game_h
