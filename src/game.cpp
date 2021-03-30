#include "game.h"

GameWindow::GameWindow(QWindow *parent) : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);

    mousePos = QPoint(0.0f, 0.0f);

    inputHandler = new Input();
}

GameWindow::~GameWindow()
{
    delete m_device;
}

void GameWindow::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

void GameWindow::initialize()
{
	initializeOpenGLFunctions();

    //glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	defaultShaderProgram = new QOpenGLShaderProgram(this);
    defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/default.vs");
    defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shader/default.fs");
    defaultShaderProgram->link();

    guiShaderProgram = new QOpenGLShaderProgram(this);
    guiShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "shader/default.vs");
    guiShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "shader/default.fs");
    guiShaderProgram->link();

    // texture Atlas
    // ---------

    QImage textureAtlas = QImage(QString("res/tiles/TeclaEatsAtlas.png"));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &baseTileTexture1);
    glBindTexture(GL_TEXTURE_2D, baseTileTexture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    defaultShaderProgram->setUniformValue("texture1", baseTileTexture1);

    // texture Menu
    // ---------

    textureAtlas = QImage(QString("res/menu/Menu.png"));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &baseMenu1);
    glBindTexture(GL_TEXTURE_2D, baseMenu1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    // texture Font
    // ---------

    textureAtlas = QImage(QString("res/font/font.png"));
    textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // load image, create texture and generate mipmaps
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    camera = new Camera(QVector3D(0.0f, 0.0f, 3.0f));

    tileLoader = new TilesLoader("res/tiles/");

    fontLoader = new FontLoader("res/font/fontmap.ft");

    Map map(&ecs, tileLoader, 10, 9, 10);
    tileMap = map.getMap();

    QObject::connect(inputHandler, SIGNAL(updatedKeyInput(Input*, double)), camera, SLOT(updateKeyboard(Input*, double)));
    QObject::connect(inputHandler, SIGNAL(updatedMouseInput(Input*, double)), camera, SLOT(updateMouse(Input*, double)));
}

