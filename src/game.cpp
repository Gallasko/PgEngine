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

    auto debugText = ecs.createEntity();
    auto debugTextC = ecs.attach<Sentence>(debugText, { "Debug: ", 8.0f, fontLoader});
    
    debugTextC->setX(10);
    debugTextC->setY(10);

    fpsCounter = ecs.createEntity();
    auto fpsCounterC = ecs.attach<Sentence>(fpsCounter, {"00", 8.0f, fontLoader});
    
    fpsCounterC->setX(10);
    fpsCounterC->setTopAnchor(debugTextC);
    fpsCounterC->setTopMargin(10);

    auto fpsText = ecs.createEntity();
    auto fpsTextC = ecs.attach<Sentence>(fpsText, {"Fps", 8.0f, fontLoader});
    
    fpsTextC->setX(10);
    fpsTextC->setTopAnchor(debugTextC);
    fpsTextC->setTopMargin(10);
    fpsTextC->setLeftAnchor(fpsCounterC);
    fpsTextC->setLeftMargin(10);

    auto text = ecs.createEntity();
    auto textC = ecs.attach<Sentence>(text, {"ABCDEFGHIJKLMN", 4.0f, fontLoader});
    
    textC->setX(10);
    textC->setTopAnchor(fpsCounterC);
    textC->setTopMargin(10);

    auto text2 = ecs.createEntity();
    auto text2C = ecs.attach<Sentence>(text2, {"OPQRSTUVWXYZ", 4.0f, fontLoader});
    
    text2C->setX(10);
    text2C->setTopAnchor(textC);
    text2C->setTopMargin(10);

    auto text3 = ecs.createEntity();
    auto text3C = ecs.attach<Sentence>(text3, {"abcdefghijklmn", 4.0f, fontLoader});
    
    text3C->setX(10);
    text3C->setTopAnchor(text2C);
    text3C->setTopMargin(10);

    auto text4 = ecs.createEntity();
    auto text4C = ecs.attach<Sentence>(text4, {"opqrstuvwxyz", 4.0f, fontLoader});
    
    text4C->setX(10);
    text4C->setTopAnchor(text3C);
    text4C->setTopMargin(10);

    auto mousePosLabel = ecs.createEntity();
    auto mousePosLabelC = ecs.attach<Sentence>(mousePosLabel, {"Mouse Pos: ", 6.0f, fontLoader});
    
    mousePosLabelC->setX(10);
    mousePosLabelC->setTopAnchor(text4C);
    mousePosLabelC->setTopMargin(10);

    mousePosText = ecs.createEntity();
    auto mousePosTextC = ecs.attach<Sentence>(mousePosText, {"(0, 0)", 6.0f, fontLoader});
    
    mousePosTextC->setX(10);
    mousePosTextC->setTopAnchor(mousePosLabelC);
    mousePosTextC->setTopMargin(10);

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
        
        auto fpsText = fpsCounter->get<Sentence>();
        fpsText->setText(std::to_string(nbFrames), fontLoader);

        nbFrames = 0;
        lastFPSCount += 1000;
    }

    auto mousePosTextC = mousePosText->get<Sentence>();
    mousePosTextC->setText("(" + std::to_string(mousePos.x()) + ", " + std::to_string(mousePos.y()) + ")", fontLoader);

    renderGame();

    renderUi();

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

void GameWindow::renderGame()
{
    const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    defaultShaderProgram->bind();

    float resolution = (float)width() / (float)height();

    float gameScale = 0.25f;

    projection.setToIdentity();
    //projection.perspective(60 / resolution, resolution, 0.1f, 100.0f); // Fix the zoom
    projection.perspective(((float)height() * retinaScale) / 16.0f, resolution, 0.1f, 100.0f); // Fix the zoom
    //projection.ortho(-resolution, resolution, -1, 1, -1, 1); // Fix the zoom

    //std::cout << ((float)height() * retinaScale) / 16.0f << std::endl;

    //std::cout << camera->Zoom / resolution << std::endl;
    //std::cout << (float)width() / (float)height() << std::endl;
    //std::cout << (float)height() << std::endl;

    view = camera->GetViewMatrix();
    scale.setToIdentity();
    scale.scale(QVector3D(gameScale, gameScale, 0.0f));

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
            model.translate(QVector3D((tilePos->x - tilePos->y) * gameScale / 2.0f, (tilePos->x + tilePos->y) * gameScale / 4.0f, 0.0f));

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

}

void GameWindow::renderUi()
{
    const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    view.setToIdentity();

    // Text rendering

    glBindTexture(GL_TEXTURE_2D, fontTexture);

    for(auto sentence : ecs.view<Sentence>())
    {
        if(sentence.visible)
        {
            int currentX = sentence.x;
            for(int i = 0; i < sentence.nbChara; i++)
            {
                auto chara = sentence.letters[i];

                scale.setToIdentity();
                scale.scale(QVector3D((float)chara->getWidth() * sentence.scale / width(), (float)chara->getHeight() * sentence.scale / height(), 0.0f));

                model.setToIdentity();
                model.translate(QVector3D(-1.0f + (float)((chara->getWidth() * sentence.scale / 2) + currentX) / width(), 1.0f + (float)( -(chara->getHeight() * sentence.scale / 2) - chara->getOffset() * sentence.scale - sentence.y) / height(), 0.0f));

                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);
                defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

                chara->getMesh()->bind();
                glDrawElements(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, 0);

                currentX += sentence.scale * chara->getWidth() + 1;
            }
        }
    }
    
    // End Text Rendering

    /*
    static int textXPos = 0;
    static int textYPos = 0;
    static bool falling = true;

    if(falling)
    {
        textYPos++;
        if(textYPos == 2)
            falling = false;
    }
    else{
        textYPos--;
        if(textYPos == -2)
            falling = true;
    }
    */

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
