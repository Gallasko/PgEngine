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

#include "camera.h"
#include "Input/input.h"
#include "constant.h"

//pragma pack data so it doesnt get padded
#pragma pack(push, 0)

struct Particle
{
    constant::Vector3D pos;

    float lifetime;
};

#pragma pack(pop)

struct ParticleInfo
{
    float timeElapsed = 0.0f;

    std::vector<constant::Vector3D> dir;
};

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

public slots:
    void renderLater();
    void renderNow();

signals:
    void quitApp();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    void renderUi();
    void tick();
    bool m_animating = false;
    bool ticking = false;

    //Render var
    QOpenGLContext *m_context = nullptr;
    QOpenGLPaintDevice *m_device = nullptr;

    QOpenGLExtraFunctions *extraFunctions = nullptr; 

    QOpenGLShaderProgram *defaultShaderProgram = nullptr;
    QOpenGLShaderProgram *guiShaderProgram = nullptr;
    QOpenGLShaderProgram *textShaderProgram = nullptr;
    QOpenGLVertexArrayObject *SquareVAO;
	QOpenGLBuffer *SquareVBO;
	QOpenGLBuffer *SquareEBO;
	
    QOpenGLBuffer *instanceVBO;

    const unsigned int nbMaxParticle = 200;
    Particle particleList[200];
    ParticleInfo particleInfoList[200];

    unsigned int baseTileTexture1;
    unsigned int baseMenu1;
    unsigned int fontTexture;

    Input *inputHandler;

    //camera var
    Camera *camera = nullptr;
	//float lastCameraX = constant::SCREEN_WIDTH / 2.0f;
	//float lastCameraY = constant::SCREEN_HEIGHT / 2.0f;
	bool firstMouse = true;
    float xSensitivity = 1.0f;
    float ySensitivity = 1.0f;

    QPoint mousePos;
};

#endif // Game_h
