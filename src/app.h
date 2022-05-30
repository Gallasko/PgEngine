#pragma once

#include <QWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QOpenGLExtraFunctions>

#include <QKeyEvent>
#include <QMouseEvent>

#include <iostream>
// timer includes
#include <chrono>
#include <thread>

#include "ECS/entitysystem.h"

#include "Input/input.h"
#include "Input/inputcomponent.h"
#include "constant.h"
#include "Loaders/fontloader.h"
#include "Renderer/renderer.h"
#include "UI/sentencesystem.h"
#include "UI/uianimation.h"
#include "UI/listview.h"

using namespace pg;

//TODO make a MainWindow that handle all the QT events and can start all the base engine systems !

class EditorWindow : public QWindow, protected QOpenGLFunctions
{
	Q_OBJECT

public:
    explicit EditorWindow(QWindow *parent = nullptr);
    ~EditorWindow();

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

public slots:
    void renderLater();
    void renderNow();

signals:
    void quitApp();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    // Ui Input callbacks
    void openContextMenu(Input* inputHandler, double...);
    void closeContextMenu(Input* inputHandler, double);

    UiComponent *sceneEntityC;
    TextureComponent* contextMenu;

    void renderUi();
    void tick();
    void quit(Input* inputHandler, double...);

    bool m_animating = false;
    bool ticking = false;

    //Render var
    QOpenGLContext *m_context = nullptr;
    QOpenGLPaintDevice *m_device = nullptr;

    EntitySystem ecs;
    MasterRenderer masterRenderer;

    Input *inputHandler;
    FontLoader *fontLoader;

    EntitySystem::Entity *screenEntity;
    UiComponent *screenUi;

    float xSensitivity = 1.0f;
    float ySensitivity = 1.0f;

    qint64 currentTime = 0;

    QPoint mousePos;

    bool debug = false;
    bool debugSwitched = false;
};