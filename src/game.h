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

#include <mutex>


#include "Engine/ECS/entitysystem.h"

#include "Engine/camera.h"
#include "Engine/Input/input.h"
#include "Engine/Input/inputcomponent.h"
#include "Engine/constant.h"
#include "GameElements/Systems/map.h"
#include "Engine/Loaders/fontloader.h"
#include "Engine/Renderer/renderer.h"
#include "Engine/UI/sentencesystem.h"
#include "Engine/UI/uianimation.h"
#include "Engine/UI/listview.h"

#include "Engine/Renderer/particle.h"

#include "GameElements/Gui/tileselector.h"
#include "GameElements/Gui/escapepanel.h"

#include "UI/texture.h"


//TODO create unary test for all the basic component of the framework

//TODO manage resize event so it doesnt crash the app

//TODO move this struct and all code relative to the pigeon movement on the map in its class

using namespace pg;

struct PigeonEntity
{
    std::vector<constant::Vector2D> path;

    unsigned int currentTime = 0;
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
    void wheelEvent(QWheelEvent *event);

    //UI callback function
    void changeRandomText(Input* inputHandler, double deltaTime...);
    void payTeclaFlooz(Input* inputHandler, double deltaTime);
    void showPigeonWidget(Input* inputHandler, double deltaTime...);
    
    void gameplayTest(Input* inputHandler, double...);

public slots:
    void renderLater();
    void renderNow();

signals:
    void quitApp();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    //void updateGameState(double deltaTime);
    void renderGame();
    void renderUi();
    void tick();
    void quit(Input* inputHandler, double...);
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

    //TODO on map pigeon stuff
    QOpenGLVertexArrayObject *pigeonVAO;
    std::vector<PigeonEntity> pigeonEntities;
    std::mutex pigeonMutex;

    //TODO listView stuff
    UiFrame frame;
    ListView *listView;

    //TODO escape stuff
    EscapePanel *escapePanel;

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