void GameWindow::render()
{
    static auto currentTime = QDateTime::currentMSecsSinceEpoch();
    static auto lastTime = QDateTime::currentMSecsSinceEpoch();

    static int nbFrames = 0;
    static auto lastFPSCount = currentTime;

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.1f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTime = QDateTime::currentMSecsSinceEpoch();
    inputHandler->updateInput(float(currentTime - lastTime) / 1000);

    //fps counter
    nbFrames++;
    if(currentTime - lastFPSCount >= 1000)
    {
        std::cout << nbFrames << std::endl;
        nbFrames = 0;
        lastFPSCount += 1000;
    }

    defaultShaderProgram->bind();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    float resolution = (float)width() / (float)height();

    projection.setToIdentity();
    //projection.perspective(60 / resolution, resolution, 0.1f, 100.0f); // Fix the zoom
    projection.perspective(((float)height() * retinaScale) / 16.0f, resolution, 0.1f, 100.0f); // Fix the zoom
    //projection.ortho(-resolution, resolution, -1, 1, -1, 1); // Fix the zoom

    std::cout << ((float)height() * retinaScale) / 16.0f << std::endl;

    //std::cout << camera->Zoom / resolution << std::endl;
    //std::cout << (float)width() / (float)height() << std::endl;
    //std::cout << (float)height() << std::endl;

    view = camera->GetViewMatrix();
    scale.setToIdentity();
    scale.scale(QVector3D(0.3f, 0.3f, 0.0f));

    glBindTexture(GL_TEXTURE_2D, baseTileTexture1);

    float selectedTileX = ((mousePos.x() - width() * retinaScale / 2 )) / 64.0f * 1.6f;// + camera->Position.x();
    float selectedTileY = ((height() * retinaScale / 2 - mousePos.y())) / 64.0f * 1.6f;// + camera->Position.y();

    //std::cout << "X: " << selectedTileX << " Y: " << selectedTileY << std::endl; 

    for(int x = 9; x >= 0; x--)
    {
        for(int y = 8; y >= 0; y--)
        {
            auto tilePos = tileMap[x][y]->get<Position>();
            auto tileTex = tileMap[x][y]->get<TileHolder>();

            //std::cout << "X: " << x << " Y: " << y << tileTex->tileId->getName() << std::endl;

            model.setToIdentity();
            model.translate(QVector3D((tilePos->x - tilePos->y) / 2, (tilePos->x + tilePos->y) / 4, 0.0f));

            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

            tileTex->tileId->getMesh()->bind();
            glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

            if(x == std::floor(selectedTileX / 2 + selectedTileY + 1) &&  y == std::floor(selectedTileY - selectedTileX / 2 + 1))
            {
                tileLoader->getTile("Selected Tile")->getMesh()->bind();
                glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, fontTexture);

    projection.setToIdentity();
    view.setToIdentity();
    
    scale.setToIdentity();
    scale.scale(QVector3D(8 / 75.0f, 9 / 75.0f, 0.0f));

    model.setToIdentity();
    model.translate(QVector3D(0.1f, 0.1f, 0.0f));

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

    fontLoader->getChara("2")->getMesh()->bind();
    //tileLoader->getTile("Dirt")->getMesh()->bind();
    glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

    scale.setToIdentity();
    scale.scale(QVector3D(9 / 50.0f, 9 / 50.0f, 0.0f));

    model.setToIdentity();
    model.translate(QVector3D(0.1f + 6 * 10 / 50.0f, 0.1f, 0.0f));

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

    fontLoader->getChara("0")->getMesh()->bind();
    //tileLoader->getTile("Dirt")->getMesh()->bind();
    glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

/*
    guiShaderProgram->bind();

    glBindTexture(GL_TEXTURE_2D, baseMenu1);

    projection.setToIdentity();
    view.setToIdentity();
    scale.setToIdentity();
    scale.scale(QVector3D(2.0f, 2.0f, 0.0f));
    scale.scale(QVector3D((width() * retinaScale - width() * retinaScale / 1.5f) / width() * retinaScale, (height() * retinaScale - height() * retinaScale / 1.5f) / height() * retinaScale, 0.0f));
    model.setToIdentity();
    model.translate(QVector3D(-0.5f, 0.5f, 0.0f));

    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("projection"), projection);
    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("view"), view);
    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("model"), model);
    guiShaderProgram->setUniformValue(guiShaderProgram->uniformLocation("scale"), scale);

    VAO->bind();
    glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);
*/

    lastTime = currentTime;
}

void GameWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYPRESSED);
}

void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        event->ignore();
    else
        inputHandler->registerKeyInput((Qt::Key)event->key(), Input::InputState::KEYRELEASED);
}

void GameWindow::mouseMoveEvent(QMouseEvent *event)
{
    static float lastMousePosX = event->pos().x();
	static float lastMousePosY = event->pos().y();

    mousePos = event->pos();

    QPoint mouseDelta;

    mouseDelta.setX((lastMousePosX - event->pos().x()) * xSensitivity);
    mouseDelta.setY((event->pos().y() - lastMousePosY) * ySensitivity); // reversed since y-coordinates go from bottom to top
	
    //std::cout << mouseDelta.x() << mouseDelta.y() << std::endl;

    inputHandler->registerMouseMove(event->pos(), mouseDelta);

    lastMousePosX = event->pos().x();
	lastMousePosY = event->pos().y();
}

void GameWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSEPRESS);
}

void GameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() != Qt::NoButton)
        inputHandler->registerMouseInput((Qt::MouseButton)event->button(), Input::InputState::MOUSERELEASE);
}

void GameWindow::renderLater()
{
    requestUpdate();
}

void GameWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}

bool GameWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void GameWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void GameWindow::tick()
{
    auto currentTime = QDateTime::currentMSecsSinceEpoch();
    auto lastTime = QDateTime::currentMSecsSinceEpoch();

    while(ticking)
    {
        lastTime = QDateTime::currentMSecsSinceEpoch();



        currentTime = QDateTime::currentMSecsSinceEpoch();
        std::this_thread::sleep_for(std::chrono::milliseconds(40 - (currentTime - lastTime)));
    }
}
